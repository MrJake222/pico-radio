#include "icons.hpp"

static const uint8_t icon_backspace_data[] = {
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x0d, 0x26, 0x26, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x1f,
        0x01, 0x03, 0x07, 0xd2, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8,
        0x04, 0x00, 0x8e, 0xfc, 0x3c, 0x24, 0x19, 0x2b, 0x2a, 0x2c, 0x20, 0x1a, 0x2a, 0x65, 0xff,
        0x00, 0x32, 0xff, 0x60, 0x00, 0x32, 0x7d, 0x00, 0x00, 0x00, 0x36, 0x84, 0x00, 0x40, 0xfe,
        0x02, 0xdb, 0xbd, 0x00, 0x05, 0x37, 0xff, 0xa9, 0x00, 0x3b, 0xff, 0xb0, 0x02, 0x4b, 0xff,
        0x8d, 0xf5, 0x1c, 0x01, 0x04, 0x00, 0x3a, 0xf4, 0xac, 0xea, 0xa6, 0x00, 0x00, 0x48, 0xff,
        0xff, 0x93, 0x00, 0x04, 0x00, 0x06, 0x00, 0x65, 0xff, 0xe0, 0x00, 0x03, 0x00, 0x47, 0xff,
        0x8d, 0xf5, 0x1c, 0x01, 0x04, 0x00, 0x3a, 0xf4, 0xac, 0xea, 0xa6, 0x00, 0x00, 0x48, 0xff,
        0x02, 0xdb, 0xbd, 0x00, 0x05, 0x37, 0xff, 0xa9, 0x00, 0x3b, 0xff, 0xb0, 0x02, 0x4b, 0xff,
        0x00, 0x32, 0xff, 0x60, 0x00, 0x32, 0x7d, 0x00, 0x00, 0x00, 0x36, 0x84, 0x00, 0x40, 0xfe,
        0x04, 0x00, 0x8e, 0xfc, 0x3c, 0x24, 0x19, 0x2b, 0x2a, 0x2c, 0x20, 0x1a, 0x2a, 0x65, 0xff,
        0x01, 0x03, 0x07, 0xd2, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8,
        0x00, 0x01, 0x00, 0x0d, 0x26, 0x26, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x1f,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const struct icon icon_backspace {
    .w = 15,
    .h = 15,
    .data = icon_backspace_data
};

static const uint8_t icon_back_data[] = {
        0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x0c, 0x03, 0x04, 0x04, 0x04, 0x04, 0x01, 0x00,
        0x00, 0x13, 0xa0, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x2d, 0xf4, 0xd5, 0x87, 0x90, 0x8f, 0x8e, 0x91, 0x77, 0x08, 0x00,
        0x13, 0x91, 0xd9, 0x5e, 0x3b, 0x3b, 0x3c, 0x3f, 0x82, 0xad, 0x08,
        0x00, 0x00, 0x48, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x5b,
        0x00, 0x03, 0x00, 0x00, 0x03, 0x02, 0x02, 0x07, 0x00, 0x78, 0x72,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2e, 0xc3, 0x22,
        0x00, 0x03, 0x8b, 0xa2, 0xa1, 0xa2, 0xa3, 0xa8, 0xb6, 0x40, 0x00,
        0x00, 0x00, 0x12, 0x16, 0x15, 0x16, 0x15, 0x14, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
};

const struct icon icon_back {
    .w = 11,
    .h = 11,
    .data = icon_back_data
};

static const uint8_t icon_search_data[] = {
        0x03, 0x00, 0x37, 0xc3, 0xfc, 0xff, 0xef, 0x8e, 0x08, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x57, 0xfc, 0xb3, 0x5c, 0x52, 0x75, 0xe3, 0xcc, 0x10, 0x02, 0x01, 0x00,
        0x30, 0xff, 0x66, 0x00, 0x00, 0x00, 0x00, 0x07, 0xce, 0xc2, 0x00, 0x02, 0x00,
        0xc4, 0xb6, 0x00, 0x05, 0x04, 0x03, 0x06, 0x00, 0x1a, 0xff, 0x3f, 0x00, 0x03,
        0xff, 0x4f, 0x02, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0xc8, 0x90, 0x00, 0x04,
        0xff, 0x38, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x02, 0xb6, 0x9e, 0x00, 0x04,
        0xfb, 0x59, 0x02, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0xd2, 0x88, 0x00, 0x04,
        0xb2, 0xca, 0x00, 0x02, 0x05, 0x04, 0x06, 0x00, 0x2e, 0xfc, 0x30, 0x01, 0x02,
        0x20, 0xf7, 0x89, 0x00, 0x00, 0x00, 0x00, 0x1e, 0xdd, 0xd3, 0x00, 0x01, 0x02,
        0x00, 0x3a, 0xf3, 0xd7, 0x84, 0x72, 0xa3, 0xf6, 0xf9, 0xff, 0x9e, 0x01, 0x00,
        0x03, 0x00, 0x19, 0x8e, 0xd4, 0xdf, 0xc0, 0x57, 0x22, 0xd3, 0xff, 0xa9, 0x08,
        0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0xbd, 0xff, 0xc9,
        0x00, 0x00, 0x02, 0x04, 0x02, 0x01, 0x03, 0x03, 0x03, 0x00, 0x0a, 0xbc, 0xe9,
};

const struct icon icon_search {
    .w = 13,
    .h = 13,
    .data = icon_search_data
};

static const uint8_t icon_star_empty_data[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x12, 0xb2, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x44, 0xff, 0x38, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x03, 0x04, 0x03, 0x07, 0x01, 0x84, 0x8e, 0x7c, 0x01, 0x07, 0x03, 0x04, 0x03, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xab, 0x14, 0xaf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x06, 0x57, 0x52, 0x64, 0x65, 0x8f, 0x8f, 0x00, 0x9d, 0x85, 0x65, 0x63, 0x51, 0x56, 0x04,
        0x07, 0xad, 0xfb, 0x74, 0x69, 0x71, 0x1b, 0x01, 0x22, 0x72, 0x66, 0x78, 0xff, 0xa0, 0x05,
        0x01, 0x00, 0x68, 0xad, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xae, 0x5f, 0x00, 0x01,
        0x00, 0x03, 0x00, 0x48, 0xcc, 0x44, 0x04, 0x09, 0x04, 0x50, 0xca, 0x3e, 0x00, 0x03, 0x00,
        0x00, 0x00, 0x07, 0x00, 0x85, 0x6d, 0x00, 0x00, 0x00, 0x7c, 0x6e, 0x00, 0x07, 0x00, 0x00,
        0x00, 0x00, 0x01, 0x04, 0xb3, 0x09, 0x23, 0x96, 0x18, 0x14, 0xaa, 0x00, 0x02, 0x00, 0x00,
        0x00, 0x02, 0x00, 0x30, 0x95, 0x4d, 0xbc, 0x71, 0xc2, 0x49, 0x96, 0x1e, 0x00, 0x01, 0x00,
        0x00, 0x04, 0x00, 0x75, 0xe3, 0x96, 0x17, 0x00, 0x22, 0xa8, 0xe6, 0x5d, 0x00, 0x04, 0x00,
        0x00, 0x03, 0x00, 0x92, 0x77, 0x00, 0x00, 0x05, 0x00, 0x00, 0x89, 0x84, 0x00, 0x03, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x02, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
};

const struct icon icon_star_empty {
    .w = 15,
    .h = 15,
    .data = icon_star_empty_data
};

static const uint8_t icon_star_filled_data[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0d, 0x9a, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x38, 0xff, 0x2a, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x03, 0x03, 0x03, 0x07, 0x02, 0x80, 0xfe, 0x71, 0x03, 0x08, 0x03, 0x03, 0x03, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x4f, 0x4c, 0x4c, 0x4c, 0x64, 0xfc, 0xff, 0xf7, 0x5c, 0x4c, 0x4b, 0x4d, 0x4c, 0x01,
        0x04, 0x91, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x86, 0x02,
        0x01, 0x00, 0x53, 0xeb, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xe5, 0x49, 0x00, 0x01,
        0x00, 0x03, 0x00, 0x26, 0xc4, 0xfe, 0xfd, 0xff, 0xfd, 0xfe, 0xba, 0x1e, 0x00, 0x03, 0x00,
        0x00, 0x00, 0x07, 0x00, 0x5f, 0xff, 0xf9, 0xfd, 0xfa, 0xff, 0x48, 0x00, 0x07, 0x00, 0x00,
        0x00, 0x00, 0x02, 0x01, 0xb7, 0xfc, 0xff, 0xff, 0xff, 0xfc, 0xa0, 0x01, 0x03, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x12, 0xef, 0xff, 0xbc, 0x49, 0xcc, 0xff, 0xe0, 0x07, 0x00, 0x01, 0x00,
        0x00, 0x04, 0x00, 0x61, 0xff, 0x88, 0x04, 0x00, 0x0a, 0x9a, 0xff, 0x48, 0x00, 0x03, 0x00,
        0x00, 0x03, 0x00, 0x80, 0x60, 0x00, 0x01, 0x04, 0x00, 0x00, 0x72, 0x72, 0x00, 0x03, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const struct icon icon_star_filled {
    .w = 15,
    .h = 15,
    .data = icon_star_filled_data
};

static const uint8_t icon_fav_back_data[] = {
        0x00, 0x18, 0x8c, 0x73, 0x58, 0x62, 0x6a, 0x55, 0x6b, 0x47, 0x69, 0x31, 0x32, 0x00, 0x00,
        0x00, 0x05, 0x1c, 0x17, 0x12, 0x13, 0x15, 0x11, 0x1a, 0x1a, 0x35, 0x2e, 0x5f, 0x70, 0x54,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x40, 0xe0,
        0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x03, 0x00, 0x16, 0xed, 0x80, 0x84, 0x3c,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0xbc, 0xff, 0xff, 0x26, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x3a, 0xd3, 0xc0, 0xba, 0x34, 0x02,
        0x00, 0x01, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x46, 0xf4, 0x16, 0x0a, 0xce, 0xda, 0xd9, 0xda, 0xd9, 0xda, 0xda, 0xdc, 0xc8, 0x10,
        0x00, 0x18, 0x6c, 0x05, 0x03, 0x3d, 0x41, 0x40, 0x40, 0x41, 0x41, 0x40, 0x41, 0x3b, 0x05,
        0x00, 0x41, 0xe3, 0x15, 0x09, 0xc1, 0xcd, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcd, 0xbb, 0x0f,
        0x00, 0x19, 0x75, 0x06, 0x03, 0x42, 0x47, 0x46, 0x46, 0x46, 0x46, 0x46, 0x47, 0x41, 0x05,
        0x00, 0x3a, 0xe6, 0x1c, 0x04, 0xbf, 0xd0, 0xce, 0xcf, 0xcf, 0xcf, 0xce, 0xd0, 0xc2, 0x15,
        0x00, 0x16, 0x70, 0x06, 0x02, 0x3d, 0x42, 0x41, 0x42, 0x42, 0x42, 0x41, 0x42, 0x3d, 0x06,
        0x00, 0x46, 0xf3, 0x16, 0x0a, 0xcd, 0xda, 0xd8, 0xd9, 0xd9, 0xd9, 0xd8, 0xda, 0xc7, 0x10,
        0x00, 0x0e, 0x3e, 0x03, 0x02, 0x24, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x23, 0x03,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const struct icon icon_fav_back {
    .w = 15,
    .h = 16,
    .data = icon_fav_back_data
};

static const uint8_t icon_battery_0_data[] = {
        0x00, 0x0c, 0xd2, 0xe6, 0xe6, 0xd1, 0x0b, 0x00,
        0x00, 0x2e, 0xa4, 0xa8, 0xa8, 0xa3, 0x2c, 0x00,
        0x8c, 0xb9, 0x99, 0x95, 0x95, 0x99, 0xb9, 0x8d,
        0xc7, 0x2f, 0x02, 0x39, 0x34, 0x01, 0x2e, 0xc7,
        0xbd, 0x2f, 0x05, 0x71, 0x6a, 0x03, 0x2e, 0xbd,
        0xbf, 0x30, 0x00, 0x09, 0x08, 0x00, 0x30, 0xbf,
        0xbf, 0x2f, 0x00, 0x03, 0x03, 0x00, 0x2f, 0xbf,
        0xbf, 0x2f, 0x01, 0x00, 0x00, 0x01, 0x2f, 0xbf,
        0xbe, 0x2e, 0x00, 0x35, 0x32, 0x00, 0x2e, 0xbe,
        0xc5, 0x37, 0x08, 0x38, 0x36, 0x08, 0x38, 0xc5,
        0x87, 0xc8, 0xba, 0xb1, 0xb2, 0xba, 0xc8, 0x88,
};

static const uint8_t icon_battery_50_data[] = {
        0x00, 0x0e, 0xf0, 0xff, 0xff, 0xee, 0x0c, 0x01,
        0x00, 0x32, 0xba, 0xbf, 0xbe, 0xb9, 0x31, 0x00,
        0xa0, 0xca, 0xad, 0xaa, 0xaa, 0xad, 0xcb, 0xa1,
        0xe1, 0x35, 0x02, 0x39, 0x35, 0x01, 0x35, 0xe2,
        0xd6, 0x36, 0x07, 0x73, 0x6d, 0x05, 0x37, 0xd7,
        0xd8, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x2e, 0xd9,
        0xd8, 0x57, 0x2b, 0x2b, 0x2b, 0x2b, 0x57, 0xd9,
        0xd7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd8,
        0xd6, 0xff, 0xfe, 0xff, 0xff, 0xfe, 0xff, 0xd7,
        0xde, 0xff, 0xfe, 0xff, 0xff, 0xfe, 0xff, 0xdf,
        0x99, 0xf9, 0xf8, 0xfa, 0xfa, 0xf8, 0xf8, 0x9a,
};

static const uint8_t icon_battery_100_data[] = {
        0x00, 0x0e, 0xf0, 0xff, 0xff, 0xee, 0x0c, 0x01,
        0x00, 0x34, 0xbc, 0xc0, 0xc0, 0xbb, 0x33, 0x00,
        0xa0, 0xc3, 0xa4, 0xa6, 0xa6, 0xa4, 0xc3, 0xa1,
        0xe1, 0x57, 0x2b, 0x4c, 0x49, 0x2b, 0x57, 0xe2,
        0xd5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd6,
        0xd7, 0xff, 0xfe, 0xff, 0xff, 0xfe, 0xff, 0xd8,
        0xd7, 0xff, 0xfe, 0xff, 0xff, 0xfe, 0xff, 0xd8,
        0xd7, 0xff, 0xfe, 0xff, 0xff, 0xfe, 0xff, 0xd8,
        0xd6, 0xff, 0xfe, 0xff, 0xff, 0xfe, 0xff, 0xd7,
        0xde, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf,
        0x99, 0xf9, 0xf8, 0xfa, 0xfa, 0xf8, 0xf8, 0x9a,
};

const struct icon icon_battery_0 {
    .w = 8,
    .h = 11,
    .data = icon_battery_0_data
};

const struct icon icon_battery_50 {
    .w = 8,
    .h = 11,
    .data = icon_battery_50_data
};

const struct icon icon_battery_100 {
    .w = 8,
    .h = 11,
    .data = icon_battery_100_data
};

static const uint8_t icon_sd_data[] = {
        0x00, 0x49, 0x72, 0x6f, 0x71, 0x6f, 0x72, 0x43,
        0x57, 0xab, 0x90, 0x9c, 0x9b, 0x98, 0xa6, 0x95,
        0x8c, 0xbd, 0xa8, 0xad, 0xac, 0xa9, 0xb4, 0x86,
        0x89, 0x6e, 0x15, 0x16, 0x16, 0x14, 0x3f, 0x9b,
        0x97, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x3b, 0x95,
        0x8b, 0x29, 0x03, 0x06, 0x06, 0x03, 0x2d, 0x94,
        0x83, 0x20, 0x00, 0x00, 0x00, 0x00, 0x16, 0x88,
        0x87, 0x18, 0x3c, 0x69, 0x68, 0x3a, 0x13, 0x8b,
        0x87, 0x06, 0x71, 0xe1, 0xb8, 0x7c, 0x06, 0x89,
        0x8e, 0x43, 0x45, 0x61, 0x67, 0x42, 0x3c, 0x95,
        0x42, 0x86, 0x7c, 0x78, 0x76, 0x7d, 0x87, 0x4b,
};

static const uint8_t icon_sd_disabled_data[] = {
        0xb3, 0x5e, 0x71, 0x70, 0x71, 0x70, 0x78, 0xb9,
        0x8c, 0xe2, 0x91, 0x9c, 0x9d, 0x94, 0xdd, 0xb3,
        0x7a, 0xf1, 0xcf, 0xa2, 0xa3, 0xcc, 0xe7, 0x7e,
        0x88, 0x72, 0xbc, 0x33, 0x32, 0xb9, 0x45, 0x99,
        0x9b, 0x2e, 0x32, 0xa8, 0xa7, 0x35, 0x2b, 0x98,
        0x8a, 0x31, 0x00, 0xc1, 0xc3, 0x00, 0x35, 0x94,
        0x87, 0x10, 0x37, 0x9b, 0x9b, 0x3b, 0x07, 0x8c,
        0x82, 0x20, 0xda, 0x7b, 0x7a, 0xd8, 0x1b, 0x85,
        0x78, 0xa7, 0xc2, 0xcf, 0xa8, 0xc2, 0xa7, 0x7b,
        0xb0, 0xc5, 0x44, 0x64, 0x6a, 0x42, 0xc2, 0xbb,
        0xbd, 0x81, 0x7d, 0x78, 0x76, 0x7e, 0x81, 0xbc,
};

const struct icon icon_sd {
        .w = 8,
        .h = 11,
        .data = icon_sd_data
};

const struct icon icon_sd_disabled {
        .w = 8,
        .h = 11,
        .data = icon_sd_disabled_data
};

static const uint8_t icon_folder_data[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x47, 0x63, 0x5e, 0x62, 0x22, 0x00, 0x02, 0x01, 0x02, 0x02, 0x00, 0x00,
        0x34, 0xb7, 0x70, 0x6f, 0x76, 0xb6, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x42, 0x82, 0x00, 0x00, 0x00, 0x24, 0xa9, 0xa3, 0x9b, 0xa8, 0x46, 0x03, 0x05,
        0x40, 0x8b, 0x04, 0x07, 0x00, 0x00, 0x00, 0x19, 0x18, 0x51, 0x6b, 0x00, 0x00,
        0x40, 0x8a, 0x00, 0x00, 0x50, 0xa6, 0x99, 0x94, 0x93, 0x94, 0x9a, 0xa0, 0x4b,
        0x40, 0x8b, 0x00, 0x0a, 0xc8, 0xee, 0xe5, 0xe8, 0xe8, 0xe7, 0xe3, 0xf4, 0x7e,
        0x40, 0x8e, 0x01, 0x4c, 0xe6, 0xd4, 0xd7, 0xd7, 0xd7, 0xd7, 0xd9, 0xd2, 0x1f,
        0x3f, 0x8e, 0x00, 0xa2, 0xe6, 0xd7, 0xda, 0xda, 0xda, 0xd7, 0xe7, 0x97, 0x00,
        0x45, 0x7c, 0x0a, 0xdc, 0xd7, 0xd6, 0xd7, 0xd7, 0xd7, 0xd4, 0xea, 0x43, 0x00,
        0x29, 0xab, 0xb0, 0xf2, 0xe7, 0xe9, 0xe9, 0xe9, 0xe8, 0xf1, 0xb5, 0x06, 0x01,
        0x00, 0x35, 0x5e, 0x4e, 0x51, 0x50, 0x51, 0x50, 0x50, 0x4f, 0x13, 0x00, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const struct icon icon_local {
        .w = 13,
        .h = 13,
        .data = icon_folder_data
};

const struct icon icon_folder {
        .w = 13,
        .h = 13,
        .data = icon_folder_data
};

static const uint8_t icon_shift_empty_data[] = {
        0x00, 0x00, 0x01, 0x04, 0x00, 0x49, 0xc8, 0x49, 0x00, 0x04, 0x01, 0x00, 0x00,
        0x00, 0x02, 0x00, 0x00, 0x76, 0xaf, 0x3f, 0xaf, 0x76, 0x00, 0x00, 0x02, 0x00,
        0x04, 0x00, 0x07, 0xa1, 0x98, 0x03, 0x00, 0x03, 0x98, 0xa1, 0x07, 0x00, 0x04,
        0x00, 0x1c, 0xb5, 0x6b, 0x00, 0x01, 0x03, 0x01, 0x00, 0x6a, 0xb5, 0x1c, 0x00,
        0x53, 0xd9, 0x62, 0x00, 0x04, 0x01, 0x00, 0x01, 0x04, 0x00, 0x62, 0xd9, 0x53,
        0xa1, 0xa7, 0xa0, 0x96, 0x00, 0x03, 0x00, 0x03, 0x00, 0x96, 0xa0, 0xa6, 0xa1,
        0x00, 0x00, 0x28, 0x9f, 0x00, 0x03, 0x00, 0x03, 0x00, 0x9f, 0x29, 0x00, 0x00,
        0x05, 0x03, 0x2f, 0x9a, 0x00, 0x01, 0x00, 0x01, 0x00, 0x9a, 0x2f, 0x03, 0x05,
        0x02, 0x00, 0x2f, 0xa3, 0x08, 0x0b, 0x08, 0x0b, 0x08, 0xa3, 0x2f, 0x00, 0x02,
        0x02, 0x00, 0x19, 0xc5, 0xb1, 0xb1, 0xb1, 0xb1, 0xb1, 0xc5, 0x19, 0x00, 0x02,
};

const struct icon icon_shift_empty {
        .w = 13,
        .h = 10,
        .data = icon_shift_empty_data
};

static const uint8_t icon_shift_filled_data[] = {
        0x00, 0x00, 0x01, 0x04, 0x00, 0x3e, 0xc6, 0x3e, 0x00, 0x04, 0x01, 0x00, 0x00,
        0x00, 0x02, 0x01, 0x00, 0x61, 0xf3, 0xff, 0xf3, 0x61, 0x00, 0x01, 0x02, 0x00,
        0x03, 0x00, 0x02, 0x91, 0xff, 0xff, 0xfc, 0xff, 0xff, 0x91, 0x02, 0x00, 0x03,
        0x00, 0x14, 0xb8, 0xff, 0xfc, 0xfe, 0xff, 0xfe, 0xfc, 0xff, 0xb9, 0x14, 0x00,
        0x4d, 0xed, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xed, 0x4e,
        0x9f, 0xb3, 0xb3, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xb3, 0xb3, 0x9f,
        0x00, 0x00, 0x10, 0xf8, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xf8, 0x11, 0x00, 0x00,
        0x05, 0x03, 0x20, 0xf7, 0xfe, 0xfd, 0xfe, 0xfd, 0xfe, 0xf7, 0x20, 0x03, 0x05,
        0x02, 0x00, 0x1e, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x1f, 0x00, 0x02,
        0x02, 0x00, 0x16, 0xd6, 0xe1, 0xe0, 0xe1, 0xe0, 0xe1, 0xd6, 0x16, 0x00, 0x02,
};

const struct icon icon_shift_filled {
        .w = 13,
        .h = 10,
        .data = icon_shift_filled_data
};