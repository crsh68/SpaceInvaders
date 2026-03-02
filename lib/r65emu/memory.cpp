#include "memory.h"
#include "r65emu.h"

void memory_t::put(const prom &p, Memory::address at) {
    _rom[_rom_count].data = p.data();
    _rom[_rom_count].size = p.size();
    _rom[_rom_count].base = at;
    _rom_count++;
}

void memory_t::put(Memory::Device &d, Memory::address at) {
    _devices[_dev_count].device = &d;
    _devices[_dev_count].base = at;
    _devices[_dev_count].size = d.pages();
    _dev_count++;
}

uint8_t memory_t::read(Memory::address addr) {
    // Check ROM areas
    for (int i = 0; i < _rom_count; i++) {
        if (addr >= _rom[i].base && addr < _rom[i].base + _rom[i].size) {
            return _rom[i].data[addr - _rom[i].base];
        }
    }
    
    // Check devices
    for (int i = 0; i < _dev_count; i++) {
        if (addr >= _devices[i].base && addr < _devices[i].base + _devices[i].size) {
            _devices[i].device->access(addr - _devices[i].base);
            return (uint8_t)(*_devices[i].device);
        }
    }
    
    return 0xFF;
}

void memory_t::write(Memory::address addr, uint8_t value) {
    for (int i = 0; i < _dev_count; i++) {
        if (addr >= _devices[i].base && addr < _devices[i].base + _devices[i].size) {
            _devices[i].device->access(addr - _devices[i].base);
            *_devices[i].device = value;
            return;
        }
    }
}
