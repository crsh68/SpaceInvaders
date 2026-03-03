# Space Invaders — ESP32-C3 Super Mini (PlatformIO)

Emulator klasične arkadne igre Space Invaders za ESP32-C3 Super Mini.

---

## Hardware

### ESP32-C3 Super Mini
- CPU: 160MHz RISC-V
- RAM: 400KB
- Flash: 4MB

### ST7789 240×240 Display (SPI)
```
ESP32-C3  →  ST7789
GPIO4     →  MOSI
GPIO5     →  SCLK
GPIO7     →  CS
GPIO6     →  DC
GPIO1     →  RST
GPIO9     →  LED (backlight)
```

### Kontrole
```
GPIO2   →  LEFT
GPIO3   →  RIGHT
GPIO10  →  FIRE
GPIO20  →  COIN
GPIO21  →  START
GPIO0   →  PAUSE (BOOT button)
```

### Zvuk — MAX98357A (I2S)
```
ESP32-C3  →  MAX98357A
GPIO7     →  BCLK
GPIO20    →  LRC
GPIO21    →  DIN
```

---

## PlatformIO Setup

### 1. Instaliraj PlatformIO
- VS Code: instaliraj "PlatformIO IDE" ekstenziju
- Ili: https://platformio.org/install

### 2. Otvori projekt
```
File → Open Folder → odaberi SpaceInvaders folder
```

### 3. Kompajliraj i uploadi
```bash
pio run --target upload
```

### 4. Serial Monitor
```bash
pio device monitor
```

---

## Konfiguracija

GPIO pinovi se mijenjaju u `platformio.ini` pod `build_flags`:
```ini
-D PIN_LEFT=2
-D PIN_RIGHT=3
-D PIN_FIRE=10
-D PIN_COIN=20
-D PIN_START_P1=21
```

---

## Audio arhitektura

Zvuk koristi **FreeRTOS audio task** koji radi neovisno od glavne petlje,
što eliminira audio jitter uzrokovan SPI display transferima i CPU emulacijom.

**4 neovisna miksana kanala:**
- `_chUfo` — loop kanal, svira kontinuirano dok UFO leti
- `_chBasehit` — eksplozija igrača, ne može je prekinuti drugi zvuk
- `_chEffects` — shot, invhit, extend, ufohit
- `_chWalk` — fleet movement beepovi (walk1–4)

**Edge detection** na I/O portovima 3 i 5 sprječava višestruko
triggiranje zvukova jer originalni ROM drži bitove HIGH više frameova.

---

## Struktura projekta

```
SpaceInvaders/
├── platformio.ini
├── src/
│   ├── main.cpp        — glavna petlja
│   ├── io.cpp / io.h   — I/O, audio (FreeRTOS task + I2S)
│   ├── display.h/cpp   — ST7789 driver
│   ├── config.h        — GPIO pinovi
│   ├── vblank.h        — VBlank interrupt
│   ├── rome.h … romh.h — ROM podaci
│   └── sounds.h        — PCM audio sampli
├── include/
├── lib/
└── README.md
```

---

## Biblioteke

Automatski se instaliraju pri prvom buildu:
- **r65emu** — i8080 CPU emulator (GitHub: jscrane)
- **TFT_eSPI** — display driver (Bodmer)

---

## Kontrole u igri

| Tipka  | Akcija              |
|--------|---------------------|
| LEFT   | Pomjeri brod lijevo |
| RIGHT  | Pomjeri brod desno  |
| FIRE   | Pucaj               |
| COIN   | Ubaci kredit        |
| START  | Započni igru        |
| PAUSE  | Pauziraj / nastavi  |

---

## Performanse

| | ESP8266 | ESP32-C3 |
|---|---|---|
| CPU | 80MHz | 160MHz |
| RAM | 80KB | 400KB |
| SPI | 40MHz | 80MHz |
| Audio | PWM | I2S (MAX98357A) |

---

## Credits

- Originalni ESP8266 projekt: temelj emulacije
- **r65emu**: jscrane (GitHub)
- **TFT_eSPI**: Bodmer
- Space Invaders: © Taito Corporation 1978

---

## Licenca

- Space Invaders ROM: © Taito Corporation — samo za edukativne svrhe
- Kod projekta: MIT
- r65emu / TFT_eSPI: pogledaj njihove GitHub licence
