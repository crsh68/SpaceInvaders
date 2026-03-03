#ifndef __PORTS_H__
#define __PORTS_H__

#include <Arduino.h>

// Forward declaration
class i8080;

// Ports interface for i8080 emulator
class Ports {
public:
    virtual ~Ports() {}
    
    // Port input (IN instruction)
    virtual uint8_t in(uint16_t port, i8080 *cpu) = 0;
    
    // Port output (OUT instruction)
    virtual void out(uint16_t port, uint8_t b, i8080 *cpu) = 0;
};

#endif
