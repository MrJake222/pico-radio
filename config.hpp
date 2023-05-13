#include "lwipopts.h"

// MP3 compressed data buffer
#define BUF_HIDDEN_MP3_SIZE_FRAMES     2
#define BUF_MP3_SIZE_FRAMES            16
#define BUF_MP3_SIZE_BYTES_PER_FRAME   1024
#define BUF_HIDDEN_MP3_SIZE_BYTES      (BUF_HIDDEN_MP3_SIZE_FRAMES * BUF_MP3_SIZE_BYTES_PER_FRAME)
#define BUF_MP3_SIZE_BYTES             (BUF_MP3_SIZE_FRAMES * BUF_MP3_SIZE_BYTES_PER_FRAME)

// for MP3 playing
#define MP3_SAMPLES_PER_FRAME       1152 // sample = 2 channels * 16 bit = 32bit word
#define BUF_PCM_SIZE_FRAMES         8
#define BUF_PCM_SIZE_32BIT          (MP3_SAMPLES_PER_FRAME * BUF_PCM_SIZE_FRAMES)
#define BUF_PCM_HALF_32BIT          (BUF_PCM_SIZE_32BIT / 2)

// for WAV playing
#define BUF_PCM_SIZE_BYTES     (BUF_PCM_SIZE_32BIT * 4)
#define BUF_PCM_HALF_BYTES     (BUF_PCM_SIZE_BYTES / 2)

// HTTP
#define HTTP_TMP_BUF_SIZE_BYTES     1024
#define HTTP_DATA_BUF_SIZE_BYTES    TCP_WND