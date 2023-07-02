#pragma once
#include "lwipopts.h"

// --------------------------------- Playback buffers --------------------------------- //
// RAW buffer must be at least the size of PCM buffer
// for WAV decoding (can be fixed by modifying DMA address chain)

// MP3 compressed data buffer
#define BUF_HIDDEN_MP3_SIZE_FRAMES     2
#define BUF_MP3_SIZE_FRAMES            18
#define BUF_MP3_SIZE_BYTES_PER_FRAME   1024
#define BUF_HIDDEN_MP3_SIZE_BYTES      (BUF_HIDDEN_MP3_SIZE_FRAMES * BUF_MP3_SIZE_BYTES_PER_FRAME)
#define BUF_MP3_SIZE_BYTES             (BUF_MP3_SIZE_FRAMES * BUF_MP3_SIZE_BYTES_PER_FRAME)

// for MP3 playing
#define MP3_SAMPLES_PER_FRAME       1152 // sample = 2 channels * 16 bit = 32bit word
#define BUF_PCM_SIZE_FRAMES         4
#define BUF_PCM_SIZE_32BIT          (MP3_SAMPLES_PER_FRAME * BUF_PCM_SIZE_FRAMES)
#define BUF_PCM_HALF_32BIT          (BUF_PCM_SIZE_32BIT / 2)

// for WAV playing
#define BUF_PCM_SIZE_BYTES     (BUF_PCM_SIZE_32BIT * 4)

// --------------------------------- Playback variables --------------------------------- //
// buffer reversing after bad frame, dangerous can lead to loops
#define BUF_REVERSE                     0
// buffer overrun protection, can lead to loop
#define BUF_OVERRUN_PROTECTION          0
// target buffer health
#define HTTP_CONTENT_BUFFER_TARGET      70
#define PLAYER_END_TIMEOUT_MS           1000

// --------------------------------- Http --------------------------------- //
#define HTTP_HOST_MAX_LEN               128
#define HTTP_PATH_MAX_LEN               256

#define HTTP_QUERY_RESP_BUF_SIZE        256
#define HTTP_LOCATION_HDR_SIZE          256
#define HTTP_CONTENT_TYPE_HDR_SIZE      256

#define HTTP_DATA_BUF_SIZE_BYTES        TCP_WND
#define HTTP_CONNECT_TIMEOUT_MS         5000
#define HTTP_POLL_INTERVAL_MS           2000     // minimum is half a second
#define HTTP_NOTIFY_INDEX               1

// --------------------------------- Radio --------------------------------- //
// Search prompt length
#define MAX_PROMPT_LEN        16
#define SEARCH_URL_BUF_LEN    (128 + MAX_PROMPT_LEN + 1)

// Max stations per query
#define MAX_STATIONS          64 // max stations returned from search
#define MAX_STATIONS_PLS       4 // max stations to resolve from *.pls file
#define ST_UUID_LEN            0 // disabled
#define ST_NAME_LEN           32
#define ST_URL_LEN            64

// --------------------------------- RTOS --------------------------------- //
#define PRI_PLAYER              3  // handles player's buffer ack & playback stop
#define PRI_FIFO_QUEUE          3  // handles message passing to for ex. player task
#define PRI_LWIP_TCPIP          2
#define PRI_RADIO_SEARCH        1
#define PRI_WIFI_SETUP          1
#define PRI_INPUT               1

// --------------------------------- SD --------------------------------- //
#define SD_ENABLE 1

// --------------------------------- I2S --------------------------------- //
#define I2S_CLK_CHANNEL_BASE 18 // 18-clk 19-channel
#define I2S_DATA             20

// --------------------------------- LCD --------------------------------- //
#define LCD_SPI     1
#define LCD_SCK     10
#define LCD_TX      11
#define LCD_CS      9
#define LCD_RST     6
#define LCD_DC      8
#define LCD_BL      7

// --------------------------------- Buttons --------------------------------- //
#define BTN_UP      12
#define BTN_DOWN    21
#define BTN_LEFT    15
#define BTN_RIGHT   13
#define BTN_CENTER  14
