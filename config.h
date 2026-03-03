#ifndef __CONFIG_H__
#define __CONFIG_H__

// =========================================
// ESP32-C3 Super Mini - Pinout
// =========================================
// DISPLAY:
//   GPIO1  = TFT_RST
//   GPIO4  = TFT_MOSI
//   GPIO5  = TFT_SCLK
//   GPIO6  = TFT_DC
//   BL     = VCC direktno (uvijek upaljen)
//
// MAX98357A I2S:
//   GPIO7  = BCLK
//   GPIO20 = LRCLK
//   GPIO21 = DIN
//
// KONTROLE:
//   GPIO0  = START (BOOT tipka)
//   GPIO2  = LEFT
//   GPIO3  = RIGHT
//   GPIO10 = COIN
//   GPIO9  = FIRE
// =========================================

#define PIN_LEFT     2
#define PIN_RIGHT    3
#define PIN_FIRE     9
#define PIN_COIN     10
#define PIN_START_P1 0

#define P1_LEFT   2
#define P1_RIGHT  3
#define P1_SHOOT  9
#define COIN      10
#define P1_START  0
#define P2_LEFT   5
#define P2_RIGHT  6
#define P2_SHOOT  7
#define P2_START  8

#endif
