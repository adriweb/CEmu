#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

#include <core/defines.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    uint16_t hl, hls;
    struct {
        uint8_t l;
        uint8_t h;
    };
} short_reg_t;

typedef union {
    uint32_t hl;
    uint16_t hls;
    struct {
        uint8_t l;
        uint8_t h;
        uint8_t u;
    };
} long_reg_t;

typedef struct {
    union {
        short_reg_t af;
        uint16_t AF;
        struct {
            union {
                uint8_t F;
                struct {
                    uint8_t C  : 1;
                    uint8_t N  : 1;
                    uint8_t PV : 1;
                    uint8_t _3 : 1;
                    uint8_t H  : 1;
                    uint8_t _5 : 1;
                    uint8_t Z  : 1;
                    uint8_t S  : 1;
                } flags;
            };
            uint8_t A;
        };
    };
    union {
        long_reg_t bc;
        uint32_t BC;
        uint16_t BCS;
        struct {
            uint8_t C;
            uint8_t B;
            uint8_t BCU;
        };
    };
    union {
        long_reg_t de;
        uint32_t DE;
        uint16_t DES;
        struct {
            uint8_t E;
            uint8_t D;
            uint8_t DEU;
        };
    };
    union {
        long_reg_t index[4];
        struct {
            union {
                long_reg_t hl;
                uint32_t HL;
                uint16_t HLs;
                struct {
                    uint8_t L;
                    uint8_t H;
                    uint8_t HLU;
                };
            };
            uint32_t _HL;
            union {
                long_reg_t ix;
                uint32_t IX;
                uint16_t IXS;
                struct {
                    uint8_t IXL;
                    uint8_t IXH;
                    uint8_t IXU;
                };
            };
            union {
                long_reg_t iy;
                uint32_t IY;
                uint16_t IYS;
                struct {
                    uint8_t IYL;
                    uint8_t IYH;
                    uint8_t IYU;
                };
            };
        };
    };
    uint16_t _AF;
    uint32_t _BC, _DE;
    union {
        long_reg_t stack[2];
        struct {
            uint16_t SPS, SPSU;
            uint32_t SPL;
        };
    };
    union {
        long_reg_t pc;
        uint32_t PC;
        uint16_t PCS;
        struct {
            uint8_t PCL;
            uint8_t PCH;
            uint8_t PCU;
        };
    };
    uint16_t I;
    uint8_t R, MBASE;  /* interrupt, r, and z80 MBASE */
} eZ80registers_t;

typedef enum {
    FLAG_S =  1 << 7,
    FLAG_Z =  1 << 6,
    FLAG_5 =  1 << 5,
    FLAG_H =  1 << 4,
    FLAG_3 =  1 << 3,
    FLAG_PV = 1 << 2,
    FLAG_N  = 1 << 1,
    FLAG_C  = 1 << 0,
    FLAG_NONE = 0
} eZ80flags;

void exx(eZ80registers_t *r);
int parity(uint8_t x);

/* S Z 5 H 3 PV N C */
#define __flag_s(a)  ((a) ? FLAG_S  : 0)
#define __flag_5(a)  ((a) ? FLAG_5  : 0)
#define __flag_h(a)  ((a) ? FLAG_H  : 0)
#define __flag_3(a)  ((a) ? FLAG_3  : 0)
#define __flag_pv(a) ((a) ? FLAG_PV : 0)
#define __flag_c(a)  ((a) ? FLAG_C  : 0)

#define _flag_carry_w(a,mode) __flag_c((a) & (0x10000<<(mode<<3)))
#define _flag_carry_b(a) __flag_c((a) & 0x100)

#define _flag_sign_w(a,mode) __flag_s((a) & (0x8000<<(mode<<3)))
#define _flag_sign_b(a) __flag_s((a) & 0x80)

#define _flag_parity(a) __flag_pv(!parity(a))

#define _flag_undef(a) (a & (FLAG_3 | FLAG_5))

#define _flag_overflow_b_add(op1, op2, result) __flag_pv(~((op1) ^ (op2)) & ((op1) ^ (result)) & 0x80)
#define _flag_overflow_b_sub(op1, op2, result) __flag_pv(((op1) ^ (op2)) & ((op1) ^ (result)) & 0x80)
#define _flag_overflow_w_add(op1, op2, result, mode) __flag_pv(~((op1) ^ (op2)) & ((op1) ^ (result)) & (0x8000 << ((mode) << 3)))
#define _flag_overflow_w_sub(op1, op2, result, mode) __flag_pv(((op1) ^ (op2)) & ((op1) ^ (result)) & (0x8000 << ((mode) << 3)))

#define _flag_halfcarry_b_add(op1, op2, carry) __flag_h(((op1 & 0xf) + (op2 & 0xf) + carry) & 0x10)
#define _flag_halfcarry_b_sub(op1, op2, carry) __flag_h(((op1 & 0xf) - (op2 & 0xf) - carry) & 0x10)
#define _flag_halfcarry_w_add(op1, op2, carry) __flag_h(((op1 & 0xfff) + (op2 & 0xfff) + carry) & 0x1000)
#define _flag_halfcarry_w_sub(op1, op2, carry) __flag_h(((op1 & 0xfff) - (op2 & 0xfff) - carry) & 0x1000)

#define _flag_subtract(a)   ((a) ? FLAG_N : 0)
#define _flag_zero(a)       ((a) ? 0 : FLAG_Z)

#define mask_mode(a, mode)  (a &= (0xFFFF<<(mode<<3))|0xFF)

#ifdef __cplusplus
}
#endif

#endif
