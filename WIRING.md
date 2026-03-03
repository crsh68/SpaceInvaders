# WIRING DIAGRAM - ESP32-C3 Super Mini

```
┌────────────────────────────────────────────────────────────┐
│             ESP32-C3 SUPER MINI                            │
│                                                            │
│   [USB-C]                                                  │
│     ││                                                     │
│   ┌─┴┴────────────────────────────────────────────────┐    │
│   │                                                   │    │
│   │  GPIO0  ●─── PAUSE (BOOT button) ────── [GND]     │    │
│   │  GPIO1  ────────────────────────────── TFT_RST    │    │
│   │  GPIO2  ●─── LEFT button ─────────────── [GND]    │    │
│   │  GPIO3  ●─── RIGHT button ────────────── [GND]    │    │
│   │  GPIO4  ────────────────────────────── TFT_MOSI   │    │
│   │  GPIO5  ────────────────────────────── TFT_SCLK   │    │
│   │  GPIO6  ────────────────────────────── TFT_DC     │    │
│   │  GPIO7  ────────────────────────────── TFT_CS     │    │
│   │  GPIO8  ──┬─ 100Ω ──┬─ Speaker ──┬──── [GND]      │    │
│   │           │          └─ 100µF ────┘               │    │
│   │  GPIO9  ────────────────────────────── TFT_LED    │    │
│   │  GPIO10 ●─── FIRE button ─────────────── [GND]    │    │
│   │  GPIO20 ●─── COIN button ─────────────── [GND]    │    │
│   │  GPIO21 ●─── START button ────────────── [GND]    │    │
│   │                                                   │    │
│   │  [3V3]  ────────────────────────────── TFT_VCC    │    │
│   │  [GND]  ────────────────────────────── TFT_GND    │    │
│   │                                                   │    │
│   └───────────────────────────────────────────────────┘    │
└────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────┐
│     ST7789 TFT Display (240x240)         │
│                                          │
│  VCC  ─── 3.3V                           │
│  GND  ─── GND                            │
│  CS   ─── GPIO7                          │
│  RST  ─── GPIO1                          │
│  DC   ─── GPIO6                          │
│  MOSI ─── GPIO4 (SDA)                    │
│  SCK  ─── GPIO5 (CLK)                    │
│  LED  ─── GPIO9 (opciono - ili 3.3V)     │
│  MISO ─── (ne koristi se)                │
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
│  GPIO7  ───── BCLK                       │
│  GPIO20 ───── LRCLK                      │
│  GPIO21 ───── DIN                        │
└──────────────────────────────────────────┘
```

## Napomene:

1. **INPUT_PULLUP** - Svi GPIO pinovi sa ● su INPUT_PULLUP
2. **SPI Pins** - GPIO 4, 5, 6, 7 su optimalni za hardware SPI
3. **Speaker** - Kondenzator 100µF blokira DC offset
4. **LED Backlight** - Može biti direktno na 3.3V ako nemaš PWM
5. **BOOT Button** - GPIO8 je već na ploči kao BOOT tipka

## Preporučeni Kablovi:

- TFT Display: Kratki kablovi (<10cm)
- Tipke: Može biti duži (20-30cm)
- Speaker: Kratki kabal (<15cm)

## Napajanje:

- USB-C napajanje: 5V/1A minimum
- ESP32-C3: ~60mA idle, ~150mA peak
- TFT Display: ~50-100mA
- **UKUPNO:** 500mA-1A preporučeno
