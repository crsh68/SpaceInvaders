#include "i8080.h"
#include "memory.h"
#include "ports.h"

// Flag bits
#define FLAG_S   0x80  // Sign
#define FLAG_Z   0x40  // Zero
#define FLAG_AC  0x10  // Auxiliary Carry
#define FLAG_P   0x04  // Parity
#define FLAG_C   0x01  // Carry

// Parity lookup table
static const uint8_t parity_table[256] = {
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1
};

i8080::i8080(memory_t &mem, Ports &ports) 
    : _memory(mem), _ports(ports), _halted(false), _irq_pending(0), _inte(false) {
    reset();
}

void i8080::reset() {
    PC = 0;
    SP = 0;
    A = B = C = D = E = H = L = 0;
    flags = 0x02;  // Bit 1 always set
    _halted = false;
    _inte = false;
    _irq_pending = 0;
}

uint8_t i8080::fetch() {
    return _memory.read(PC++);
}

uint16_t i8080::fetch16() {
    uint8_t lo = fetch();
    uint8_t hi = fetch();
    return (hi << 8) | lo;
}

void i8080::push(uint16_t value) {
    SP -= 2;
    _memory.write(SP, value & 0xFF);
    _memory.write(SP + 1, value >> 8);
}

uint16_t i8080::pop() {
    uint8_t lo = _memory.read(SP);
    uint8_t hi = _memory.read(SP + 1);
    SP += 2;
    return (hi << 8) | lo;
}

void i8080::set_flags(uint8_t result, bool carry, bool aux_carry) {
    flags = 0x02;  // Bit 1 always set
    
    if (result == 0) flags |= FLAG_Z;
    if (result & 0x80) flags |= FLAG_S;
    if (parity_table[result]) flags |= FLAG_P;
    if (carry) flags |= FLAG_C;
    if (aux_carry) flags |= FLAG_AC;
}

// Get register pair
inline uint16_t get_bc(i8080 *cpu) { return (cpu->B << 8) | cpu->C; }
inline uint16_t get_de(i8080 *cpu) { return (cpu->D << 8) | cpu->E; }
inline uint16_t get_hl(i8080 *cpu) { return (cpu->H << 8) | cpu->L; }

// Set register pair
inline void set_bc(i8080 *cpu, uint16_t val) { cpu->B = val >> 8; cpu->C = val & 0xFF; }
inline void set_de(i8080 *cpu, uint16_t val) { cpu->D = val >> 8; cpu->E = val & 0xFF; }
inline void set_hl(i8080 *cpu, uint16_t val) { cpu->H = val >> 8; cpu->L = val & 0xFF; }

void i8080::run(unsigned instructions) {
    for (unsigned i = 0; i < instructions && !_halted; i++) {
        // Check for interrupts
        if (_inte && _irq_pending) {
            _inte = false;  // Disable interrupts
            push(PC);
            PC = _irq_pending * 8;  // RST instruction
            _irq_pending = 0;
        }
        
        execute();
    }
}

void i8080::execute() {
    uint8_t opcode = fetch();
    uint8_t temp8;
    uint16_t temp16, addr;
    int16_t result16;
    
    switch (opcode) {
        // NOP
        case 0x00: break;
        
        // LXI B,d16
        case 0x01: set_bc(this, fetch16()); break;
        
        // STAX B
        case 0x02: _memory.write(get_bc(this), A); break;
        
        // INX B
        case 0x03: set_bc(this, get_bc(this) + 1); break;
        
        // INR B
        case 0x04:
            temp8 = B + 1;
            set_flags(temp8, flags & FLAG_C, ((B & 0x0F) + 1) > 0x0F);
            B = temp8;
            break;
        
        // DCR B
        case 0x05:
            temp8 = B - 1;
            set_flags(temp8, flags & FLAG_C, !((B & 0x0F) < 1));
            B = temp8;
            break;
        
        // MVI B,d8
        case 0x06: B = fetch(); break;
        
        // RLC
        case 0x07:
            flags = (flags & ~FLAG_C) | (A >> 7);
            A = (A << 1) | (A >> 7);
            break;
        
        // DAD B
        case 0x09:
            temp16 = get_hl(this) + get_bc(this);
            flags = (flags & ~FLAG_C) | ((temp16 > 0xFFFF) ? FLAG_C : 0);
            set_hl(this, temp16);
            break;
        
        // LDAX B
        case 0x0A: A = _memory.read(get_bc(this)); break;
        
        // DCX B
        case 0x0B: set_bc(this, get_bc(this) - 1); break;
        
        // INR C
        case 0x0C:
            temp8 = C + 1;
            set_flags(temp8, flags & FLAG_C, ((C & 0x0F) + 1) > 0x0F);
            C = temp8;
            break;
        
        // DCR C
        case 0x0D:
            temp8 = C - 1;
            set_flags(temp8, flags & FLAG_C, !((C & 0x0F) < 1));
            C = temp8;
            break;
        
        // MVI C,d8
        case 0x0E: C = fetch(); break;
        
        // RRC
        case 0x0F:
            flags = (flags & ~FLAG_C) | (A & 0x01);
            A = (A >> 1) | (A << 7);
            break;
        
        // LXI D,d16
        case 0x11: set_de(this, fetch16()); break;
        
        // STAX D
        case 0x12: _memory.write(get_de(this), A); break;
        
        // INX D
        case 0x13: set_de(this, get_de(this) + 1); break;
        
        // INR D
        case 0x14:
            temp8 = D + 1;
            set_flags(temp8, flags & FLAG_C, ((D & 0x0F) + 1) > 0x0F);
            D = temp8;
            break;
        
        // DCR D
        case 0x15:
            temp8 = D - 1;
            set_flags(temp8, flags & FLAG_C, !((D & 0x0F) < 1));
            D = temp8;
            break;
        
        // MVI D,d8
        case 0x16: D = fetch(); break;
        
        // RAL
        case 0x17:
            temp8 = (A << 1) | (flags & FLAG_C);
            flags = (flags & ~FLAG_C) | (A >> 7);
            A = temp8;
            break;
        
        // DAD D
        case 0x19:
            temp16 = get_hl(this) + get_de(this);
            flags = (flags & ~FLAG_C) | ((temp16 > 0xFFFF) ? FLAG_C : 0);
            set_hl(this, temp16);
            break;
        
        // LDAX D
        case 0x1A: A = _memory.read(get_de(this)); break;
        
        // DCX D
        case 0x1B: set_de(this, get_de(this) - 1); break;
        
        // INR E
        case 0x1C:
            temp8 = E + 1;
            set_flags(temp8, flags & FLAG_C, ((E & 0x0F) + 1) > 0x0F);
            E = temp8;
            break;
        
        // DCR E
        case 0x1D:
            temp8 = E - 1;
            set_flags(temp8, flags & FLAG_C, !((E & 0x0F) < 1));
            E = temp8;
            break;
        
        // MVI E,d8
        case 0x1E: E = fetch(); break;
        
        // RAR
        case 0x1F:
            temp8 = (A >> 1) | ((flags & FLAG_C) << 7);
            flags = (flags & ~FLAG_C) | (A & 0x01);
            A = temp8;
            break;
        
        // LXI H,d16
        case 0x21: set_hl(this, fetch16()); break;
        
        // SHLD addr
        case 0x22:
            addr = fetch16();
            _memory.write(addr, L);
            _memory.write(addr + 1, H);
            break;
        
        // INX H
        case 0x23: set_hl(this, get_hl(this) + 1); break;
        
        // INR H
        case 0x24:
            temp8 = H + 1;
            set_flags(temp8, flags & FLAG_C, ((H & 0x0F) + 1) > 0x0F);
            H = temp8;
            break;
        
        // DCR H
        case 0x25:
            temp8 = H - 1;
            set_flags(temp8, flags & FLAG_C, !((H & 0x0F) < 1));
            H = temp8;
            break;
        
        // MVI H,d8
        case 0x26: H = fetch(); break;
        
        // DAA
        case 0x27: {
            uint8_t correction = 0;
            bool carry = flags & FLAG_C;
            
            if ((A & 0x0F) > 9 || (flags & FLAG_AC)) correction = 0x06;
            if ((A >> 4) > 9 || carry || ((A >> 4) >= 9 && (A & 0x0F) > 9)) {
                correction |= 0x60;
                carry = true;
            }
            
            temp8 = A + correction;
            set_flags(temp8, carry, ((A & 0x0F) + (correction & 0x0F)) > 0x0F);
            A = temp8;
            break;
        }
        
        // DAD H
        case 0x29:
            temp16 = get_hl(this) + get_hl(this);
            flags = (flags & ~FLAG_C) | ((temp16 > 0xFFFF) ? FLAG_C : 0);
            set_hl(this, temp16);
            break;
        
        // LHLD addr
        case 0x2A:
            addr = fetch16();
            L = _memory.read(addr);
            H = _memory.read(addr + 1);
            break;
        
        // DCX H
        case 0x2B: set_hl(this, get_hl(this) - 1); break;
        
        // INR L
        case 0x2C:
            temp8 = L + 1;
            set_flags(temp8, flags & FLAG_C, ((L & 0x0F) + 1) > 0x0F);
            L = temp8;
            break;
        
        // DCR L
        case 0x2D:
            temp8 = L - 1;
            set_flags(temp8, flags & FLAG_C, !((L & 0x0F) < 1));
            L = temp8;
            break;
        
        // MVI L,d8
        case 0x2E: L = fetch(); break;
        
        // CMA
        case 0x2F: A = ~A; break;

        // LXI SP,d16
        case 0x31: SP = fetch16(); break;
        
        // STA addr
        case 0x32: _memory.write(fetch16(), A); break;
        
        // INX SP
        case 0x33: SP++; break;
        
        // INR M
        case 0x34:
            addr = get_hl(this);
            temp8 = _memory.read(addr) + 1;
            set_flags(temp8, flags & FLAG_C, ((_memory.read(addr) & 0x0F) + 1) > 0x0F);
            _memory.write(addr, temp8);
            break;
        
        // DCR M
        case 0x35:
            addr = get_hl(this);
            temp8 = _memory.read(addr) - 1;
            set_flags(temp8, flags & FLAG_C, !((_memory.read(addr) & 0x0F) < 1));
            _memory.write(addr, temp8);
            break;
        
        // MVI M,d8
        case 0x36: _memory.write(get_hl(this), fetch()); break;
        
        // STC
        case 0x37: flags |= FLAG_C; break;
        
        // DAD SP
        case 0x39:
            temp16 = get_hl(this) + SP;
            flags = (flags & ~FLAG_C) | ((temp16 > 0xFFFF) ? FLAG_C : 0);
            set_hl(this, temp16);
            break;
        
        // LDA addr
        case 0x3A: A = _memory.read(fetch16()); break;
        
        // DCX SP
        case 0x3B: SP--; break;
        
        // INR A
        case 0x3C:
            temp8 = A + 1;
            set_flags(temp8, flags & FLAG_C, ((A & 0x0F) + 1) > 0x0F);
            A = temp8;
            break;
        
        // DCR A
        case 0x3D:
            temp8 = A - 1;
            set_flags(temp8, flags & FLAG_C, !((A & 0x0F) < 1));
            A = temp8;
            break;
        
        // MVI A,d8
        case 0x3E: A = fetch(); break;
        
        // CMC
        case 0x3F: flags ^= FLAG_C; break;

        // MOV instructions (0x40-0x7F, except 0x76 which is HLT)
        // MOV B,B through MOV A,A
        case 0x40: B = B; break;
        case 0x41: B = C; break;
        case 0x42: B = D; break;
        case 0x43: B = E; break;
        case 0x44: B = H; break;
        case 0x45: B = L; break;
        case 0x46: B = _memory.read(get_hl(this)); break;
        case 0x47: B = A; break;
        
        case 0x48: C = B; break;
        case 0x49: C = C; break;
        case 0x4A: C = D; break;
        case 0x4B: C = E; break;
        case 0x4C: C = H; break;
        case 0x4D: C = L; break;
        case 0x4E: C = _memory.read(get_hl(this)); break;
        case 0x4F: C = A; break;
        
        case 0x50: D = B; break;
        case 0x51: D = C; break;
        case 0x52: D = D; break;
        case 0x53: D = E; break;
        case 0x54: D = H; break;
        case 0x55: D = L; break;
        case 0x56: D = _memory.read(get_hl(this)); break;
        case 0x57: D = A; break;
        
        case 0x58: E = B; break;
        case 0x59: E = C; break;
        case 0x5A: E = D; break;
        case 0x5B: E = E; break;
        case 0x5C: E = H; break;
        case 0x5D: E = L; break;
        case 0x5E: E = _memory.read(get_hl(this)); break;
        case 0x5F: E = A; break;
        
        case 0x60: H = B; break;
        case 0x61: H = C; break;
        case 0x62: H = D; break;
        case 0x63: H = E; break;
        case 0x64: H = H; break;
        case 0x65: H = L; break;
        case 0x66: H = _memory.read(get_hl(this)); break;
        case 0x67: H = A; break;
        
        case 0x68: L = B; break;
        case 0x69: L = C; break;
        case 0x6A: L = D; break;
        case 0x6B: L = E; break;
        case 0x6C: L = H; break;
        case 0x6D: L = L; break;
        case 0x6E: L = _memory.read(get_hl(this)); break;
        case 0x6F: L = A; break;
        
        case 0x70: _memory.write(get_hl(this), B); break;
        case 0x71: _memory.write(get_hl(this), C); break;
        case 0x72: _memory.write(get_hl(this), D); break;
        case 0x73: _memory.write(get_hl(this), E); break;
        case 0x74: _memory.write(get_hl(this), H); break;
        case 0x75: _memory.write(get_hl(this), L); break;
        
        // HLT
        case 0x76: _halted = true; break;
        
        case 0x77: _memory.write(get_hl(this), A); break;
        
        case 0x78: A = B; break;
        case 0x79: A = C; break;
        case 0x7A: A = D; break;
        case 0x7B: A = E; break;
        case 0x7C: A = H; break;
        case 0x7D: A = L; break;
        case 0x7E: A = _memory.read(get_hl(this)); break;
        case 0x7F: A = A; break;

        // ADD instructions (0x80-0x87)
        case 0x80: temp16 = A + B; set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (B & 0x0F)) > 0x0F); A = temp16; break;
        case 0x81: temp16 = A + C; set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (C & 0x0F)) > 0x0F); A = temp16; break;
        case 0x82: temp16 = A + D; set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (D & 0x0F)) > 0x0F); A = temp16; break;
        case 0x83: temp16 = A + E; set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (E & 0x0F)) > 0x0F); A = temp16; break;
        case 0x84: temp16 = A + H; set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (H & 0x0F)) > 0x0F); A = temp16; break;
        case 0x85: temp16 = A + L; set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (L & 0x0F)) > 0x0F); A = temp16; break;
        case 0x86: temp8 = _memory.read(get_hl(this)); temp16 = A + temp8; set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (temp8 & 0x0F)) > 0x0F); A = temp16; break;
        case 0x87: temp16 = A + A; set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (A & 0x0F)) > 0x0F); A = temp16; break;

        // ADC instructions (0x88-0x8F)
        case 0x88: temp16 = A + B + (flags & FLAG_C); set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (B & 0x0F) + (flags & FLAG_C)) > 0x0F); A = temp16; break;
        case 0x89: temp16 = A + C + (flags & FLAG_C); set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (C & 0x0F) + (flags & FLAG_C)) > 0x0F); A = temp16; break;
        case 0x8A: temp16 = A + D + (flags & FLAG_C); set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (D & 0x0F) + (flags & FLAG_C)) > 0x0F); A = temp16; break;
        case 0x8B: temp16 = A + E + (flags & FLAG_C); set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (E & 0x0F) + (flags & FLAG_C)) > 0x0F); A = temp16; break;
        case 0x8C: temp16 = A + H + (flags & FLAG_C); set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (H & 0x0F) + (flags & FLAG_C)) > 0x0F); A = temp16; break;
        case 0x8D: temp16 = A + L + (flags & FLAG_C); set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (L & 0x0F) + (flags & FLAG_C)) > 0x0F); A = temp16; break;
        case 0x8E: temp8 = _memory.read(get_hl(this)); temp16 = A + temp8 + (flags & FLAG_C); set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (temp8 & 0x0F) + (flags & FLAG_C)) > 0x0F); A = temp16; break;
        case 0x8F: temp16 = A + A + (flags & FLAG_C); set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (A & 0x0F) + (flags & FLAG_C)) > 0x0F); A = temp16; break;

        // SUB instructions (0x90-0x97)
        case 0x90: result16 = A - B; set_flags(result16, result16 < 0, (A & 0x0F) < (B & 0x0F)); A = result16; break;
        case 0x91: result16 = A - C; set_flags(result16, result16 < 0, (A & 0x0F) < (C & 0x0F)); A = result16; break;
        case 0x92: result16 = A - D; set_flags(result16, result16 < 0, (A & 0x0F) < (D & 0x0F)); A = result16; break;
        case 0x93: result16 = A - E; set_flags(result16, result16 < 0, (A & 0x0F) < (E & 0x0F)); A = result16; break;
        case 0x94: result16 = A - H; set_flags(result16, result16 < 0, (A & 0x0F) < (H & 0x0F)); A = result16; break;
        case 0x95: result16 = A - L; set_flags(result16, result16 < 0, (A & 0x0F) < (L & 0x0F)); A = result16; break;
        case 0x96: temp8 = _memory.read(get_hl(this)); result16 = A - temp8; set_flags(result16, result16 < 0, (A & 0x0F) < (temp8 & 0x0F)); A = result16; break;
        case 0x97: result16 = A - A; set_flags(result16, result16 < 0, (A & 0x0F) < (A & 0x0F)); A = result16; break;

        // SBB instructions (0x98-0x9F)
        case 0x98: result16 = A - B - (flags & FLAG_C); set_flags(result16, result16 < 0, (A & 0x0F) < ((B & 0x0F) + (flags & FLAG_C))); A = result16; break;
        case 0x99: result16 = A - C - (flags & FLAG_C); set_flags(result16, result16 < 0, (A & 0x0F) < ((C & 0x0F) + (flags & FLAG_C))); A = result16; break;
        case 0x9A: result16 = A - D - (flags & FLAG_C); set_flags(result16, result16 < 0, (A & 0x0F) < ((D & 0x0F) + (flags & FLAG_C))); A = result16; break;
        case 0x9B: result16 = A - E - (flags & FLAG_C); set_flags(result16, result16 < 0, (A & 0x0F) < ((E & 0x0F) + (flags & FLAG_C))); A = result16; break;
        case 0x9C: result16 = A - H - (flags & FLAG_C); set_flags(result16, result16 < 0, (A & 0x0F) < ((H & 0x0F) + (flags & FLAG_C))); A = result16; break;
        case 0x9D: result16 = A - L - (flags & FLAG_C); set_flags(result16, result16 < 0, (A & 0x0F) < ((L & 0x0F) + (flags & FLAG_C))); A = result16; break;
        case 0x9E: temp8 = _memory.read(get_hl(this)); result16 = A - temp8 - (flags & FLAG_C); set_flags(result16, result16 < 0, (A & 0x0F) < ((temp8 & 0x0F) + (flags & FLAG_C))); A = result16; break;
        case 0x9F: result16 = A - A - (flags & FLAG_C); set_flags(result16, result16 < 0, (A & 0x0F) < ((A & 0x0F) + (flags & FLAG_C))); A = result16; break;

        // ANA instructions (0xA0-0xA7)
        case 0xA0: A &= B; set_flags(A, false, ((A | B) & 0x08) != 0); break;
        case 0xA1: A &= C; set_flags(A, false, ((A | C) & 0x08) != 0); break;
        case 0xA2: A &= D; set_flags(A, false, ((A | D) & 0x08) != 0); break;
        case 0xA3: A &= E; set_flags(A, false, ((A | E) & 0x08) != 0); break;
        case 0xA4: A &= H; set_flags(A, false, ((A | H) & 0x08) != 0); break;
        case 0xA5: A &= L; set_flags(A, false, ((A | L) & 0x08) != 0); break;
        case 0xA6: temp8 = _memory.read(get_hl(this)); A &= temp8; set_flags(A, false, ((A | temp8) & 0x08) != 0); break;
        case 0xA7: A &= A; set_flags(A, false, ((A | A) & 0x08) != 0); break;

        // XRA instructions (0xA8-0xAF)
        case 0xA8: A ^= B; set_flags(A, false, false); break;
        case 0xA9: A ^= C; set_flags(A, false, false); break;
        case 0xAA: A ^= D; set_flags(A, false, false); break;
        case 0xAB: A ^= E; set_flags(A, false, false); break;
        case 0xAC: A ^= H; set_flags(A, false, false); break;
        case 0xAD: A ^= L; set_flags(A, false, false); break;
        case 0xAE: A ^= _memory.read(get_hl(this)); set_flags(A, false, false); break;
        case 0xAF: A ^= A; set_flags(A, false, false); break;

        // ORA instructions (0xB0-0xB7)
        case 0xB0: A |= B; set_flags(A, false, false); break;
        case 0xB1: A |= C; set_flags(A, false, false); break;
        case 0xB2: A |= D; set_flags(A, false, false); break;
        case 0xB3: A |= E; set_flags(A, false, false); break;
        case 0xB4: A |= H; set_flags(A, false, false); break;
        case 0xB5: A |= L; set_flags(A, false, false); break;
        case 0xB6: A |= _memory.read(get_hl(this)); set_flags(A, false, false); break;
        case 0xB7: A |= A; set_flags(A, false, false); break;

        // CMP instructions (0xB8-0xBF)
        case 0xB8: result16 = A - B; set_flags(result16, result16 < 0, (A & 0x0F) < (B & 0x0F)); break;
        case 0xB9: result16 = A - C; set_flags(result16, result16 < 0, (A & 0x0F) < (C & 0x0F)); break;
        case 0xBA: result16 = A - D; set_flags(result16, result16 < 0, (A & 0x0F) < (D & 0x0F)); break;
        case 0xBB: result16 = A - E; set_flags(result16, result16 < 0, (A & 0x0F) < (E & 0x0F)); break;
        case 0xBC: result16 = A - H; set_flags(result16, result16 < 0, (A & 0x0F) < (H & 0x0F)); break;
        case 0xBD: result16 = A - L; set_flags(result16, result16 < 0, (A & 0x0F) < (L & 0x0F)); break;
        case 0xBE: temp8 = _memory.read(get_hl(this)); result16 = A - temp8; set_flags(result16, result16 < 0, (A & 0x0F) < (temp8 & 0x0F)); break;
        case 0xBF: result16 = A - A; set_flags(result16, result16 < 0, (A & 0x0F) < (A & 0x0F)); break;

        // Conditional returns
        case 0xC0: if (!(flags & FLAG_Z)) { PC = pop(); } break;  // RNZ
        case 0xC8: if (flags & FLAG_Z) { PC = pop(); } break;     // RZ
        case 0xD0: if (!(flags & FLAG_C)) { PC = pop(); } break;  // RNC
        case 0xD8: if (flags & FLAG_C) { PC = pop(); } break;     // RC
        case 0xE0: if (!(flags & FLAG_P)) { PC = pop(); } break;  // RPO
        case 0xE8: if (flags & FLAG_P) { PC = pop(); } break;     // RPE
        case 0xF0: if (!(flags & FLAG_S)) { PC = pop(); } break;  // RP
        case 0xF8: if (flags & FLAG_S) { PC = pop(); } break;     // RM

        // POP instructions
        case 0xC1: set_bc(this, pop()); break;
        case 0xD1: set_de(this, pop()); break;
        case 0xE1: set_hl(this, pop()); break;
        case 0xF1: temp16 = pop(); A = temp16 >> 8; flags = (temp16 & 0xFF) | 0x02; break;

        // Conditional jumps
        case 0xC2: addr = fetch16(); if (!(flags & FLAG_Z)) PC = addr; break;  // JNZ
        case 0xCA: addr = fetch16(); if (flags & FLAG_Z) PC = addr; break;     // JZ
        case 0xD2: addr = fetch16(); if (!(flags & FLAG_C)) PC = addr; break;  // JNC
        case 0xDA: addr = fetch16(); if (flags & FLAG_C) PC = addr; break;     // JC
        case 0xE2: addr = fetch16(); if (!(flags & FLAG_P)) PC = addr; break;  // JPO
        case 0xEA: addr = fetch16(); if (flags & FLAG_P) PC = addr; break;     // JPE
        case 0xF2: addr = fetch16(); if (!(flags & FLAG_S)) PC = addr; break;  // JP
        case 0xFA: addr = fetch16(); if (flags & FLAG_S) PC = addr; break;     // JM

        // JMP
        case 0xC3: PC = fetch16(); break;

        // Conditional calls
        case 0xC4: addr = fetch16(); if (!(flags & FLAG_Z)) { push(PC); PC = addr; } break;  // CNZ
        case 0xCC: addr = fetch16(); if (flags & FLAG_Z) { push(PC); PC = addr; } break;     // CZ
        case 0xD4: addr = fetch16(); if (!(flags & FLAG_C)) { push(PC); PC = addr; } break;  // CNC
        case 0xDC: addr = fetch16(); if (flags & FLAG_C) { push(PC); PC = addr; } break;     // CC
        case 0xE4: addr = fetch16(); if (!(flags & FLAG_P)) { push(PC); PC = addr; } break;  // CPO
        case 0xEC: addr = fetch16(); if (flags & FLAG_P) { push(PC); PC = addr; } break;     // CPE
        case 0xF4: addr = fetch16(); if (!(flags & FLAG_S)) { push(PC); PC = addr; } break;  // CP
        case 0xFC: addr = fetch16(); if (flags & FLAG_S) { push(PC); PC = addr; } break;     // CM

        // PUSH instructions
        case 0xC5: push(get_bc(this)); break;
        case 0xD5: push(get_de(this)); break;
        case 0xE5: push(get_hl(this)); break;
        case 0xF5: push((A << 8) | flags); break;

        // ADI, ACI, SUI, SBI, ANI, XRI, ORI, CPI
        case 0xC6: temp8 = fetch(); temp16 = A + temp8; set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (temp8 & 0x0F)) > 0x0F); A = temp16; break;
        case 0xCE: temp8 = fetch(); temp16 = A + temp8 + (flags & FLAG_C); set_flags(temp16, temp16 > 0xFF, ((A & 0x0F) + (temp8 & 0x0F) + (flags & FLAG_C)) > 0x0F); A = temp16; break;
        case 0xD6: temp8 = fetch(); result16 = A - temp8; set_flags(result16, result16 < 0, (A & 0x0F) < (temp8 & 0x0F)); A = result16; break;
        case 0xDE: temp8 = fetch(); result16 = A - temp8 - (flags & FLAG_C); set_flags(result16, result16 < 0, (A & 0x0F) < ((temp8 & 0x0F) + (flags & FLAG_C))); A = result16; break;
        case 0xE6: temp8 = fetch(); A &= temp8; set_flags(A, false, ((A | temp8) & 0x08) != 0); break;
        case 0xEE: temp8 = fetch(); A ^= temp8; set_flags(A, false, false); break;
        case 0xF6: temp8 = fetch(); A |= temp8; set_flags(A, false, false); break;
        case 0xFE: temp8 = fetch(); result16 = A - temp8; set_flags(result16, result16 < 0, (A & 0x0F) < (temp8 & 0x0F)); break;

        // RST instructions
        case 0xC7: push(PC); PC = 0x00; break;
        case 0xCF: push(PC); PC = 0x08; break;
        case 0xD7: push(PC); PC = 0x10; break;
        case 0xDF: push(PC); PC = 0x18; break;
        case 0xE7: push(PC); PC = 0x20; break;
        case 0xEF: push(PC); PC = 0x28; break;
        case 0xF7: push(PC); PC = 0x30; break;
        case 0xFF: push(PC); PC = 0x38; break;

        // RET
        case 0xC9: PC = pop(); break;

        // CALL
        case 0xCD: addr = fetch16(); push(PC); PC = addr; break;

        // OUT
        case 0xD3: _ports.out(fetch(), A, this); break;

        // IN
        case 0xDB: A = _ports.in(fetch(), this); break;

        // XTHL
        case 0xE3:
            temp8 = L; L = _memory.read(SP); _memory.write(SP, temp8);
            temp8 = H; H = _memory.read(SP + 1); _memory.write(SP + 1, temp8);
            break;

        // XCHG
        case 0xEB:
            temp8 = H; H = D; D = temp8;
            temp8 = L; L = E; E = temp8;
            break;

        // DI
        case 0xF3: _inte = false; break;

        // EI
        case 0xFB: _inte = true; break;

        // PCHL
        case 0xE9: PC = get_hl(this); break;

        // SPHL
        case 0xF9: SP = get_hl(this); break;

        default:
            // Unknown opcode
            Serial.print("Unknown opcode: 0x");
            Serial.println(opcode, HEX);
            break;
    }
}
