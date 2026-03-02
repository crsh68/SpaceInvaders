#ifndef __I8080_H__
#define __I8080_H__

#include <Arduino.h>
#include "memory.h"
#include "ports.h"

// i8080 CPU emulator
class i8080 {
public:
    i8080(memory_t &mem, Ports &ports);
    
    void run(unsigned instructions);
    void reset();
    bool halted() const { return _halted; }
    void raise(uint8_t level) { _irq_pending = level; }
    
    // Registers (public for interrupt handling)
    uint16_t PC;
    uint16_t SP;
    uint8_t A, B, C, D, E, H, L;
    uint8_t flags;
    
private:
    memory_t &_memory;
    Ports &_ports;
    bool _halted;
    uint8_t _irq_pending;
    bool _inte;  // Interrupt enable flag
    
    // Helper methods
    uint8_t fetch();
    uint16_t fetch16();
    void push(uint16_t value);
    uint16_t pop();
    void set_flags(uint8_t result, bool carry, bool aux_carry);
    
    // Instruction execution
    void execute();
};

#endif
