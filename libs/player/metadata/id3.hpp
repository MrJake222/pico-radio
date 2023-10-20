#pragma once

#include <config.hpp>
#include <circularbuffer.hpp>

struct id3_hdr {
    char iden[3];
    uint8_t v_major;
    uint8_t v_minor;
    uint8_t flags;
    uint8_t size_ss[4];
};

struct id3_hdr_ext {
    uint8_t size_ss[4];
    // TODO id3_hdr_ext: support more options
};

struct id3_frame {
    char id[4];
    uint8_t size[4];
    uint8_t f1;
    uint8_t f2; // "flags"
};

enum {
    id3_enc_iso     = 0x00,
    id3_enc_utf16   = 0x01,
    id3_enc_utf16be = 0x02,
    id3_enc_utf8    = 0x03,
};

enum bom_value {
    bom_invalid,
    bom_le,
    bom_be
};

#define ID3_HDR_UNSYNC(flags) ((flags) & (1<<7))
#define ID3_HDR_EXT(flags)    ((flags) & (1<<6))
#define ID3_HDR_EXP(flags)    ((flags) & (1<<5))
#define ID3_HDR_FOOTER(flags) ((flags) & (1<<4))

#define ID3_BUF_LEN   (PLAYER_META_BUF_LEN)

class ID3 {

    volatile CircularBuffer& cbuf;

    struct id3_hdr hdr;
    struct id3_hdr_ext hdr_ext;
    struct id3_frame frame;

    char buf_field[ID3_BUF_LEN];

    char artist[ID3_BUF_LEN];
    char title[ID3_BUF_LEN];

    void text_field_to_utf8(uint8_t* p, int frame_size, char* out);

    static uint32_t decode_synchsafe(const uint8_t* b, int b_len);
    static void utf16_to_utf8(uint16_t utf16, uint8_t* utf8, int* utf8_len);
    static uint32_t big_to_little_endian(const uint8_t data[4]);

public:
    ID3(volatile CircularBuffer& cbuf_)
        : cbuf(cbuf_) { }

    void begin();
    int try_parse();

    const char* get_artist() { return artist; }
    const char* get_title() { return title; }
};