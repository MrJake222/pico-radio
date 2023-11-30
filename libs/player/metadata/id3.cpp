#include "id3.hpp"

#include <cstring>
#include <cstdio>

// needed because some function require reference to
// bool <abort>, and we never abort
static const bool FALSE = false;

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
            cbuf.read_ack_large(size_left, FALSE);
            break;
        }

        memcpy(&frame, cbuf.read_ptr(), sizeof(id3_frame));
        const char frame_name[5] = { frame.id[0], frame.id[1], frame.id[2], frame.id[3], '\0' };

        // only id3v2.4 encodes frame sizes as sync-safe integers
        const uint32_t frame_size =
                hdr.v_major >= 4
                    ? decode_synchsafe(frame.size, 4)
                    : big_to_little_endian(frame.size);

        printf("  found frame '%s' len=%lu: ", frame_name, frame_size);
        cbuf.read_ack(10); // ack header

        if (frame_name[0] == 'T') {
            puts("text field");
            text_field_to_utf8(cbuf.read_ptr(), (int)frame_size, buf_field);
        }
        else {
            puts("unsupported");
            buf_field[0] = '\0';
        }

        if (strcmp(frame_name, "TPE1") == 0) {
            // artist tag
            strcpy(artist, buf_field);
        }
        else if (strcmp(frame_name, "TIT2") == 0) {
            // title tag
            strcpy(title, buf_field);
        }

        // ack frame
        cbuf.read_ack_large(frame_size, FALSE);
        size_left -= frame_size + 10; // + header size
    }

    return 0;
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

uint32_t ID3::decode_synchsafe(const uint8_t* b, int b_len) {
    uint32_t out = 0;

    for (int i=0; i<b_len; i++) {
        out |= ((uint32_t)b[i]) << ((b_len-1-i) * 7);
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

uint32_t ID3::big_to_little_endian(const uint8_t data[4]) {
    return data[0] << 24
         | data[1] << 16
         | data[2] << 8
         | data[3];
}
