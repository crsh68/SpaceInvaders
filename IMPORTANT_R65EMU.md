# ⚠️ VAŽNA NAPOMENA - r65emu Biblioteka

## Problem sa GitHub r65emu

GitHub verzija r65emu biblioteke je **promijenjena** i više **nije kompatibilna** sa Space Invaders projektom:
- ❌ Nema `ports.h`
- ❌ Nema `i8080.h`
- ❌ Nema `memory.h`
- ❌ Potpuno različita struktura

## ✅ Rješenje - Lokalna Implementacija

Ovaj projekat **NE koristi** GitHub r65emu!

Umjesto toga, uključena je **lokalna implementacija** i8080 emulatora u `lib/r65emu/` folderu:

```
lib/r65emu/
├── library.json       ← Lokalna biblioteka
├── r65emu.h          ← Osnovne definicije
├── ports.h           ← IO ports interface
├── i8080.h          ← CPU emulator header
├── i8080.cpp         ← CPU emulator (~656 linija)
├── memory.h          ← Memory management
└── hardware.cpp      ← Hardware funkcije
```

## 🚀 Što To Znači Za Tebe?

### ✅ PREDNOSTI:
- **NE trebaju vanjske biblioteke** (osim TFT_eSPI)
- **Sve već uključeno** - samo otvori i kompajliraj
- **Garantovano radi** - testirana lokalna verzija
- **Brža kompilacija** - ne čeka GitHub download

### 📦 Što Treba Instalirati:

**Samo TFT_eSPI!**

PlatformIO će automatski instalirati samo:
```
bodmer/TFT_eSPI @ ^2.5.43
```

r65emu je **već lokalno** u `lib/` folderu!

---

## 🔧 Ako Vidiš Greške

### Greška: "No module named 'intelhex'"
**Popravljena!** 
- `platformio.ini` sada koristi `platform_packages = tool-esptoolpy@~1.40400.0`

### Greška: "ports.h: No such file"
**Popravljena!**
- `ports.h` je sada u `lib/r65emu/`
- Svi include-ovi ažurirani

### Greška: "hw/user.h not configured!"
**Ignoriši - Warning, ne Error!**
- Ovo je samo warning iz starog r65emu koda
- Ne utiče na funkcionalnost

---

## 📝 Za Developere

Ako želiš vidjeti i8080 emulator kod:
```
lib/r65emu/i8080.cpp  → ~656 linija CPU emulator koda
lib/r65emu/memory.h   → Memory management
lib/r65emu/ports.h    → IO ports interface
```

Kompletan i funkcionalan i8080 emulator!

---

## ✅ Checklist Prije Kompajliranja

- [ ] PlatformIO IDE instaliran u VS Code
- [ ] Projekat otvoren (`SpaceInvaders_ESP32C3_Final`)
- [ ] **NE** instaliraj ručno r65emu - već je uključen!
- [ ] Samo čekaj da PlatformIO instalira TFT_eSPI
- [ ] Klikni Build

**Trebalo bi raditi bez problema!**

---

**Status:** ✅ Sve potrebno je uključeno u projektu!
