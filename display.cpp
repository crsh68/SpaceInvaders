#include <Arduino.h>
#include <SPI.h>
#include <memory.h>
#include <r65emu.h>

#include "config.h"
#include "display.h"

#define ST7789_SWRESET  0x01
#define ST7789_SLPOUT   0x11
#define ST7789_NORON    0x13
#define ST7789_INVON    0x21
#define ST7789_DISPON   0x29
#define ST7789_CASET    0x2A
#define ST7789_RASET    0x2B
#define ST7789_RAMWR    0x2C
#define ST7789_MADCTL   0x36
#define ST7789_COLMOD   0x3A

static SPIClass spi(FSPI);

static inline void sendCmd(uint8_t cmd) {
    digitalWrite(TFT_DC, LOW);
    spi.transfer(cmd);
}
static inline void sendData8(uint8_t d) {
    digitalWrite(TFT_DC, HIGH);
    spi.transfer(d);
}

static void setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    sendCmd(ST7789_CASET);
    digitalWrite(TFT_DC, HIGH);
    spi.transfer16(x0); spi.transfer16(x1);
    sendCmd(ST7789_RASET);
    digitalWrite(TFT_DC, HIGH);
    spi.transfer16(y0); spi.transfer16(y1);
    sendCmd(ST7789_RAMWR);
    digitalWrite(TFT_DC, HIGH);
}

// Swap bytes za ST7789 big-endian SPI
static inline uint16_t sw(uint16_t c) { return (c >> 8) | (c << 8); }

static void st7789_fillScreen(uint16_t color) {
    uint16_t c = sw(color);
    setWindow(0, 0, 239, 239);
    for (uint32_t i = 0; i < 240UL * 240UL; i++)
        spi.transfer16(c);
}

static void st7789_drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= 240 || y >= 240) return;
    setWindow(x, y, x, y);
    spi.transfer16(sw(color));
}

#define C_BLACK   0x0000
#define C_WHITE   0xFFFF
#define C_RED     0x07E0  // zamijenjena sa zelenom - hw specifičnost ovog modula
#define C_GREEN   0x001F  // zamijenjena sa plavom - hw specifičnost ovog modula

void Display::begin() {
    pinMode(10, OUTPUT); digitalWrite(10, HIGH); // BL

    pinMode(TFT_DC,  OUTPUT); digitalWrite(TFT_DC,  HIGH);
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH); delay(50);
    digitalWrite(TFT_RST, LOW);  delay(20);
    digitalWrite(TFT_RST, HIGH); delay(150);

    spi.begin(TFT_SCLK, -1, TFT_MOSI, -1);
    spi.setFrequency(40000000);
    spi.setDataMode(SPI_MODE3);

    sendCmd(ST7789_SWRESET); delay(150);
    sendCmd(ST7789_SLPOUT);  delay(500);
    sendCmd(ST7789_COLMOD);  sendData8(0x55); delay(10);
    sendCmd(ST7789_MADCTL);  sendData8(0x00);
    sendCmd(ST7789_INVON);   delay(10);
    sendCmd(ST7789_NORON);   delay(10);
    sendCmd(ST7789_DISPON);  delay(10);

    st7789_fillScreen(C_BLACK);

    _xoff = (240 - DISPLAY_X) / 2;
    _yoff = 0;
}

void Display::clear() {
    st7789_fillScreen(C_BLACK);
}

void Display::draw(Memory::address a, uint8_t b) {
    uint16_t y = ((DISPLAY_Y - ((a % BYTES_PER_LINE) * 8))-8)*240/256;    // * 240 / 256;
    uint16_t x = (a / BYTES_PER_LINE);

    uint8_t d = _buf[a] ^ b;
    for (unsigned i = 0, bit = 0x01; i < 8; i++, bit *= 2) {
        if (d & bit) {
            uint16_t fg = C_WHITE;
            uint16_t yi = y - i;
            if (yi > 24+240/256 && yi <= 56*240/256)    
                fg = C_RED;
            else if (yi > 176*240/256 && yi <= 232*240/256)
                fg = C_GREEN;
            else if (yi > 232*240/256 && x >= 16 && x < 134)
                fg = C_GREEN;
            st7789_drawPixel(x + _xoff, yi + _yoff, (b & bit) ? fg : C_BLACK);
        }
    }
    _buf[a] = b;
}
