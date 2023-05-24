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
#include "formatmp3.hpp"
#include "formatwav.hpp"
#include "decodebase.hpp"
#include "decodefile.hpp"
#include "decodestream.hpp"

// wifi
#include <pico/cyw43_arch.h>

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

enum class FileType {
    WAV,
    MP3,
    RADIO,
    UNSUPPORTED
};

volatile CircularBuffer raw_buf(BUF_MP3_SIZE_BYTES, BUF_HIDDEN_MP3_SIZE_BYTES);

FormatMP3 format_mp3(raw_buf);
FormatWAV format_wav(raw_buf);

DecodeFile dec_file(
    audio_pcm,
    BUF_PCM_SIZE_32BIT,
    a_done_irq,
    b_done_irq);

DecodeStream dec_stream(
    audio_pcm,
    BUF_PCM_SIZE_32BIT,
    a_done_irq,
    b_done_irq);

DecodeBase* dec;

void dma_start() {
    printf("dma start\n");

    // setup chaining
    dma_chain_enable(dma_channel_a, dma_channel_b);
    dma_chain_enable(dma_channel_b, dma_channel_a);

    // start playback
    dma_channel_start(dma_channel_a);
}

void core1_entry() {
    dec->core1_init();

    i2s_program_set_bit_freq(pio, sm, dec->bit_freq());
    dma_start();

    while (dec->core1_loop());
}

void play(const char* path, FileType type) {
    printf("\nplaying: %s as MP3 file\n", path);

    // TODO add WAV
    switch (type) {
        case FileType::WAV:
            dec = &dec_file;
            dec->begin(path, &format_wav);
            break;

        case FileType::MP3:
            dec = &dec_file;
            dec->begin(path, &format_mp3);
            break;

        case FileType::RADIO:
            dec = &dec_stream;
            dec->begin(path, &format_mp3);
            break;

        default:
            puts("format unsupported");
            return;
    }

    dec->core0_init();

    multicore_reset_core1();
    multicore_launch_core1(core1_entry);

    while (dec->core0_loop()) {
        int chr = getchar_timeout_us(0);
        if (chr > 0) {
            dec->user_abort();
            break;
        }
    }

    printf("\nfinished file reading.\n");

    dec->core0_end();
    multicore_reset_core1();

    printf("\nfinished decode.\n");

    if (dec->decode_finished_by_A()) {
        // a channel finished, and it's data reload triggerred EOF
        // wait for another a channel finish, disable b channel firing
        dma_chain_disable(dma_channel_a);
        while (!a_done_irq);
        a_done_irq = false;
    }
    else if (dec->decode_finished_by_B()) {
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

    dec->end();

    pio_sm_put_blocking(pio, sm, 0);
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

FileType get_file_type(const char* filepath) {

    if (strncmp(filepath, "http", 4) == 0) {
        return FileType::RADIO;
    }

    const char *extension = filepath + strlen(filepath) - 4;

    if (strcmp(extension, ".mp3") == 0)
        return FileType::MP3;

    else if ((strcmp(extension, ".wav") == 0) || (strcmp(extension, "wave") == 0))
        return FileType::WAV;

    else
        return FileType::UNSUPPORTED;
}

void print_mem_usage() {
    int size_pcm = sizeof(audio_pcm);
    printf("PCM buffer size:     %d\n", size_pcm);
    int size_raw = sizeof(raw_buf) + raw_buf.size + raw_buf.size_hidden;
    printf("RAW buffer size:     %d\n", size_raw);

    int size_mp3 = sizeof(format_mp3);
    printf("MP3 format size:     %d\n", size_mp3);
    int size_wav = sizeof(format_wav);
    printf("WAV format size:     %d\n", size_wav);

    int size_file = sizeof(dec_file);
    printf("File decoder size:   %d\n", size_file);
    int size_stream = sizeof(dec_stream);
    printf("Stream decoder size: %d\n", size_stream);

    printf("Total: %d\n",
           size_pcm + size_raw +\
           size_mp3 + size_wav +\
           size_file + size_stream);
}

int main() {

    // set_sys_clock_khz(140000, true);
    set_sys_clock_khz(180000, true);

    // UART on USB
    // stdio_usb_init();
    // UART on 0/1 and USB
    stdio_init_all();

    //sleep_ms(2000);
    printf("\n\nHello usb pico-radio!\n");
    printf("sys clock: %lu MHz\n", clock_get_hz(clk_sys)/1000000);
    print_mem_usage();
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

    // WiFi configuration
    int err;
    err = cyw43_arch_init();
    if (err) {
        printf("wifi arch init error code %d\n", err);
        while(1);
    }

    cyw43_arch_enable_sta_mode();
    const char* WIFI_SSID = "Bapplejems";
    const char* WIFI_PASSWORD = "ForThosE4bOut";
    // const char* WIFI_SSID = "NLP";
    // const char* WIFI_SSID = "NPC";
    // const char* WIFI_SSID = "MyNet";
    // const char* WIFI_SSID = "BPi";
    // const char* WIFI_PASSWORD = "bequick77";
    // const char* WIFI_SSID = "NorbertAP";
    // const char* WIFI_PASSWORD = "fearofthedark";

    printf("Connecting to Wi-Fi...\n");
    int con_res = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
    if (con_res) {
        printf("connection failed code %d\n", con_res);
        while(1);
    } else {
        printf("Connected.\n");
    }

    cyw43_arch_lwip_begin();
    const ip4_addr_t* addr;
    do {
        addr = netif_ip4_addr(netif_list);
    } while (ip4_addr_isany_val(*addr));

    printf("got ip: %s\n", ip4addr_ntoa(addr));
    cyw43_arch_lwip_end();

    char path[1024] = "/";
    std::vector<std::string> files;
    scan_files(path, files);

    // radio
    files.emplace_back("http://stream.rcs.revma.com/an1ugyygzk8uv"); // Radio 357
    files.emplace_back("http://rmfstream1.interia.pl:8000/rmf_fm");  // RMF FM
    files.emplace_back("http://zt03.cdn.eurozet.pl/zet-tun.mp3");   // Radio Zet
    files.emplace_back("http://stream.streambase.ch/radio32/mp3-192/direct");   // Radio 32 Switzerland

    while (1) {
        for (uint i=0; i<files.size(); i++) {
            printf("[%02d] %s\n", i+1, files[i].c_str());
        }

        const char* filepath;
        FileType type;

        while (1) {
            uint choice;
            scanf("%d", &choice);
            if ((choice < 1) || (choice > files.size())) {
                puts("invalid number");
            }
            else {
                filepath = files[choice - 1].c_str();
                type = get_file_type(filepath);

                if (type == FileType::UNSUPPORTED) {
                    puts("unsupported format. Supported: wav, wave, mp3, http(s)");
                }
                else {
                    // valid & supported
                    break;
                }
            }
        }

        play(filepath, type);

        printf("\033[2J"); // clear screen
    }
}
