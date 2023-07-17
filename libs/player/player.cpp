#include <player.hpp>

#include <pico/multicore.h>
#include <hardware/dma.h>
#include <hardware/pio.h>

#include <FreeRTOS.h>
#include <task.h>

#include <helix_static.h>

#include <config.hpp>
#include <i2s.pio.h>
#include <circularbuffer.hpp>
#include <buffers.hpp>
#include <formatmp3.hpp>
#include <formatwav.hpp>
#include <decodebase.hpp>
#include <decodefile.hpp>
#include <decodestream.hpp>

static volatile bool a_done_irq = false;
static volatile bool b_done_irq = false;

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
static uint32_t audio_pcm[BUF_PCM_SIZE_32BIT];

// PIO
static PIO pio;
static uint sm;

// DMA
static volatile int dma_channel_a;
static volatile int dma_channel_b;

// Playback
static char filepath[1024];

HELIX_STATIC_DECLARE();
static FormatMP3 format_mp3(get_cbuf(), (HMP3Decoder)&mp3DecInfo);
static FormatWAV format_wav(get_cbuf());

[[noreturn]] static void core1_entry();

static DecodeFile dec_file(
        audio_pcm,
        BUF_PCM_SIZE_32BIT,
        a_done_irq,
        b_done_irq,
        get_cbuf(),
        core1_entry);

static DecodeStream dec_stream(
        audio_pcm,
        BUF_PCM_SIZE_32BIT,
        a_done_irq,
        b_done_irq,
        get_cbuf(),
        get_http_client(),
        core1_entry);

static DecodeBase* dec;

// player tasks
xTaskHandle player_task_h;
xTaskHandle player_stat_task_h;

// callbacks
static void* cb_arg;
static player_cb_fn fail_cb;
static player_cb_fn_dec upd_cb;

// task to notify when playback really ends
xTaskHandle task_to_notify_end;

/* ------------------------------- INIT FUNCTIONS -------------------------------*/
static void dma_irq_A() {
    a_done_irq = true;
    dma_channel_set_read_addr(dma_channel_a, audio_pcm, false);
}

static void dma_irq_B() {
    b_done_irq = true;
    dma_channel_set_read_addr(dma_channel_b, audio_pcm + BUF_PCM_HALF_32BIT, false);
}

static void dma_irq0() {
    if (dma_channel_get_irq0_status(dma_channel_a)) {
        dma_channel_acknowledge_irq0(dma_channel_a);
        dma_irq_A();
    }

    if (dma_channel_get_irq0_status(dma_channel_b)) {
        dma_channel_acknowledge_irq0(dma_channel_b);
        dma_irq_B();
    }
}

static void configure_pio_tx_dma(int dma_chan, uint32_t* data, uint count, int chain_to) {
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

void player_init() {
    // PIO I2S configuration
    pio = pio0;

    // load program to <pio> block
    uint offset = pio_add_program(pio, &i2s_program);

    // claim unused state machine
    sm = pio_claim_unused_sm(pio, true);

    // configure & start program on <sm> machine at <pio> PIO block
    i2s_program_init(pio, sm, offset, I2S_CLK_CHANNEL_BASE, I2S_DATA);
    puts("PIO I2S configuration done");


    // DMA configuration
    // TODO move dma irq to core1
    irq_add_shared_handler(DMA_IRQ_0, dma_irq0, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_a = dma_claim_unused_channel(true);
    dma_channel_b = dma_claim_unused_channel(true);

    configure_pio_tx_dma(dma_channel_a, audio_pcm, BUF_PCM_HALF_32BIT, dma_channel_b);
    configure_pio_tx_dma(dma_channel_b, audio_pcm + BUF_PCM_HALF_32BIT, BUF_PCM_HALF_32BIT, dma_channel_a);
    puts("DMA configuration done");

    HELIX_STATIC_INIT(mp3DecInfo, fh, si, sfi, hi, di, mi, sbi);
    puts("Decoder begin done");
}


/* ------------------------------- PLAYING FUNCTIONS -------------------------------*/
static void dma_chain_disable(int dma_chan) {
    dma_hw->ch[dma_chan].al1_ctrl = (dma_hw->ch[dma_chan].al1_ctrl & ~DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS) | (dma_chan << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB);
}

static void dma_chain_enable(int dma_chan, int chain_to) {
    dma_hw->ch[dma_chan].al1_ctrl = (dma_hw->ch[dma_chan].al1_ctrl & ~DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS) | (chain_to << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB);
}

static void dma_start() {
    puts("dma start");

    // setup chaining
    dma_chain_enable(dma_channel_a, dma_channel_b);
    dma_chain_enable(dma_channel_b, dma_channel_a);

    // start playback
    dma_channel_start(dma_channel_a);
}

[[noreturn]] static void core1_entry() {
    dec->dma_preload();

    i2s_program_set_bit_freq(pio, sm, dec->bit_freq());
    dma_start();

    while (!dec->decode_finished()) {
        dec->dma_watch();
    }

    // core1 never returns
    // terminated by <multicore_reset_core1>
    while(true);
}

static void player_task(void* arg) {

    FileType type = filetype_from_name(filepath);
    int r;
    bool failed = false;

    // init decoder & format
    // this may open files + preload data/establish a connection/etc.
    switch (type) {
        case FileType::WAV:
            dec = &dec_file;
            dec->begin(filepath, &format_wav);
            break;

        case FileType::MP3:
            dec = &dec_file;
            dec->begin(filepath, &format_mp3);
            break;

        case FileType::RADIO:
            dec = &dec_stream;
            dec->begin(filepath, &format_mp3);
            break;

        default:
            puts("format unsupported");
            failed = true;
            goto clean_up;
    }

    r = dec->setup();
    if (r < 0) {
        puts("setup failed");
        failed = true;
        // playback didn't start
        // just clean-up
        goto clean_up;
    }

    if (player_stat_task_h)
        // start displaying stats
        xTaskNotifyGive(player_stat_task_h);

    // this starts core1
    // and blocks for the time of playback
    r = dec->play();
    if (r < 0) {
        puts("play failed");
        failed = true;
    }

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
        // user abort / failure
        dma_chain_disable(dma_channel_a);
        dma_chain_disable(dma_channel_b);
        dma_channel_abort(dma_channel_a);
        dma_channel_abort(dma_channel_b);
        a_done_irq = false;
        b_done_irq = false;
    }

    puts("dma channels stopped.");

clean_up:
    pio_sm_put_blocking(pio, sm, 0);

    dec->end();
    dec = nullptr;

    if (failed && fail_cb)
        fail_cb(cb_arg);

    if (task_to_notify_end)
        xTaskNotifyGive(task_to_notify_end);

    vTaskDelete(nullptr);
}

static void player_update_stats() {
    const int current = dec->current_time();
    const int duration = dec->duration();

    printf("%02d:%02d / %02d:%02d   decode %2d%%   health %2d%%  ",
           current/60, current%60,
           duration/60, duration%60,
           dec->core1_usage(),
           dec->buf_health()
    );

    printf("rtos ram used: %2d%%  max %2d%%  player free stack: %ld  ",
           (configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize()) * 100 / configTOTAL_HEAP_SIZE,
           (configTOTAL_HEAP_SIZE - xPortGetMinimumEverFreeHeapSize()) * 100 / configTOTAL_HEAP_SIZE,
           uxTaskGetStackHighWaterMark(nullptr));

    // Lwip stats (all)
    // to enable/disable see DEBUG* in lwipopts.h
    // stats_display();
    // pbuf stats only
    printf("pbuf max used %2d avail %2d err %2d  ",
           STATS_GET(memp)[MEMP_PBUF_POOL]->max,
           PBUF_POOL_SIZE,
           STATS_GET(memp)[MEMP_PBUF_POOL]->err);


    puts("");

    // char stats[512];
    // vTaskGetRunTimeStats(stats);
    // puts(stats);
    // puts("");

    // external stats
    upd_cb(cb_arg, dec);
}

static void player_stat_task(void* arg) {

    while (!player_is_started())
        // wait for playback to start
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    TickType_t last_wake;
    last_wake = xTaskGetTickCount();

    int last_current = -1;

    while (player_is_started()) {
        const int current = dec->current_time();

        if (current != last_current || current < 1) {
            player_update_stats();
            last_current = current;
        }

        xTaskDelayUntil(&last_wake,
                        500 / portTICK_PERIOD_MS);
    }

    vTaskDelete(nullptr);
}

void player_start(const char* path, void* cb_arg_, player_cb_fn fail_cb_, player_cb_fn_dec upd_cb_) {
    printf("\nplaying: %s as %s file\n", path, filetype_from_name_string(path));

    strcpy(filepath, path);
    cb_arg = cb_arg_;
    fail_cb = fail_cb_;
    upd_cb = upd_cb_;
    task_to_notify_end = nullptr;

    xTaskCreate(
            player_task,
            "player",
            STACK_PLAYER,
            nullptr,
            PRI_PLAYER,
            &player_task_h);

    xTaskCreate(
            player_stat_task,
            "player stat",
            STACK_PLAYER_STAT,
            nullptr,
            PRI_PLAYER_STAT,
            &player_stat_task_h);
}

void player_stop() {
    if (!player_is_started())
        return;

    dec->stop_playback();
}

bool player_is_started() {
    return dec;
}

void player_wait_for_end() {
    if (!player_is_started())
        return;

    // TODO player closing too fast errors
    // player calls error callback -13 when stopped too early
    // see player_stop
    task_to_notify_end = xTaskGetCurrentTaskHandle();
    ulTaskNotifyTake(pdTRUE, PLAYER_END_TIMEOUT_MS / portTICK_PERIOD_MS);
    task_to_notify_end = nullptr;
}
