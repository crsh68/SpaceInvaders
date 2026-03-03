#include <Arduino.h>
#include <driver/i2s.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <r65emu.h>
#include <ports.h>
#include <i8080.h>
#include <memory.h>

#include "io.h"
#include "config.h"
#include "sounds.h"

// ── I2S Audio (MAX98357A) ──────────────────────────────────────
#define I2S_BCLK   7
#define I2S_LRCLK  20
#define I2S_DIN    21
#define AUDIO_FREQ 11127

// Mali DMA buffer — audio task ga puni kontinuirano
#define DMA_BUF_COUNT  4
#define DMA_BUF_LEN    64
#define FEED_CHUNK     DMA_BUF_LEN

// ── Audio kanali ──────────────────────────────────────────────

struct Channel {
    const uint8_t *data = nullptr;
    size_t         size = 0;
    size_t         pos  = 0;

    void play(const uint8_t *d, size_t s) {
        data = d; size = s; pos = 0;
    }

    inline int16_t next() {
        if (!data || pos >= size) { data = nullptr; return 0; }
        return ((int16_t)data[pos++] - 128) << 7;
    }
};

struct LoopChannel {
    const uint8_t *data   = nullptr;
    size_t         size   = 0;
    size_t         pos    = 0;
    bool           active = false;

    void start(const uint8_t *d, size_t s) {
        data = d; size = s;
        if (!active) pos = 0;
        active = true;
    }

    void stop() { active = false; }

    inline int16_t next() {
        if (!active || !data || size == 0) return 0;
        if (pos >= size) pos = 0;
        return ((int16_t)data[pos++] - 128) << 7;
    }
};

static Channel     _chEffects;
static Channel     _chBasehit;
static Channel     _chWalk;
static LoopChannel _chUfo;

// Mutex koji štiti kanale između audio taska i main loop-a (out())
static SemaphoreHandle_t _audioMutex;

static int16_t _feedBuf[FEED_CHUNK * 2];

// ── Audio task — izvršava se neovisno od loop() ───────────────
static void audioTask(void *param) {
    while (true) {
        // Uzmi mutex samo za generiranje samplea (kratko)
        xSemaphoreTake(_audioMutex, portMAX_DELAY);
        for (int i = 0; i < FEED_CHUNK; i++) {
            int32_t mix = (int32_t)_chEffects.next()
                        + (int32_t)_chBasehit.next()
                        + (int32_t)_chWalk.next()
                        + (int32_t)_chUfo.next();
            if (mix >  32767) mix =  32767;
            if (mix < -32768) mix = -32768;
            int16_t s = (int16_t)mix;
            _feedBuf[i * 2]     = s;
            _feedBuf[i * 2 + 1] = s;
        }
        xSemaphoreGive(_audioMutex);

        // i2s_write blokira dok DMA ne prihvati chunk — ovo je srce taska
        // Blokira ~5.7ms (64 samplea @ 11127 Hz), čime diktira ritam taska
        size_t written = 0;
        i2s_write(I2S_NUM_0, _feedBuf, sizeof(_feedBuf), &written, portMAX_DELAY);
    }
}

void IO::begin() {
    i2s_config_t cfg = {
        .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate          = AUDIO_FREQ,
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = DMA_BUF_COUNT,
        .dma_buf_len          = DMA_BUF_LEN,
        .use_apll             = false,
        .tx_desc_auto_clear   = true,
    };
    i2s_pin_config_t pins = {
        .bck_io_num   = I2S_BCLK,
        .ws_io_num    = I2S_LRCLK,
        .data_out_num = I2S_DIN,
        .data_in_num  = I2S_PIN_NO_CHANGE,
    };
    i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pins);
    i2s_zero_dma_buffer(I2S_NUM_0);

    _audioMutex = xSemaphoreCreateMutex();

    // Audio task na jezgri 0, main loop je na jezgri 0 (single-core C3)
    // Prioritet 2 — viši od loop() (prioritet 1) da DMA nikad ne ostane prazan
    xTaskCreate(audioTask, "audio", 2048, nullptr, 2, nullptr);
}

// audioUpdate() više nije potreban — task radi sam
// Ostavimo praznu implementaciju da main.cpp ne treba mijenjati
void IO::audioUpdate() { }

uint8_t IO::in(uint16_t port, i8080 *cpu) {
    uint16_t w;
    switch (port) {
    case 0: return 0x0f;
    case 1: return _p1;
    case 2: return _p2;
    case 3:
        w = (_s1 << 8) + _s0;
        return (w >> (8 - _soff)) & 0xff;
    }
    return 0x00;
}

void IO::out(uint16_t port, uint8_t b, i8080 *cpu) {
    uint8_t changed, cleared;
    switch (port) {
    case 2: _soff = b & 0x07; break;
    case 4: _s0 = _s1; _s1 = b; break;

    case 3:
        changed = b & ~_lastPort3;
        cleared = ~b & _lastPort3;
        // Zaštiti kanale muteexom — out() se poziva iz main loop-a
        xSemaphoreTake(_audioMutex, portMAX_DELAY);
        if (changed & 0x01) _chUfo.start(ufo,     sizeof(ufo));
        if (cleared & 0x01) _chUfo.stop();
        if (changed & 0x02) _chEffects.play(shot,    sizeof(shot));
        if (changed & 0x04) _chBasehit.play(basehit, sizeof(basehit));
        if (changed & 0x08) _chEffects.play(invhit,  sizeof(invhit));
        if (changed & 0x10) _chEffects.play(extend,  sizeof(extend));
        xSemaphoreGive(_audioMutex);
        _lastPort3 = b;
        break;

    case 5:
        changed = b & ~_lastPort5;
        xSemaphoreTake(_audioMutex, portMAX_DELAY);
        if (changed & 0x01) _chWalk.play(walk1,  sizeof(walk1));
        if (changed & 0x02) _chWalk.play(walk2,  sizeof(walk2));
        if (changed & 0x04) _chWalk.play(walk3,  sizeof(walk3));
        if (changed & 0x08) _chWalk.play(walk4,  sizeof(walk4));
        if (changed & 0x10) _chEffects.play(ufohit, sizeof(ufohit));
        xSemaphoreGive(_audioMutex);
        _lastPort5 = b;
        break;

    default: break;
    }
}

void IO::down(uint8_t key) {
    switch (key) {
    case P1_START: _p1 |=  0x04; break;
    case P1_LEFT:  _p1 |=  0x20; break;
    case P1_RIGHT: _p1 |=  0x40; break;
    case P1_SHOOT: _p1 |=  0x10; break;
    case P2_START: _p1 |=  0x02; break;
    case P2_LEFT:  _p2 |=  0x20; break;
    case P2_RIGHT: _p2 |=  0x40; break;
    case P2_SHOOT: _p2 |=  0x10; break;
    case COIN:     _p1 |=  0x01; break;
    }
}

void IO::up(uint8_t key) {
    switch (key) {
    case P1_START: _p1 &= ~0x04; break;
    case P1_LEFT:  _p1 &= ~0x20; break;
    case P1_RIGHT: _p1 &= ~0x40; break;
    case P1_SHOOT: _p1 &= ~0x10; break;
    case P2_START: _p1 &= ~0x02; break;
    case P2_LEFT:  _p2 &= ~0x20; break;
    case P2_RIGHT: _p2 &= ~0x40; break;
    case P2_SHOOT: _p2 &= ~0x10; break;
    case COIN:     _p1 &= ~0x01; break;
    }
}
