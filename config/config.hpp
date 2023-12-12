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

// PCM decompressed data
// only valid for MPEG1, use only below (maximum values)
#define MP3_SAMPLES_PER_FRAME       1152 // sample = 2 channels * 16 bit = 32bit word )
#define BUF_PCM_SIZE_FRAMES         4    // can be reduced to 2 (optimize format mp3 runs)
// use only for purposes defined below (if you need buffer size use CircularBuffer::size parameter)
#define BUF_PCM_SIZE_32BIT          (MP3_SAMPLES_PER_FRAME * BUF_PCM_SIZE_FRAMES)   // used to define buffer and to calculate frame count
#define BUF_PCM_HALF_32BIT          (BUF_PCM_SIZE_32BIT / 2)                        // used by DMA to manage half-transfers with chaining

// --------------------------------- Playback variables --------------------------------- //
// buffer reversing after bad frame, dangerous can lead to loops
#define BUF_REVERSE                     0
// buffer overrun protection, can lead to loop
#define BUF_OVERRUN_PROTECTION          0
// minimum buffer health for core1 to start decoding (beginning of playback or after underflow)
#define BUF_HEALTH_MIN                  50
// buffer health when a underflow is detected
#define BUF_HEALTH_UNDERFLOW            10

#define PLAYER_END_TIMEOUT_MS           1000
#define PLAYER_META_BUF_LEN             128

// --------------------------------- Http --------------------------------- //
#define HTTP_HOST_MAX_LEN               128
#define HTTP_PATH_MAX_LEN               256

#define HTTP_QUERY_RESP_BUF_SIZE        256
#define HTTP_LOCATION_HDR_SIZE          256
#define HTTP_CONTENT_TYPE_HDR_SIZE      256

#define HTTP_TIMEOUT_MS                 5000
#define HTTP_POLL_INTERVAL_MS           5000     // minimum is half a second
                                                 // calculated from MP3 buffer size (64kB / 128kbps = 4s)
#define HTTP_NOTIFY_INDEX               1
#define HTTP_MAX_REDIRECTS              2

// --------------------------------- Radio --------------------------------- //
// Search prompt length
#define MAX_PROMPT_LEN        16
#define PROMPT_URL_BUF_LEN    (MAX_PROMPT_LEN*3)                // (spaces are %20, hence *3)
#define SEARCH_URL_BUF_LEN    (128 + PROMPT_URL_BUF_LEN + 1)

// List parsing
#define LIST_MAX_LINE_LENGTH 128

// Max stations per query
#define MAX_ENTRIES          16 // max stations returned from search
#define MAX_ENTRIES_PLS       2 // max stations to resolve from *.pls file
#define ENT_NAME_LEN         32
#define ENT_URL_LEN         128

// --------------------------------- RTOS --------------------------------- //
#define PRI_PLAYER              3  // handles player's buffer ack & playback stop
#define PRI_PLAYER_STAT         2  // handles player's statistics updates
#define PRI_FIFO_QUEUE          3  // handles message passing to for ex. player task
#define PRI_LWIP_TCPIP          5
#define PRI_LIST_LOADER         2  // quickly load favs on startup
#define PRI_DISPLAY_TICKER      1  // updates scrollable texts
#define PRI_INIT                2  // don't get starved by wifi best
#define PRI_WIFI_CONN           1
#define PRI_WIFI_BEST           1
#define PRI_INPUT               1
#define PRI_SD                  1

#include <ffconf.h>
#include <FreeRTOSConfig.h>

#define MIN_FREE_STACK            100
#define RESV_STACK_FATFS          (((FF_MAX_LFN+1)*2 + ((FF_MAX_LFN + 44U) / 15 * 32)) / sizeof(configSTACK_DEPTH_TYPE))
#define RESV_STACK_LFSACC         350
#define STACK_PLAYER              (200 + MIN_FREE_STACK + RESV_STACK_FATFS)
#define STACK_PLAYER_STAT         (164 + MIN_FREE_STACK)
#define STACK_FIFO_QUEUE          configMINIMAL_STACK_SIZE // currently unused
#define STACK_LIST_LOADER         (180 + MIN_FREE_STACK + RESV_STACK_FATFS)
#define STACK_DISPLAY_TICKER      (160 + MIN_FREE_STACK)
#define STACK_INIT                (225 + MIN_FREE_STACK)
#define STACK_WIFI_CONN           (250 + MIN_FREE_STACK)
#define STACK_WIFI_BEST           (410 + MIN_FREE_STACK + RESV_STACK_LFSACC)
#define STACK_INPUT               (273 + MIN_FREE_STACK)
#define STACK_SD                  (226 + MIN_FREE_STACK)

// --------------------------------- Flash --------------------------------- //
#define LITTLEFS_SIZE             (1<<16) // 64K, base 0x101F0000
#define LITTLEFS_CACHES           1024    // 3 blocks of this size
#define PATH_FAVOURITES           "/favourites.m3u"
#define PATH_WIFI                 "/wifi.m3u"

// --------------------------------- WiFi --------------------------------- //
#define WIFI_SSID_MAX_LEN           32
#define WIFI_PWD_MAX_LEN            64
#define WIFI_CONN_SCAN_TIMEOUT_MS   5000
#define WIFI_CONN_TRY_TIMEOUT_MS    5000
#define WIFI_CONN_TRIES             3
#define WIFI_SCAN_POLL_MS           100
#define WIFI_SCAN_POLL_MAX_TIMES    100 // 10s max

// --------------------------------- SD --------------------------------- //
#define FATFS_MAX_PATH_LEN        (128*4)
#define SD_CD_DEBOUNCE_MS         500

// --------------------------------- LCD --------------------------------- //
#define LCD_SCROLLED_TEXTS_MAX          4
#define LCD_SCROLLED_TEXTS_LEN_MAX      64
#define LCD_TICK_INTERVAL_MS            50
#define LCD_BL_TIMEOUT_MS               30000
#define LCD_MAX_KB_INPUT                (WIFI_PWD_MAX_LEN)

// --------------------------------- Buttons --------------------------------- //
#define BTN_DEBOUNCE_MS         20
#define BTN_REPEAT_MS           500
#define BTN_REPEAT_PER_SECOND   26      // this is also a timescale for the task loop

// --------------------------------- PINOUTS --------------------------------- //
#ifdef PINOUT_PROTOTYPE
#define STDIO_UART_ID   0
#define STDIO_BAUD      115200
#define STDIO_TX        16
#define STDIO_RX        17

#define I2S_CLK_CHANNEL_BASE 18 // 18-clk 19-channel
#define I2S_DATA             20
#define AMP_MUTE             -1

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

#define ADC_VCC     -1
#define ADC_VBATT   -1
#endif

#ifdef PINOUT_V1
#define STDIO_UART_ID   0
#define STDIO_BAUD      115200
#ifdef PINOUT_V1_0
#define STDIO_TX        0 // blown pin (oops) forces us
                          // to use different UART pin
#else
#define STDIO_TX        16 // schematic
#endif
#define STDIO_RX        17

#define I2S_CLK_CHANNEL_BASE  8 // 8-clk 9-channel
#define I2S_DATA              7
#define AMP_MUTE              26

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

#define ADC_VCC         28
#define ADC_VCC_MUL      2
#define ADC_VBATT       27
#define ADC_VBATT_MUL    2
#endif
