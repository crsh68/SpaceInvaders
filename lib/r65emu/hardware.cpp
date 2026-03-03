#include <Arduino.h>
#include "r65emu.h"
#include "memory.h"
#include "i8080.h"

memory_t memory;

void hardware_init(i8080 &cpu) {
    cpu.reset();
}

void hardware_reset() {
    // Display reset se radi u Display::begin()
}
