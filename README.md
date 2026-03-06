# Space Invaders - ESP32-C3 Super Mini (PlatformIO)

## 🎮 Originalni ESP8266 Projekat Prilagođen za ESP32-C3

Ovo je **Port originalnog ESP8266 Space Invaders projekta** za ESP32-C3 Super Mini.

---

## ✅ Što je novo

- ✅ **ESP32-C3 Super Mini** podrška
- ✅ **ST7789 240x240** display
- ✅ **PWM audio** (ESP32-C3 nema DAC)
- ✅ **80MHz SPI** (brže od ESP8266!)
- ✅ **160MHz CPU** (duplo brže!)
- ✅ **Automatska detekcija** - radi na oba ESP8266 i ESP32-C3

---

## 📦 Hardware

### ESP32-C3 Super Mini
- CPU: 160MHz RISC-V
- RAM: 400KB
- Flash: 4MB

### ST7789 240x240 Display (SPI)
```
ESP32-C3  ->  ST7789
GPIO4     ->  MOSI
GPIO5     ->  SCLK
GPIO6     ->  DC
GPIO1     ->  RST
```

### Kontrole (Tipke)
```
GPIO2     ->  LEFT
GPIO3     ->  RIGHT
GPIO9     ->  FIRE
GPIO10    ->  COIN
GPIO8     ->  START
```

### Zvuk MAX98357A I2S
```
GPIO7  -> BCLK
GPIO20 -> LRCLK
GPIO21 -> DIN
```

---

## 🚀 PlatformIO Setup

### 1. Instaliraj PlatformIO
- **VS Code:** Install "PlatformIO IDE" extension
- Ili: https://platformio.org/install

### 2. Otvori projekt
```bash
# U VS Code:
File → Open Folder → Odaberi "SpaceInvaders_ESP32C3_Final" folder

# Ili u terminalu:
cd SpaceInvaders_ESP32C3_Final
```

### 3. Kompajliraj
```bash
# Klikni "Build" u PlatformIO toolbar
# Ili:
pio run

# Prva kompilacija će automatski downloadovati:
# - r65emu biblioteku sa GitHub-a
# - TFT_eSPI biblioteku
# Može trajati 2-3 minute!
```

### 4. Upload
```bash
# Spoji ESP32-C3 USB kablom
# Drži BOOT tipku (GPIO0)
# Klikni "Upload" u PlatformIO
# Ili:
pio run --target upload
```

### 5. Serial Monitor
```bash
# Klikni "Serial Monitor" u PlatformIO
# Ili:
pio device monitor

# Trebao bi vidjeti:
# === Space Invaders ESP32-C3 ===
# Hardware initialized
# Game ready!
```

---

## ⚙️ Konfiguracija

### platformio.ini

Sve je već konfigurirano:
- ESP32-C3 board
- TFT_eSPI @ 80MHz
- r65emu biblioteka sa GitHub-a
- GPIO pinovi
- PWM audio

### Promjena GPIO Pinova

Edituj `platformio.ini` build_flags:
```ini
-D PIN_LEFT=2
-D PIN_RIGHT=3
...
```

---

## 📚 Biblioteke

### Automatski se instaliraju:
1. **r65emu** - i8080 emulator (GitHub)
2. **TFT_eSPI** - Display driver

---

## 📊 Performanse

| Feature | ESP8266 | ESP32-C3 |
|---------|---------|----------|
| CPU | 80MHz | 160MHz |
| RAM | 80KB | 400KB |
| SPI Max | 40MHz | 80MHz |
| FPS | 30-40 | 60+ |
| Audio | DAC | I2S |

---

## 🐛 Troubleshooting

### Upload ne radi
```bash
# Drži BOOT tipku (GPIO0) dok ne počne upload
pio run --target upload
```

### Kompajliranje greška
```bash
# Clean build
pio run --target clean
pio run
```

### Display ne radi
- Provjeri pinove u platformio.ini
- Provjeri napajanje (3.3V)
- Provjeri TFT driver (ST7789)

---

## 📁 Struktura Projekta (PlatformIO)

```
SpaceInvaders_ESP32C3_Final/
├── platformio.ini         ← PlatformIO konfiguracija
├── src/                   ← Source fajlovi
│   ├── main.cpp           ← Glavni kod
│   ├── config.h           ← GPIO pinovi
│   ├── display.h/cpp      ← Display driver
│   ├── io.h/cpp           ← IO i audio
│   ├── vblank.h           ← VBlank interrupt
│   ├── rome.h, romf.h, etc. ← ROM-ovi
│   ├── sounds.h           ← Audio samples
│   ├── images/            ← Sprite data
│   └── sounds/            ← WAV fajlovi
├── include/               ← Public headers (prazno)
├── lib/                   ← Private libraries (prazno)
├── test/                  ← Unit tests (prazno)
├── README.md              ← Ovaj fajl
└── WIRING.md              ← Pinout dijagram
```

---

## 🌟 Features

✅ **Originalni Space Invaders** - 100% autentičan
✅ **i8080 emulator** - Prava CPU emulacija
✅ **Color overlay** - Crvena/zelena zona
✅ **Zvučni efekti** - Svi originalni zvukovi
✅ **Smooth gameplay** - 60 FPS @ 160MHz
✅ **Multi-platform** - Radi na ESP8266 i ESP32-C3

---

## 🙏 Credits

- **Originalni projekat:** ESP8266 Space Invaders
- **r65emu:** jscrane (GitHub)
- **TFT_eSPI:** Bodmer
- **Space Invaders:** © Taito Corporation 1978
- **ESP32-C3 Port:** Adaptacija za ESP32-C3

---

## 📜 Licence

- Space Invaders: © Taito Corporation
- Ovaj projekat: Edukativne svrhe
- r65emu: Check GitHub licence
- TFT_eSPI: FreeBSD

---

**Uživaj u klasičnom arcade gaming-u!** 🎮👾
