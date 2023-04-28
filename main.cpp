#include <cstdio>
#include <cstdlib>
#include <pico/stdlib.h>
#include <hardware/dma.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include <pico/multicore.h>
#include <cstring>
#include <vector>
#include <string>

#include "i2s.pio.h"

#include "f_util.h"
#include "ff.h"
#include "hw_config.h"

#include "config.hpp"
#include "libs/waveheader.hpp"
#include "libs/mp3.hpp"

const uint PIN_DBG = 13;
#define DBG_ON() gpio_put(PIN_DBG, true)
#define DBG_OFF() gpio_put(PIN_DBG, false)

const uint I2C_CLK_CHANNEL_BASE = 18; // 18-clk 19-channel
const uint I2C_DATA = 20;

volatile bool a_done_irq = false;
volatile bool b_done_irq = false;

/*
 * PIO handles endianness for us, sees data as:
 * audio_pcm_bytes index offset 3 2 1 0
 * This swaps channels so PIO expects to get data:
 *  byte: llo lhi rlo rhi (as read from WAV)
 *   int: rhi rlo lhi llo (as interpreted by uint32_t)
 *
 * Additionally, for channel manipulation you can cast audio to
 * uint16_t* to provide a 16-bit view of which:
 *  offset 0 is left channel
 *  offset 1 is right channel
 */
uint32_t audio_pcm[BUF_PCM_SIZE_32BIT];
uint32_t* const audio_pcm_half = audio_pcm + BUF_PCM_HALF_32BIT;

// PIO
PIO pio;
uint sm;

// DMA
int dma_channel_a;
int dma_channel_b;

void dma_channelA() {
    a_done_irq = true;
    dma_channel_set_read_addr(dma_channel_a, audio_pcm, false);
}

void dma_channelB() {
    b_done_irq = true;
    dma_channel_set_read_addr(dma_channel_b, audio_pcm + BUF_PCM_HALF_32BIT, false);
}

void dma_irq0() {
    if (dma_channel_get_irq0_status(dma_channel_a)) {
        dma_channel_acknowledge_irq0(dma_channel_a);
        dma_channelA();
    }

    if (dma_channel_get_irq0_status(dma_channel_b)) {
        dma_channel_acknowledge_irq0(dma_channel_b);
        dma_channelB();
    }
}

void configure_pio_tx_dma(int dma_chan, uint32_t* data, uint count, int chain_to) {
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
    channel_config_set_chain_to(&c, chain_to);

    dma_channel_set_irq0_enabled(dma_chan, true);

    dma_channel_configure(
            dma_chan,
            &c,
            &pio->txf[sm],
            data,
            count,
            false
    );
}



void fs_err(FRESULT fr, const char* tag) {
    panic("%s: %s (id=%d)\n", tag, FRESULT_str(fr), fr);
}

void dma_chain_disable(int dma_chan) {
    dma_hw->ch[dma_chan].al1_ctrl = (dma_hw->ch[dma_chan].al1_ctrl & ~DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS) | (dma_chan << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB);
}

void dma_chain_enable(int dma_chan, int chain_to) {
    dma_hw->ch[dma_chan].al1_ctrl = (dma_hw->ch[dma_chan].al1_ctrl & ~DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS) | (chain_to << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB);
}

void reinterpret_buffer(uint8_t* data, uint data_len) {

    /*int16_t* chan = (int16_t*)data;
    uint channel_len = data_len / 2;*/

    // swap channels (1% cost)
    /*for (int i=0; i<channel_len; i+=2) {
        uint16_t tmp = chan[i];
        chan[i] = chan[i+1];
        chan[i+1] = tmp;
    }*/

    // mute left (almost free)
    /*for (int i=0; i<channel_len; i+=2) {
        chan[i] = 0;
    }*/

    // int division (free)
    /*for (int i=0; i<channel_len; i+=2) {
        chan[i] /= 2;
        chan[i+1] /= 2;
    }*/

    // float division (10%)
    /*for (int i=0; i<channel_len; i+=2) {
        chan[i] /= 1.5f;
        chan[i+1] /= 1.5f;
    }*/
}

/*void mp3_benchmark(const char* path) {
    MP3 mp3(path);

    // benchmark 22ms
    ulong start = time_us_64();
    DBG_ON();
    mp3->decode_up_to_n_frames((int16_t *) audio_pcm, BUF_PCM_SIZE_FRAMES);
    mp3->decode_up_to_n_frames((int16_t *) audio_pcm, BUF_PCM_SIZE_FRAMES);
    DBG_OFF();
    ulong end = time_us_64();

    printf("load took %f ms / 1 frame\n", (end - start) / 1000.0f / (BUF_PCM_SIZE_FRAMES*2));
    while(1) {
        int chr = getchar_timeout_us(0);
        if (chr > 0) {
            return;
        }
    }
}*/

MP3* mp3;

void core1_entry() {
    while (!mp3->get_decode_finished())
        mp3->watch_decode(a_done_irq, b_done_irq);
}

void play_mp3(const char* path) {
    printf("\nplaying: %s as MP3 file\n", path);

    mp3 = new MP3(path, audio_pcm);

    i2s_program_set_bit_freq(pio, sm, mp3->get_bit_freq());

    if (mp3->needs_core1()) {
        multicore_reset_core1();
        multicore_launch_core1(core1_entry);
    }

    printf("dma start\n");

    // setup chaining
    dma_chain_enable(dma_channel_a, dma_channel_b);
    dma_chain_enable(dma_channel_b, dma_channel_a);

    // start playback
    dma_channel_start(dma_channel_a);

    while (!mp3->get_eof()) {

        mp3->watch_file_buffer();
        mp3->watch_timer();

        int chr = getchar_timeout_us(0);
        if (chr > 0) {
            mp3->user_abort();
            break;
        }
    }

    printf("\nfinished file reading.\n");

    while (!mp3->get_decode_finished());

    printf("\nfinished decode.\n");

    if (mp3->decode_finished_by_A()) {
        // a channel finished, and it's data reload triggerred EOF
        // wait for another a channel finish, disable b channel firing
        dma_chain_disable(dma_channel_a);
        while (!a_done_irq);
        a_done_irq = false;
    }
    else if (mp3->decode_finished_by_B()) {
        // b finished, disable it's chaining to a, wait for finish
        dma_chain_disable(dma_channel_b);
        while (!b_done_irq);
        b_done_irq = false;
    }
    else {
        // user abort
        dma_chain_disable(dma_channel_a);
        dma_chain_disable(dma_channel_b);
        dma_channel_abort(dma_channel_a);
        dma_channel_abort(dma_channel_b);
        a_done_irq = false;
        b_done_irq = false;
    }

    delete mp3;

    pio_sm_put_blocking(pio, sm, 0);

    puts("dma channels stopped.");
}

void play_wav(const char* path) {
    printf("playing: %s as WAV file\n\n", path);

    FRESULT fr;
    FIL fp;

    fr = f_open(&fp, path, FA_READ);
    if (fr != FR_OK) {
        fs_err(fr, "f_open");
    }

    WaveHeader hdr;
    uint read;

    fr = hdr.read(&fp, &read);
    if (fr != FR_OK) {
        fs_err(fr, "f_read header");
    }

    printf("header size: %u, read %u\n", WAVE_HEADER_SIZE, read);

    if (hdr.check()) {
        panic("wrong header\n");
    }

    hdr.print();
    i2s_program_set_bit_freq(pio, sm, hdr.get_bit_freq());

    // preload buffer
    fr = f_read(&fp, (uint8_t*)audio_pcm, BUF_PCM_SIZE_BYTES, &read);
    if (fr != FR_OK) {
        fs_err(fr, "f_read preload");
    }

    printf("dma start\n");

    // setup chaining
    dma_chain_enable(dma_channel_a, dma_channel_b);
    dma_chain_enable(dma_channel_b, dma_channel_a);

    // start playback
    dma_channel_start(dma_channel_a);

    uint sum_bytes_read = 0;
    uint last_seconds = 0;
    bool eof = false;
    bool a_done_prv, b_done_prv;

    while (!eof) {
        if (a_done_irq) {
            a_done_irq = false;
            a_done_prv = true;
            // channel A done (first one)
            // reload first half of the buffer
            DBG_ON();
            f_read(&fp, (uint8_t*)audio_pcm, BUF_PCM_HALF_BYTES, &read);
            reinterpret_buffer((uint8_t*)audio_pcm, BUF_PCM_HALF_BYTES);
            DBG_OFF();
        }

        if (b_done_irq) {
            b_done_irq = false;
            b_done_prv = true;
            // channel B done (second one)
            // reload second half of the buffer
            DBG_ON();
            f_read(&fp, (uint8_t*)audio_pcm_half, BUF_PCM_HALF_BYTES, &read);
            reinterpret_buffer((uint8_t*)audio_pcm_half, BUF_PCM_HALF_BYTES);
            DBG_OFF();
        }

        if (a_done_prv || b_done_prv) {
            sum_bytes_read += read;

            if (read < BUF_PCM_HALF_BYTES) {
                eof = true;
            }
            else {
                // only clear on non-EOF
                // these are later used for dma channel abortion
                a_done_prv = false;
                b_done_prv = false;
            }
        }

        uint seconds = hdr.byte_to_sec(sum_bytes_read);

        if (seconds != last_seconds) {
            printf("%02d:%02d / %02d:%02d\r",
                   seconds/60, seconds%60,
                   hdr.get_duration()/60, hdr.get_duration()%60);

            last_seconds = seconds;
        }

        int chr = getchar_timeout_us(0);
        if (chr > 0) {
            break;
        }
    }

    printf("\nfinished file reading.\n");

    if (a_done_prv) {
        // a channel finished, and it's data reload triggerred EOF
        // wait for another a channel finish, disable b channed firing
        dma_chain_disable(dma_channel_a);
        while (!a_done_irq);
        a_done_irq = false;
    }
    else if (b_done_prv) {
        // b finished, disable it's chaining to a, wait for finish
        dma_chain_disable(dma_channel_b);
        while (!b_done_irq);
        b_done_irq = false;
    }
    else {
        // user abort
        dma_chain_disable(dma_channel_a);
        dma_chain_disable(dma_channel_b);
        dma_channel_abort(dma_channel_a);
        dma_channel_abort(dma_channel_b);
        a_done_irq = false;
        b_done_irq = false;
    }

    puts("dma channels stopped.");
}

FRESULT scan_files(char* path, std::vector<std::string>& files) {
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);               /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            i = strlen(path);
            sprintf(&path[i], "/%s", fno.fname);

            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                res = scan_files(path, files);             /* Enter the directory */
                if (res != FR_OK) break;
            } else {                                       /* It is a file. */
                // printf("found  %s\n", path);
                files.emplace_back(path);
            }

            path[i] = 0;
        }
        f_closedir(&dir);
    }

    return res;
}

enum FileType {
    WAV,
    MP3,
    UNSUPPORTED
};

FileType get_file_type(const char* filepath) {
    const char *extension = filepath + strlen(filepath) - 4;

    if (strcmp(extension, ".mp3") == 0)
        return MP3;

    else if ((strcmp(extension, ".wav") == 0) || (strcmp(extension, "wave") == 0))
        return WAV;

    else
        return UNSUPPORTED;
}

int main() {

    // UART on USB
    stdio_usb_init();

    set_sys_clock_khz(140000, true);

    sleep_ms(2000);
    printf("\n\nHello usb pico-radio!\n");
    printf("sys clock: %lu MHz\n", clock_get_hz(clk_sys)/1000000);
    printf("MP3 buffer size: %d bytes + %d bytes hidden\n", BUF_MP3_SIZE_BYTES, BUF_HIDDEN_MP3_SIZE_BYTES);
    printf("PCM buffer size: %d bytes\n", BUF_PCM_SIZE_BYTES);
    puts("");

    // IO
    gpio_init(PIN_DBG);
    gpio_set_dir(PIN_DBG, GPIO_OUT);
    DBG_OFF();


    // PIO I2S configuration
    pio = pio0;

    // load program to <pio> block
    uint offset = pio_add_program(pio, &i2s_program);

    // claim unused state machine
    sm = pio_claim_unused_sm(pio, true);

    // configure & start program on <sm> machine at <pio> PIO block
    i2s_program_init(pio, sm, offset, I2C_CLK_CHANNEL_BASE, I2C_DATA);
    puts("PIO I2S configuration done");


    // DMA configuration
    irq_add_shared_handler(DMA_IRQ_0, dma_irq0, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_a = dma_claim_unused_channel(true);
    dma_channel_b = dma_claim_unused_channel(true);

    configure_pio_tx_dma(dma_channel_a, audio_pcm, BUF_PCM_HALF_32BIT, dma_channel_b);
    configure_pio_tx_dma(dma_channel_b, audio_pcm + BUF_PCM_HALF_32BIT, BUF_PCM_HALF_32BIT, dma_channel_a);
    puts("DMA configuration done");

    
    // FS configuration
    FRESULT fr;

    sd_card_t *pSD = sd_get_by_num(0);

    fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) {
        fs_err(fr, "f_mount");
    }

    puts("mount ok\n");

    char path[1024] = "/";
    std::vector<std::string> files;
    scan_files(path, files);

    while (1) {
        for (uint i=0; i<files.size(); i++) {
            printf("[%02d] %s\n", i+1, files[i].c_str());
        }

        uint choice;
        while (1) {
            scanf("%d", &choice);
            if ((choice < 1) || (choice > files.size())) {
                puts("invalid number");
            }
            else if (get_file_type(files[choice - 1].c_str()) == UNSUPPORTED) {
                puts("unsupported format. Supported: wav, wave, mp3");
            }
            else {
                break;
            }
        }

        const char *filepath = files[choice - 1].c_str();

        switch (get_file_type(filepath)) {
            case WAV:
                play_wav(filepath);
                break;

            case MP3:
                play_mp3(filepath);
                break;

            case UNSUPPORTED:
                break;
        }

        printf("\033[2J"); // clear screen
    }
}
