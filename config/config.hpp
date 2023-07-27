#pragma once

// --------------------------------- Playback buffers --------------------------------- //
// RAW buffer must be at least the size of PCM buffer
// for WAV decoding (can be fixed by modifying DMA address chain)
// Now these buffers are equal and 27 648 bytes each

// MP3 compressed data buffer
#define BUF_HIDDEN_MP3_SIZE_FRAMES     2
#define BUF_MP3_SIZE_FRAMES            64
#define BUF_MP3_SIZE_BYTES_PER_FRAME   1024
#define BUF_HIDDEN_MP3_SIZE_BYTES      (BUF_HIDDEN_MP3_SIZE_FRAMES * BUF_MP3_SIZE_BYTES_PER_FRAME)
#define BUF_MP3_SIZE_BYTES             (BUF_MP3_SIZE_FRAMES * BUF_MP3_SIZE_BYTES_PER_FRAME)

// for MP3 playing
#define MP3_SAMPLES_PER_FRAME       1152 // sample = 2 channels * 16 bit = 32bit word
#define BUF_PCM_SIZE_FRAMES         4    // can be reduced to 2 (optimize format mp3 runs)
#define BUF_PCM_SIZE_32BIT          (MP3_SAMPLES_PER_FRAME * BUF_PCM_SIZE_FRAMES)
#define BUF_PCM_HALF_32BIT          (BUF_PCM_SIZE_32BIT / 2)

// for WAV playing
#define BUF_PCM_SIZE_BYTES     (BUF_PCM_SIZE_32BIT * 4)

// --------------------------------- Playback variables --------------------------------- //
// buffer reversing after bad frame, dangerous can lead to loops
#define BUF_REVERSE                     0
// buffer overrun protection, can lead to loop
#define BUF_OVERRUN_PROTECTION          0
#define PLAYER_END_TIMEOUT_MS           1000
#define PLAYER_META_BUF_LEN             128

// --------------------------------- Http --------------------------------- //
#define HTTP_HOST_MAX_LEN               128
#define HTTP_PATH_MAX_LEN               256

#define HTTP_QUERY_RESP_BUF_SIZE        256
#define HTTP_LOCATION_HDR_SIZE          256
#define HTTP_CONTENT_TYPE_HDR_SIZE      256

#define HTTP_TIMEOUT_MS                 5000
#define HTTP_POLL_INTERVAL_MS           2000     // minimum is half a second
#define HTTP_NOTIFY_INDEX               1

// --------------------------------- Radio --------------------------------- //
// Search prompt length
#define MAX_PROMPT_LEN        14
#define SEARCH_URL_BUF_LEN    (128 + MAX_PROMPT_LEN + 1)

// List parsing
#define LIST_MAX_LINE_LENGTH 128

// Max stations per query
#define MAX_STATIONS          64 // max stations returned from search
#define MAX_STATIONS_PLS       4 // max stations to resolve from *.pls file
#define ST_UUID_LEN            0 // disabled
#define ST_NAME_LEN           32
#define ST_URL_LEN            64

// ICY metadata
#define ICY_BUF_LEN           128

// --------------------------------- RTOS --------------------------------- //
#define PRI_PLAYER              3  // handles player's buffer ack & playback stop
#define PRI_PLAYER_STAT         2  // handles player's statistics updates
#define PRI_FIFO_QUEUE          3  // handles message passing to for ex. player task
#define PRI_LWIP_TCPIP          5
#define PRI_LIST_LOADER         1
#define PRI_DISPLAY_TICKER      1 // updates scrollable texts
#define PRI_HW_SETUP            1
#define PRI_WIFI_SETUP          1
#define PRI_INPUT               1

#define MIN_FREE_STACK            100
#define STACK_PLAYER              (220 + MIN_FREE_STACK)
#define STACK_PLAYER_STAT         (164 + MIN_FREE_STACK)
#define STACK_FIFO_QUEUE          configMINIMAL_STACK_SIZE // currently unused
#define STACK_LIST_LOADER         (240 + MIN_FREE_STACK)
#define STACK_DISPLAY_TICKER      (160 + MIN_FREE_STACK)
#define STACK_HW_SETUP            (225 + MIN_FREE_STACK)
#define STACK_WIFI_SETUP          (200 + MIN_FREE_STACK)
#define STACK_INPUT               (273 + MIN_FREE_STACK)

// --------------------------------- Flash --------------------------------- //
#define LITTLEFS_SIZE             (1<<16) // 64K, base 0x101F0000
#define LITTLEFS_CACHES           1024    // 3 blocks of this size
#define PATH_FAVOURITES           "/favourites.m3u"

// --------------------------------- SD --------------------------------- //
#define SD_ENABLE 1

// --------------------------------- LCD --------------------------------- //
#define LCD_SCROLLED_TEXTS_MAX          4
#define LCD_SCROLLED_TEXTS_LEN_MAX      64
#define LCD_TICK_INTERVAL_MS            50

// --------------------------------- PINOUTS --------------------------------- //
#ifdef PINOUT_PROTOTYPE
#define I2S_CLK_CHANNEL_BASE 18 // 18-clk 19-channel
#define I2S_DATA             20

#define SD_SPI_ID   0
#define SD_SCK      2
#define SD_TX       3
#define SD_RX       4
#define SD_CS       5
#define SD_CD       0
#define SD_WP       1

#define LCD_SPI_ID  1
#define LCD_SCK     10
#define LCD_TX      11
#define LCD_CS      9
#define LCD_RST     6
#define LCD_DC      8
#define LCD_BL      7

#define BTN_UP      12
#define BTN_DOWN    21
#define BTN_LEFT    15
#define BTN_RIGHT   13
#define BTN_CENTER  14
#endif

#ifdef PINOUT_V1
#define I2S_CLK_CHANNEL_BASE  8 // 8-clk 9-channel
#define I2S_DATA              7

#define SD_SPI_ID   0
#define SD_SCK      2
#define SD_TX       3
#define SD_RX       4
#define SD_CS       1
#define SD_CD       5
#define SD_WP       6

#define LCD_SPI_ID   1
#define LCD_SCK     14
#define LCD_TX      15
#define LCD_CS      13
#define LCD_RST     11
#define LCD_DC      12
#define LCD_BL      10

#define BTN_UP      20
#define BTN_DOWN    19
#define BTN_LEFT    22
#define BTN_RIGHT   21
#define BTN_CENTER  18
#endif
