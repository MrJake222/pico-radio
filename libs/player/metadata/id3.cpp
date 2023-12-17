#include "id3.hpp"

#include <cstring>
#include <cstdio>
#include <util.hpp>

void ID3::begin() {
    // init as empty strings
    artist[0] = '\0';
    title[0] = '\0';
}

int ID3::try_parse() {
    memcpy(&hdr, cbuf.read_ptr(), sizeof(id3_hdr));

    if (strncmp(hdr.iden, "ID3", 3) != 0)
        // not a valid ID3 tag
        return -1;

    uint32_t size_left = decode_synchsafe(hdr.size_ss, 4);
    printf("detected valid ID3v2.%d.%d tag (size %lu bytes + hdr)\n", hdr.v_major, hdr.v_minor, size_left);
    cbuf.read_ack(10); // ack header

    if (ID3_HDR_EXT(hdr.flags)) {
        memcpy(&hdr_ext, cbuf.read_ptr(), sizeof(id3_hdr_ext));
        const uint32_t hdr_ext_size = decode_synchsafe(hdr_ext.size_ss, 4);
        cbuf.read_ack(hdr_ext_size); // ack extended header
        size_left -= hdr_ext_size;
    }

    // now the read pointer is past any headers

    while (size_left > 0) {
        if (*cbuf.read_ptr() == 0x00) {
            // padding
            cbuf.read_ack_large(size_left, FLAG_FALSE);
            break;
        }

        char fname[5];
        id3_ftype ftype;
        int fhdrsize;
        int fsize;

        switch (hdr.v_major) {
            case 4:
            case 3:
                read_frame_header_id3v2_4_3_0(fname, &ftype, &fhdrsize, &fsize);
                break;

            case 2:
                read_frame_header_id3v2_2_0(fname, &ftype, &fhdrsize, &fsize);
                break;

            default:
                puts("unsupported major ID3 version");
                return -1;
        }

        printf("  found frame '%s' len=%3d: ", fname, fsize);

        if (fname[0] == 'T') {
            text_field_to_utf8(cbuf.read_ptr(), fsize, buf_field);
            printf("text field: %s\n", buf_field);
        }
        else {
            puts("unsupported");
            buf_field[0] = '\0';
        }

        if (ftype == id3_artist) {
            // artist tag
            strcpy(artist, buf_field);
        }
        else if (ftype == id3_title) {
            // title tag
            strcpy(title, buf_field);
        }

        // ack frame
        cbuf.read_ack_large(fsize, FLAG_FALSE);
        size_left -= fsize + fhdrsize;
    }

    return 0;
}

void ID3::read_frame_header_id3v2_4_3_0(char* fname, id3_ftype* ftype, int* fhdrsize, int* fsize) {
    struct id3_frame_43_hdr frame;
    memcpy(&frame, cbuf.read_ptr(), sizeof(frame));
    *fhdrsize = sizeof(frame);

    memcpy(fname, frame.id, 4);
    fname[4] = '\0';

    if (strcmp(fname, "TPE1") == 0) {
        // artist tag
        *ftype = id3_artist;
    }
    else if (strcmp(fname, "TIT2") == 0) {
        // title tag
        *ftype = id3_title;
    }
    else {
        *ftype = id3_unknown;
    }

    // only id3v2.4.0 encodes frame sizes as sync-safe integers
    *fsize = hdr.v_major >= 4
              ? (int) decode_synchsafe(frame.size, 4)
              : (int) big_to_little_endian(frame.size, 4);

    cbuf.read_ack(sizeof(frame)); // ack frame header
}

void ID3::read_frame_header_id3v2_2_0(char* fname, id3_ftype* ftype, int* fhdrsize, int* fsize) {
    struct id3_frame_2_hdr frame;
    memcpy(&frame, cbuf.read_ptr(), sizeof(frame));
    *fhdrsize = sizeof(frame);

    memcpy(fname, frame.id, 3);
    fname[3] = '\0';

    if (strcmp(fname, "TP1") == 0) {
        // artist tag
        *ftype = id3_artist;
    }
    else if (strcmp(fname, "TT2") == 0) {
        // title tag
        *ftype = id3_title;
    }
    else {
        *ftype = id3_unknown;
    }

    // id3v2.2.0 has 3-byte long size
    *fsize = (int) big_to_little_endian(frame.size, 3);

    cbuf.read_ack(sizeof(frame)); // ack frame header
}

// don't do ack here
void ID3::text_field_to_utf8(uint8_t* p, int frame_size, char* out) {
    uint8_t encoding = *p++;
    frame_size -= 1;

    int out_len = 0;

    uint16_t utf16;
    bom_value bom = bom_invalid;

    uint8_t utf8[4];
    int utf8_len;

    while (frame_size) {
        switch (encoding) {
            case id3_enc_utf16be:
                // no byte-order-mark (implicit big-endian, set bom, intentional fall-through)
                bom = bom_be;
            case id3_enc_utf16:
                // Byte-order-mark present
                if (bom == bom_invalid) {
                    if (*p == 0xFF && *(p+1) == 0xFE)
                        bom = bom_le;
                    else if (*p == 0xFE && *(p+1) == 0xFF)
                        bom = bom_be;
                    else
                        puts("byte-order-mark invalid");
                }
                else {
                    // bom already set
                    if (bom == bom_le)
                        utf16 = *(p+1) << 8 | *p;
                    else if (bom == bom_be)
                        utf16 = *p << 8 | *(p+1);
                    else
                        utf16 = 0xFFFF;

                    utf16_to_utf8(utf16, utf8, &utf8_len);
                    for (int k=0; k<utf8_len; k++)
                        out[out_len++] = utf8[k];
                }

                frame_size -= 2;
                p += 2;
                break;

            case id3_enc_iso:
            case id3_enc_utf8:
            default:
                out[out_len++] = *p++;
                frame_size -= 1;
                break;
        }

        // 3 or fewer bytes will fit
        // max utf16 conversion size + null termination won't fit in 3 bytes
        if ((out_len + 3) >= ID3_BUF_LEN) {
            // possible buffer overflow
            break;
        }
    }

    out[out_len] = '\0';
}

uint32_t ID3::convert_bytes(const uint8_t* b, int b_len, int b_width) {
    uint32_t out = 0;

    for (int i=0; i<b_len; i++) {
        out |= ((uint32_t)b[i]) << ((b_len-1-i) * b_width);
    }

    return out;
}

void ID3::utf16_to_utf8(const uint16_t utf16, uint8_t* utf8, int* utf8_len) {
    if (utf16 <= 0x7F) {
        utf8[0] = utf16;
        *utf8_len = 1;
    }
    else if (utf16 <= 0x3FF) {
        utf8[0] = utf16 >> 6;   // first 5/11 bytes
        utf8[1] = utf16 & 0x3F; // last  6/11 bytes
        *utf8_len = 2;
    }
    else {
        utf8[0] = utf16 >> 12;         // first  4/16 bytes
        utf8[1] = (utf16 >> 6) & 0x3F; // second 6/16 bytes
        utf8[2] = utf16 & 0x3F;        // last   6/16 bytes
        *utf8_len = 3;
    }
}
