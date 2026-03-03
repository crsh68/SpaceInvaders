#ifndef __R65EMU_H__
#define __R65EMU_H__

#include <Arduino.h>
#include "memory.h"

// Forward declarations
class i8080;

// Hardware initialization and reset
void hardware_init(i8080 &cpu);
void hardware_reset();

// PROM (Program ROM) class
class prom {
public:
    prom(const uint8_t *data, size_t size) : _data(data), _size(size) {}
    
    const uint8_t *data() const { return _data; }
    size_t size() const { return _size; }
    
    uint8_t operator[](size_t addr) const {
        if (addr < _size) return _data[addr];
        return 0xFF;
    }
    
private:
    const uint8_t *_data;
    size_t _size;
};

// RAM class (defined here for convenience)
class ram: public Memory::Device {
public:
    ram(): Memory::Device(1024) {
        _mem = new uint8_t[1024];
        memset(_mem, 0, 1024);
    }
    
    ~ram() { delete[] _mem; }
    
    void operator=(uint8_t b) { _mem[_acc] = b; }
    operator uint8_t() { return _mem[_acc]; }
    
private:
    uint8_t *_mem;
};

#endif
