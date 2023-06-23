#include <lwipopts.h>

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
#define BUF_PCM_HALF_BYTES     (BUF_PCM_SIZE_BYTES / 2)

// HTTP
#define HTTP_DATA_BUF_SIZE_BYTES    TCP_WND

// SD
#define SD_ENABLE 1
// Pin data
// I2S
#define I2S_CLK_CHANNEL_BASE 18 // 18-clk 19-channel
#define I2S_DATA             20
// LCD
#define LCD_SPI     1
#define LCD_SCK     10
#define LCD_TX      11
#define LCD_CS      9
#define LCD_RST     6
#define LCD_DC      8
#define LCD_BL      7
// Buttons
#define BTN_UP      12
#define BTN_DOWN    16
#define BTN_LEFT    15
#define BTN_RIGHT   13
#define BTN_CENTER  14
