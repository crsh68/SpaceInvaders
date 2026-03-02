#include <Arduino.h>
#include <SPI.h>
#include <r65emu.h>
#include <ports.h>
#include <i8080.h>
#include <memory.h>

#include "config.h"
#include "io.h"
#include "display.h"
#include "vblank.h"

#include "rome.h"
#include "romf.h"
#include "romg.h"
#include "romh.h"

prom e(rome, sizeof(rome));
prom f(romf, sizeof(romf));
prom g(romg, sizeof(romg));
prom h(romh, sizeof(romh));

IO io;
i8080 cpu(memory, io);
ram page;
Display display;
vblank vb(cpu);

static bool lastLeft  = false;
static bool lastRight = false;
static bool lastFire  = false;
static bool lastCoin  = false;
static bool lastStart = false;

void setup() {
    memory.put(h, 0x0000);
    memory.put(g, 0x0800);
    memory.put(f, 0x1000);
    memory.put(e, 0x1800);
    memory.put(page,    0x2000);
    memory.put(display, 0x2400);

    pinMode(PIN_LEFT,     INPUT_PULLUP);
    pinMode(PIN_RIGHT,    INPUT_PULLUP);
    pinMode(PIN_FIRE,     INPUT_PULLUP);
    pinMode(PIN_COIN,     INPUT_PULLUP);
    pinMode(PIN_START_P1, INPUT_PULLUP);

    cpu.reset();
    display.begin();
    io.begin();
}

inline void handleButton(bool current, bool &last, uint8_t key) {
    if (current && !last) io.down(key);
    if (!current && last) io.up(key);
    last = current;
}

void loop() {
    bool left  = digitalRead(PIN_LEFT)     == LOW;
    bool right = digitalRead(PIN_RIGHT)    == LOW;
    bool fire  = digitalRead(PIN_FIRE)     == LOW;
    bool coin  = digitalRead(PIN_COIN)     == LOW;
    bool start = digitalRead(PIN_START_P1) == LOW;

    handleButton(left,  lastLeft,  P1_LEFT);
    handleButton(right, lastRight, P1_RIGHT);
    handleButton(fire,  lastFire,  P1_SHOOT);
    handleButton(coin,  lastCoin,  COIN);
    handleButton(start, lastStart, P1_START);

    if (!cpu.halted()) {
        cpu.run(1000);
        vb.tick(millis());
    }
}
