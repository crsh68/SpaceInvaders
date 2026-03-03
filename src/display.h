#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <memory.h>

#define BYTES_PER_LINE  32
#define DISPLAY_X       224
#define DISPLAY_Y       (BYTES_PER_LINE * 8)
#define DISPLAY_RAM     (DISPLAY_X * BYTES_PER_LINE)

// Pinovi (bez CS - display nema CS pin)
#define TFT_MOSI  4
#define TFT_SCLK  5
#define TFT_DC    6
#define TFT_RST   1

class Display: public Memory::Device {
public:
    Display(): Memory::Device(DISPLAY_RAM) {
        memset(_buf, 0, DISPLAY_RAM);
    }

    void begin();
    void clear();

    void operator=(uint8_t b) {
        if (_buf[_acc] != b) draw(_acc, b);
    }
    operator uint8_t() { return _buf[_acc]; }

private:
    void draw(Memory::address a, uint8_t b);
    uint8_t  _buf[DISPLAY_RAM];
    int16_t  _xoff, _yoff;
};

#endif
