#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <Arduino.h>

// Forward declaration
class prom;

namespace Memory {
    typedef uint16_t address;
    
    // Memory device base class
    class Device {
    public:
        Device(unsigned bytes): _pages(bytes) {}
        virtual ~Device() {}
        
        virtual void operator=(uint8_t b) = 0;
        virtual operator uint8_t() = 0;
        
        void access(address a) { _acc = a; }
        address pages() const { return _pages; }
        
    protected:
        address _acc;
        address _pages;
    };
}

// Memory management class
class memory_t {
public:
    memory_t() : _devices(), _rom_count(0), _dev_count(0) {}
    
    void put(const prom &p, Memory::address at);
    void put(Memory::Device &d, Memory::address at);
    
    uint8_t read(Memory::address addr);
    void write(Memory::address addr, uint8_t value);

private:
    struct rom_entry {
        const uint8_t *data;
        size_t size;
        Memory::address base;
    };
    
    struct dev_entry {
        Memory::Device *device;
        Memory::address base;
        Memory::address size;
    };
    
    rom_entry _rom[8];
    dev_entry _devices[8];
    int _rom_count;
    int _dev_count;
};

extern memory_t memory;

#endif
