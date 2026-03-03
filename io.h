#ifndef __IO_H__
#define __IO_H__

#include <ports.h>

class IO: public Ports {
public:
    IO(): _p1(0), _p2(0), _s0(0), _s1(0), _soff(0),
          _lastPort3(0), _lastPort5(0) {}
    void begin();
    void audioUpdate();

    uint8_t in(uint16_t p, i8080 *cpu);
    void out(uint16_t p, uint8_t b, i8080 *cpu);

    void down(uint8_t key);
    void up(uint8_t key);

private:
    uint8_t _soff, _s0, _s1, _p1, _p2;
    uint8_t _lastPort3, _lastPort5; // za edge detection zvukova
};

#endif
