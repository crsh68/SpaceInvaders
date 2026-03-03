# QUICK START - 5 Minuta do Igre!

## ⚡ Za PlatformIO (Preporučeno)

### 1. Instaliraj VS Code + PlatformIO (5 min)
```
1. Download VS Code: https://code.visualstudio.com/
2. Otvori VS Code
3. Extensions (Ctrl+Shift+X)
4. Traži "PlatformIO IDE"
5. Klikni Install
6. Čekaj instalaciju (5-10 minuta)
7. Restart VS Code
```

### 2. Otvori Projekat (30 sec)
```
1. File → Open Folder
2. Odaberi: SpaceInvaders_ESP32C3_Final
3. Čekaj da PlatformIO učita projekat
```

### 3. Kompajliraj (3 min)
```
1. Donji status bar → Klikni ✓ (Build)
   ILI
   Terminal → Run Task → PlatformIO: Build

2. PRVA KOMPILACIJA:
   - Downloaduje r65emu sa GitHub-a
   - Downloaduje TFT_eSPI
   - Kompajlira kod
   - Traje ~2-3 minute

3. Čekaj "SUCCESS"
```

### 4. Spoji Hardware (1 min)
```
1. Spoji ESP32-C3 USB-C kablom
2. Provjeři da li Windows vidi COM port
   - Device Manager → Ports
   - Trebao bi vidjeti "USB Serial Device (COMx)"
```

### 5. Upload (1 min)
```
1. Donji status bar → Klikni → (Upload)
   ILI
   Terminal → Run Task → PlatformIO: Upload

2. VAŽNO: Drži BOOT tipku (GPIO9) na ESP32-C3
   - Čim vidiš "Connecting..."
   - Pusti BOOT tipku
   
3. Čekaj "SUCCESS"
```

### 6. Testiraj! (odmah)
```
1. Donji status bar → Klikni 🔌 (Serial Monitor)
2. Odaberi baud rate: 115200
3. Trebao bi vidjeti:
   === Space Invaders ESP32-C3 ===
   Hardware initialized
   Game ready!

4. COIN (GPIO10) → Ubaci kredit
5. START (GPIO8) → Započni igru
6. LEFT/RIGHT/FIRE → Igraj!
```

---

## 📱 Za Arduino IDE

### 1. Konvertuj Projekat (2 min)
```bash
# Kopiraj sve iz src/ foldera
# U novi Arduino sketch folder nazvan "SpaceInvaders_ESP32C3"

1. Kreiraj folder: Documents/Arduino/SpaceInvaders_ESP32C3/
2. Kopiraj SVE iz src/ u taj folder
3. Preimenuj main.cpp → SpaceInvaders_ESP32C3.ino
```

### 2. Instaliraj Biblioteke (5 min)
```
r65emu:
1. https://github.com/jscrane/r65emu
2. Code → Download ZIP
3. Arduino IDE → Sketch → Include Library → Add .ZIP Library
4. Odaberi r65emu-master.zip

TFT_eSPI:
1. Tools → Manage Libraries
2. Traži "TFT_eSPI"
3. Install

ESP32 Board:
1. File → Preferences
2. Additional Boards Manager URLs:
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
3. Tools → Board → Boards Manager
4. Traži "esp32"
5. Install
```

### 3. Konfiguriraj TFT_eSPI (2 min)
```
Edituj: Documents/Arduino/libraries/TFT_eSPI/User_Setup.h

Dodaj:
#define ST7789_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 240
#define TFT_MOSI 4
#define TFT_SCLK 5
//#define TFT_CS   7  // displej nema CS pin
#define TFT_DC   6
#define TFT_RST  1
#define SPI_FREQUENCY 40000000
```

### 4. Kompajliraj i Upload (2 min)
```
1. Tools → Board → ESP32 Arduino → ESP32C3 Dev Module
2. Tools → Port → COMx
3. Sketch → Upload
4. Drži BOOT tipku dok ne vidiš "Connecting..."
```

---

## 🐛 Najčešći Problemi

### Problem: "Could not find board"
**Rješenje:** Instaliraj ESP32 board support u PlatformIO/Arduino IDE

### Problem: "Library not found"
**Rješenje:** 
- PlatformIO: Obriši `.pio` folder i kompajliraj ponovo
- Arduino IDE: Ručno instaliraj r65emu i TFT_eSPI

### Problem: Upload ne radi
**Rješenje:** 
1. Provjeri USB kabl (mora biti DATA kabl)
2. Drži BOOT tipku tokom uploada
3. Smanji upload speed u platformio.ini

### Problem: Bijeli ekran
**Rješenje:**
1. Provjeri TFT_eSPI konfiguraciju
2. Provjeri pinove (GPIO 1, 4, 5, 6, 7)
3. Provjeri napajanje (3.3V)

---

## ✅ Checklist

Prije nego što pokreneš:

- [ ] PlatformIO IDE instaliran
- [ ] Projekat otvoren u VS Code
- [ ] Kompajlacija uspješna
- [ ] ESP32-C3 spojen USB-om
- [ ] TFT display spojen (MOSI=4, SCLK=5, CS=7, DC=6, RST=1)
- [ ] Tipke spojene (LEFT=2, RIGHT=3, FIRE=10, COIN=20, START=21)
- [ ] Serial Monitor otvoren (115200 baud)

---

## 🎮 Kontrole

```
GPIO2  (LEFT)   → Lijevo
GPIO3  (RIGHT)  → Desno
GPIO9  (FIRE)   → Pucaj
GPIO10 (COIN)   → Kredit
GPIO8  (START)  → Start

```

---

## ⏱️ Ukupno Vrijeme

- **PlatformIO:** ~15 minuta (uključujući instalaciju)
- **Arduino IDE:** ~20 minuta (zbog ručne instalacije biblioteka)

---

**Sretno! Uživaj u Space Invaders!** 🎮👾
