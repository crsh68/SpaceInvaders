#include <Arduino.h>
#include <driver/i2s.h>
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

static const uint8_t  *_playing     = nullptr;
static size_t          _playingSize = 0;
static volatile size_t _playingPos  = 0;

static void IRAM_ATTR onTimer() {
    if (_playing && _playingPos < _playingSize) {
        // Konvertiramo 8-bit ulaw sample u 16-bit signed za I2S
        int16_t sample = ((int16_t)_playing[_playingPos++] - 128) * 256;
        // Šaljemo stereo (L+R isti kanal)
        uint32_t stereo = ((uint32_t)(uint16_t)sample << 16) | (uint16_t)sample;
        size_t written;
        i2s_write(I2S_NUM_0, &stereo, 4, &written, 0);
    } else {
        _playing    = nullptr;
        _playingPos = 0;
        // Tišina
        uint32_t silence = 0;
        size_t written;
        i2s_write(I2S_NUM_0, &silence, 4, &written, 0);
    }
}

static void playSound(const uint8_t *data, size_t size) {
    _playing     = data;
    _playingSize = size;
    _playingPos  = 0;
}

void IO::begin() {
    // I2S konfiguracija za MAX98357A
    i2s_config_t i2s_config = {
        .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate          = AUDIO_FREQ,
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = 8,
        .dma_buf_len          = 64,
        .use_apll             = false,
        .tx_desc_auto_clear   = true,
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num   = I2S_BCLK,
        .ws_io_num    = I2S_LRCLK,
        .data_out_num = I2S_DIN,
        .data_in_num  = I2S_PIN_NO_CHANGE,
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM_0);

    // Timer za sample playback na 11127 Hz
    hw_timer_t *timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000 / AUDIO_FREQ, true);
    timerAlarmEnable(timer);
}

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
    switch (port) {
    case 2: _soff = b & 0x07; break;
    case 4: _s0 = _s1; _s1 = b; break;
    case 3:
        if (b & 0x01) playSound(ufo,     sizeof(ufo));
        if (b & 0x02) playSound(shot,    sizeof(shot));
        if (b & 0x04) playSound(basehit, sizeof(basehit));
        if (b & 0x08) playSound(invhit,  sizeof(invhit));
        if (b & 0x10) playSound(extend,  sizeof(extend));
        break;
    case 5:
        if (b & 0x01) playSound(walk1,  sizeof(walk1));
        if (b & 0x02) playSound(walk2,  sizeof(walk2));
        if (b & 0x04) playSound(walk3,  sizeof(walk3));
        if (b & 0x08) playSound(walk4,  sizeof(walk4));
        if (b & 0x10) playSound(ufohit, sizeof(ufohit));
        break;
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
