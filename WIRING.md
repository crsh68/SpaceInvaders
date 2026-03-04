# WIRING DIAGRAM - ESP32-C3 Super Mini

```
┌────────────────────────────────────────────────────────────┐
│             ESP32-C3 SUPER MINI                            │
│                                                            │
│   [USB-C]                                                  │
│     ││                                                     │
│   ┌─┴┴────────────────────────────────────────────────┐    │
│   │                                                   │    │
│   │  GPIO0  ●─── START button ───────────── [GND]     │    │
│   │  GPIO1  ────────────────────────────── TFT_RST    │    │
│   │  GPIO2  ●─── LEFT button ────────────── [GND]     │    │
│   │  GPIO3  ●─── RIGHT button ───────────── [GND]     │    │
│   │  GPIO4  ────────────────────────────── TFT_MOSI   │    │
│   │  GPIO5  ────────────────────────────── TFT_SCLK   │    │
│   │  GPIO6  ────────────────────────────── TFT_DC     │    │
│   │  GPIO7  ────────────────────────────── MAX_BCLK   │    │
│   │  GPIO8  ●─── START button ───────────── [GND]     │    │
│   │  GPIO9  ●─── FIRE button ────────────── [GND]     |    |
│   │  GPIO10 ●─── COIN button ────────────── [GND]     │    │
│   │  GPIO20 ────────────────────────────── MAX_LRCLK  │    │
│   │  GPIO21 ────────────────────────────── MAX_DIN    │    │
│   │                                                   │    │
│   │  [3V3]  ────────────────────────────── TFT_VCC    │    │
│   │  [GND]  ────────────────────────────── TFT_GND    │    │
│   │  [5V]   ────────────────────────────── MAX_VCC    │    │
│   │                                                   │    │
│   └───────────────────────────────────────────────────┘    │
└────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────┐
│     ST7789 TFT Display (240x240)         │
│                                          │
│  VCC  ──────────────────────────── 3.3V  │
│  GND  ───────────────────────────── GND  │
│  CS   ───────────────── (ne koristi se)  │
│  RST  ─────────────────────────── GPIO1  │
│  DC   ─────────────────────────── GPIO6  │
│  MOSI ───────────────────── GPIO4 (SDA)  │
│  SCK  ───────────────────── GPIO5 (CLK)  │
│  BKL  ──────────────────────────── 3.3V  │
│  MISO ───────────────── (ne koristi se)  │
│                                          │
└──────────────────────────────────────────┘

┌─────────────────────────────────────────┐
│              TIPKE                      │
│                                         │
│  LEFT  ──┬─ momentary switch ──┬─ GND   │
│  RIGHT ──┬─ momentary switch ──┬─ GND   │
│  FIRE  ──┬─ momentary switch ──┬─ GND   │
│  COIN  ──┬─ momentary switch ──┬─ GND   │
│  START ──┬─ momentary switch ──┬─ GND   │
│                                         │
└─────────────────────────────────────────┘

┌──────────────────────────────────────────┐
│           ZVUK  MAX98357A I2S            │
│                                          │
│  GPIO7  ─────────────────────── BCLK     │
│  GPIO20 ────────────────────── LRCLK     │
│  GPIO21 ─────────────────────── DIN      │
└──────────────────────────────────────────┘
```

## Napomene:

1. **INPUT_PULLUP** - Svi GPIO pinovi sa ● su INPUT_PULLUP
2. **SPI Pins** - GPIO 4, 5, 6 su optimalni za hardware SPI
3. **LED Backlight** - Može biti direktno na 3.3V ako nemaš PWM
4. **BOOT Button** - GPIO9 je već na ploči kao BOOT tipka

## Preporučeni Kablovi:

- TFT Display: Kratki kablovi (<10cm)
- Tipke: Može biti duži (20-30cm)
- Speaker: Kratki kabal (<15cm)

## Napajanje:

- USB-C napajanje: 5V/1A minimum
- ESP32-C3: ~60mA idle, ~150mA peak
- TFT Display: ~50-100mA
- **UKUPNO:** 500mA-1A preporučeno
