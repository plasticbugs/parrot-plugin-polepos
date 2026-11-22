// made by tools/z8kgen.py from MAME 0.288 (src/devices/cpu/z8000: z8000ops.hxx,
// z8000tbl.hxx, z8000cpu.h, z8000dab.h; Juergen Buchmueller, Ernesto Corvi; BSD-3-Clause). Do not edit.
// #included by z8k.c, which supplies the CPU's state (S, z8k_st_t) and memory.

// z8000cpu.h: the register file, the vectors, the flags and the operand fields
#define RB(n)   m_regs.B[BYTE8_XOR_BE((((n) & 7) << 1) | (((n) & 8) >> 3))]
#define RW(n)   m_regs.W[BYTE4_XOR_BE(n)]
#define RL(n)   m_regs.L[BYTE_XOR_BE((n) >> 1)]
#define RQ(n)   m_regs.Q[(n) >> 2]
#define SP      (get_segmented_mode() ? 14 : 15)
#define RST     (PSA_ADDR() + 0)  /* start up m_fcw and m_pc */
#define EPU     (PSA_ADDR() + m_vector_mult * 0x0004)  /* extension processor unit? trap */
#define TRAP    (PSA_ADDR() + m_vector_mult * 0x0008)  /* privilege violation trap */
#define SYSCALL (PSA_ADDR() + m_vector_mult * 0x000c)  /* system call SC */
#define SEGTRAP (PSA_ADDR() + m_vector_mult * 0x0010)  /* segment trap */
#define NMI     (PSA_ADDR() + m_vector_mult * 0x0014)  /* non maskable interrupt */
#define NVI     (PSA_ADDR() + m_vector_mult * 0x0018)  /* non vectored interrupt */
#define VI      (PSA_ADDR() + m_vector_mult * 0x001c)  /* vectored interrupt */
#define VEC00   (PSA_ADDR() + m_vector_mult * 0x001e)  /* vector n m_pc value */
#define F_SEG   0x8000              /* segmented mode (Z8001 only) */
#define F_S_N   0x4000              /* system / normal mode */
#define F_EPU   0x2000              /* extension processor unit? */
#define F_VIE   0x1000              /* vectored interrupt enable */
#define F_NVIE  0x0800              /* non vectored interrupt enable */
#define F_10    0x0400              /* unused */
#define F_9     0x0200              /* unused */
#define F_8     0x0100              /* unused */
#define F_C     0x0080              /* carry flag */
#define F_Z     0x0040              /* zero flag */
#define F_S     0x0020              /* sign flag */
#define F_PV    0x0010              /* parity/overflow flag */
#define F_DA    0x0008              /* decimal adjust flag (0 add/adc, 1 sub/sbc) */
#define F_H     0x0004              /* half carry flag (byte arithmetic only) */
#define F_1     0x0002              /* unused */
#define F_0     0x0001              /* unused */
#define OP0     0
#define OP1     1
#define OP2     2
#define NIB0    12
#define NIB1    8
#define NIB2    4
#define NIB3    0
#define S08 0x80
#define S16 0x8000
#define S32 0x80000000
#define GET_C       ((m_fcw >> 7) & 1)
#define GET_Z       ((m_fcw >> 6) & 1)
#define GET_S       ((m_fcw >> 5) & 1)
#define GET_PV      ((m_fcw >> 4) & 1)
#define GET_DA      ((m_fcw >> 3) & 1)
#define GET_H       ((m_fcw >> 2) & 1)
#define CLR_C       m_fcw &= ~F_C
#define CLR_Z       m_fcw &= ~F_Z
#define CLR_S       m_fcw &= ~F_S
#define CLR_P       m_fcw &= ~F_PV
#define CLR_V       m_fcw &= ~F_PV
#define CLR_DA      m_fcw &= ~F_DA
#define CLR_H       m_fcw &= ~F_H
#define CLR_CZS     m_fcw &= ~(F_C|F_Z|F_S)
#define CLR_CZSP    m_fcw &= ~(F_C|F_Z|F_S|F_PV)
#define CLR_CZSV    m_fcw &= ~(F_C|F_Z|F_S|F_PV)
#define CLR_CZSVH   m_fcw &= ~(F_C|F_Z|F_S|F_PV|F_H)
#define CLR_ZS      m_fcw &= ~(F_Z|F_S)
#define CLR_ZSV     m_fcw &= ~(F_Z|F_S|F_PV)
#define CLR_ZSP     m_fcw &= ~(F_Z|F_S|F_PV)
#define SET_C       m_fcw |= F_C
#define SET_Z       m_fcw |= F_Z
#define SET_S       m_fcw |= F_S
#define SET_P       m_fcw |= F_PV
#define SET_V       m_fcw |= F_PV
#define SET_DA      m_fcw |= F_DA
#define SET_H       m_fcw |= F_H
#define SET_SC      m_fcw |= F_C | F_S
#define CC0 (0)                         /* always false */
#define CC1 (GET_PV^GET_S)              /* less than */
#define CC2 (GET_Z|(GET_PV^GET_S))      /* less than or equal */
#define CC3 (GET_Z|GET_C)               /* unsigned less than or equal */
#define CC4 GET_PV                      /* parity even / overflow */
#define CC5 GET_S                       /* minus (signed) */
#define CC6 GET_Z                       /* zero / equal */
#define CC7 GET_C                       /* carry / unsigned less than */
#define CC8 (1)                         /* always true */
#define CC9 !(GET_PV^GET_S)             /* greater than or equal */
#define CCA !(GET_Z|(GET_PV^GET_S))     /* greater than */
#define CCB !(GET_Z|GET_C)              /* unsigned greater than */
#define CCC !GET_PV                     /* parity odd / no overflow */
#define CCD !GET_S                      /* plus (not signed) */
#define CCE !GET_Z                      /* not zero / not equal */
#define CCF !GET_C                      /* not carry / unsigned greater than */
#define GET_BIT(o)      uint16_t bit = 1 << (get_operand(o) & 15)
#define GET_CCC(o,s)    uint8_t cc = (get_operand(o) >> (s)) & 15
#define GET_DST(o,s)    uint8_t dst = (get_operand(o) >> (s)) & 15
#define GET_SRC(o,s)    uint8_t src = (get_operand(o) >> (s)) & 15
#define GET_IDX(o,s)    uint8_t idx = (get_operand(o) >> (s)) & 15
#define GET_CNT(o,s)    int8_t cnt = (get_operand(o) >> (s)) & 15
#define GET_IMM4(o,s)   uint8_t imm4 = (get_operand(o) >> (s)) & 15
#define GET_I4M1(o,s)   uint8_t i4p1 = ((get_operand(o) >> (s)) & 15) + 1
#define GET_IMM1(o,s)   uint8_t imm1 = (get_operand(o) >> (s)) & 2
#define GET_IMM2(o,s)   uint8_t imm2 = (get_operand(o) >> (s)) & 3
#define GET_IMM3(o,s)   uint8_t imm3 = (get_operand(o) >> (s)) & 7
#define GET_IMM8(o)     uint8_t imm8 = (uint8_t)get_operand(o)
#define GET_IMM16(o)    uint16_t imm16 = get_operand(o)
#define GET_IDX16(o)    uint32_t idx16 = get_operand(o)
#define GET_IMM32       uint32_t imm32 = get_operand(1); imm32 = (imm32 << 16) + get_operand(2)
#define GET_DSP7        uint8_t dsp7 = get_operand(0) & 127
#define GET_DSP8        int8_t dsp8 = (int8_t)get_operand(0)
#define GET_DSP16       uint16_t tmp16 = get_operand(1); uint32_t dsp16 = addr_add(m_pc, (int16_t)tmp16)
#define GET_ADDR(o)     uint32_t addr = (uint32_t)get_addr_operand(o)
#define GET_ADDR_RAW(o)     uint32_t addr = (uint32_t)get_raw_addr_operand(o)

static const uint16_t Z8000_dab[0x800] = {
	/* add/adc results */
	0x000,0x001,0x002,0x003,0x004,0x005,0x006,0x007,
	0x008,0x009,0x010,0x011,0x012,0x013,0x014,0x015,
	0x010,0x011,0x012,0x013,0x014,0x015,0x016,0x017,
	0x018,0x019,0x020,0x021,0x022,0x023,0x024,0x025,
	0x020,0x021,0x022,0x023,0x024,0x025,0x026,0x027,
	0x028,0x029,0x030,0x031,0x032,0x033,0x034,0x035,
	0x030,0x031,0x032,0x033,0x034,0x035,0x036,0x037,
	0x038,0x039,0x040,0x041,0x042,0x043,0x044,0x045,
	0x040,0x041,0x042,0x043,0x044,0x045,0x046,0x047,
	0x048,0x049,0x050,0x051,0x052,0x053,0x054,0x055,
	0x050,0x051,0x052,0x053,0x054,0x055,0x056,0x057,
	0x058,0x059,0x060,0x061,0x062,0x063,0x064,0x065,
	0x060,0x061,0x062,0x063,0x064,0x065,0x066,0x067,
	0x068,0x069,0x070,0x071,0x072,0x073,0x074,0x075,
	0x070,0x071,0x072,0x073,0x074,0x075,0x076,0x077,
	0x078,0x079,0x080,0x081,0x082,0x083,0x084,0x085,
	0x080,0x081,0x082,0x083,0x084,0x085,0x086,0x087,
	0x088,0x089,0x090,0x091,0x092,0x093,0x094,0x095,
	0x090,0x091,0x092,0x093,0x094,0x095,0x096,0x097,
	0x098,0x099,0x100,0x101,0x102,0x103,0x104,0x105,
	0x100,0x101,0x102,0x103,0x104,0x105,0x106,0x107,
	0x108,0x109,0x110,0x111,0x112,0x113,0x114,0x115,
	0x110,0x111,0x112,0x113,0x114,0x115,0x116,0x117,
	0x118,0x119,0x120,0x121,0x122,0x123,0x124,0x125,
	0x120,0x121,0x122,0x123,0x124,0x125,0x126,0x127,
	0x128,0x129,0x130,0x131,0x132,0x133,0x134,0x135,
	0x130,0x131,0x132,0x133,0x134,0x135,0x136,0x137,
	0x138,0x139,0x140,0x141,0x142,0x143,0x144,0x145,
	0x140,0x141,0x142,0x143,0x144,0x145,0x146,0x147,
	0x148,0x149,0x150,0x151,0x152,0x153,0x154,0x155,
	0x150,0x151,0x152,0x153,0x154,0x155,0x156,0x157,
	0x158,0x159,0x160,0x161,0x162,0x163,0x164,0x165,
	0x166,0x167,0x168,0x169,0x16a,0x16b,0x16c,0x16d,
	0x16e,0x16f,0x170,0x171,0x172,0x173,0x174,0x175,
	0x176,0x177,0x178,0x179,0x17a,0x17b,0x17c,0x17d,
	0x17e,0x17f,0x180,0x181,0x182,0x183,0x184,0x185,
	0x186,0x187,0x188,0x189,0x18a,0x18b,0x18c,0x18d,
	0x18e,0x18f,0x190,0x191,0x192,0x193,0x194,0x195,
	0x196,0x197,0x198,0x199,0x19a,0x19b,0x19c,0x19d,
	0x19e,0x19f,0x1a0,0x1a1,0x1a2,0x1a3,0x1a4,0x1a5,
	0x1a6,0x1a7,0x1a8,0x1a9,0x1aa,0x1ab,0x1ac,0x1ad,
	0x1ae,0x1af,0x1b0,0x1b1,0x1b2,0x1b3,0x1b4,0x1b5,
	0x1b6,0x1b7,0x1b8,0x1b9,0x1ba,0x1bb,0x1bc,0x1bd,
	0x1be,0x1bf,0x1c0,0x1c1,0x1c2,0x1c3,0x1c4,0x1c5,
	0x1c6,0x1c7,0x1c8,0x1c9,0x1ca,0x1cb,0x1cc,0x1cd,
	0x1ce,0x1cf,0x1d0,0x1d1,0x1d2,0x1d3,0x1d4,0x1d5,
	0x1d6,0x1d7,0x1d8,0x1d9,0x1da,0x1db,0x1dc,0x1dd,
	0x1de,0x1df,0x1e0,0x1e1,0x1e2,0x1e3,0x1e4,0x1e5,
	0x1e6,0x1e7,0x1e8,0x1e9,0x1ea,0x1eb,0x1ec,0x1ed,
	0x1ee,0x1ef,0x1f0,0x1f1,0x1f2,0x1f3,0x1f4,0x1f5,
	0x1f6,0x1f7,0x1f8,0x1f9,0x1fa,0x1fb,0x1fc,0x1fd,
	0x1fe,0x1ff,0x100,0x101,0x102,0x103,0x104,0x105,
	0x106,0x107,0x108,0x109,0x10a,0x10b,0x10c,0x10d,
	0x10e,0x10f,0x110,0x111,0x112,0x113,0x114,0x115,
	0x116,0x117,0x118,0x119,0x11a,0x11b,0x11c,0x11d,
	0x11e,0x11f,0x120,0x121,0x122,0x123,0x124,0x125,
	0x126,0x127,0x128,0x129,0x12a,0x12b,0x12c,0x12d,
	0x12e,0x12f,0x130,0x131,0x132,0x133,0x134,0x135,
	0x136,0x137,0x138,0x139,0x13a,0x13b,0x13c,0x13d,
	0x13e,0x13f,0x140,0x141,0x142,0x143,0x144,0x145,
	0x146,0x147,0x148,0x149,0x14a,0x14b,0x14c,0x14d,
	0x14e,0x14f,0x150,0x151,0x152,0x153,0x154,0x155,
	0x156,0x157,0x158,0x159,0x15a,0x15b,0x15c,0x15d,
	0x15e,0x15f,0x160,0x161,0x162,0x163,0x164,0x165,
	0x166,0x167,0x168,0x169,0x16a,0x16b,0x16c,0x16d,
	0x16e,0x16f,0x170,0x171,0x172,0x173,0x174,0x175,
	0x176,0x177,0x178,0x179,0x17a,0x17b,0x17c,0x17d,
	0x17e,0x17f,0x180,0x181,0x182,0x183,0x184,0x185,
	0x186,0x187,0x188,0x189,0x18a,0x18b,0x18c,0x18d,
	0x18e,0x18f,0x190,0x191,0x192,0x193,0x194,0x195,
	0x196,0x197,0x198,0x199,0x19a,0x19b,0x19c,0x19d,
	0x19e,0x19f,0x1a0,0x1a1,0x1a2,0x1a3,0x1a4,0x1a5,
	0x1a6,0x1a7,0x1a8,0x1a9,0x1aa,0x1ab,0x1ac,0x1ad,
	0x1ae,0x1af,0x1b0,0x1b1,0x1b2,0x1b3,0x1b4,0x1b5,
	0x1b6,0x1b7,0x1b8,0x1b9,0x1ba,0x1bb,0x1bc,0x1bd,
	0x1be,0x1bf,0x1c0,0x1c1,0x1c2,0x1c3,0x1c4,0x1c5,
	0x1c6,0x1c7,0x1c8,0x1c9,0x1ca,0x1cb,0x1cc,0x1cd,
	0x1ce,0x1cf,0x1d0,0x1d1,0x1d2,0x1d3,0x1d4,0x1d5,
	0x1d6,0x1d7,0x1d8,0x1d9,0x1da,0x1db,0x1dc,0x1dd,
	0x1de,0x1df,0x1e0,0x1e1,0x1e2,0x1e3,0x1e4,0x1e5,
	0x1e6,0x1e7,0x1e8,0x1e9,0x1ea,0x1eb,0x1ec,0x1ed,
	0x1ee,0x1ef,0x1f0,0x1f1,0x1f2,0x1f3,0x1f4,0x1f5,
	0x1f6,0x1f7,0x1f8,0x1f9,0x1fa,0x1fb,0x1fc,0x1fd,
	0x1fe,0x1ff,0x100,0x101,0x102,0x103,0x104,0x105,
	0x106,0x107,0x108,0x109,0x10a,0x10b,0x10c,0x10d,
	0x10e,0x10f,0x110,0x111,0x112,0x113,0x114,0x115,
	0x116,0x117,0x118,0x119,0x11a,0x11b,0x11c,0x11d,
	0x11e,0x11f,0x120,0x121,0x122,0x123,0x124,0x125,
	0x126,0x127,0x128,0x129,0x12a,0x12b,0x12c,0x12d,
	0x12e,0x12f,0x130,0x131,0x132,0x133,0x134,0x135,
	0x136,0x137,0x138,0x139,0x13a,0x13b,0x13c,0x13d,
	0x13e,0x13f,0x140,0x141,0x142,0x143,0x144,0x145,
	0x146,0x147,0x148,0x149,0x14a,0x14b,0x14c,0x14d,
	0x14e,0x14f,0x150,0x151,0x152,0x153,0x154,0x155,
	0x156,0x157,0x158,0x159,0x15a,0x15b,0x15c,0x15d,
	0x15e,0x15f,0x160,0x161,0x162,0x163,0x164,0x165,
	0x166,0x167,0x168,0x169,0x16a,0x16b,0x16c,0x16d,
	0x16e,0x16f,0x170,0x171,0x172,0x173,0x174,0x175,
	0x176,0x177,0x178,0x179,0x17a,0x17b,0x17c,0x17d,
	0x17e,0x17f,0x180,0x181,0x182,0x183,0x184,0x185,
	0x186,0x187,0x188,0x189,0x18a,0x18b,0x18c,0x18d,
	0x18e,0x18f,0x190,0x191,0x192,0x193,0x194,0x195,
	0x196,0x197,0x198,0x199,0x19a,0x19b,0x19c,0x19d,
	0x19e,0x19f,0x1a0,0x1a1,0x1a2,0x1a3,0x1a4,0x1a5,
	0x1a6,0x1a7,0x1a8,0x1a9,0x1aa,0x1ab,0x1ac,0x1ad,
	0x1ae,0x1af,0x1b0,0x1b1,0x1b2,0x1b3,0x1b4,0x1b5,
	0x1b6,0x1b7,0x1b8,0x1b9,0x1ba,0x1bb,0x1bc,0x1bd,
	0x1be,0x1bf,0x1c0,0x1c1,0x1c2,0x1c3,0x1c4,0x1c5,
	0x1c6,0x1c7,0x1c8,0x1c9,0x1ca,0x1cb,0x1cc,0x1cd,
	0x1ce,0x1cf,0x1d0,0x1d1,0x1d2,0x1d3,0x1d4,0x1d5,
	0x1d6,0x1d7,0x1d8,0x1d9,0x1da,0x1db,0x1dc,0x1dd,
	0x1de,0x1df,0x1e0,0x1e1,0x1e2,0x1e3,0x1e4,0x1e5,
	0x1e6,0x1e7,0x1e8,0x1e9,0x1ea,0x1eb,0x1ec,0x1ed,
	0x1ee,0x1ef,0x1f0,0x1f1,0x1f2,0x1f3,0x1f4,0x1f5,
	0x1f6,0x1f7,0x1f8,0x1f9,0x1fa,0x1fb,0x1fc,0x1fd,
	0x1fe,0x1ff,0x100,0x101,0x102,0x103,0x104,0x105,
	0x106,0x107,0x108,0x109,0x10a,0x10b,0x10c,0x10d,
	0x10e,0x10f,0x110,0x111,0x112,0x113,0x114,0x115,
	0x116,0x117,0x118,0x119,0x11a,0x11b,0x11c,0x11d,
	0x11e,0x11f,0x120,0x121,0x122,0x123,0x124,0x125,
	0x126,0x127,0x128,0x129,0x12a,0x12b,0x12c,0x12d,
	0x12e,0x12f,0x130,0x131,0x132,0x133,0x134,0x135,
	0x136,0x137,0x138,0x139,0x13a,0x13b,0x13c,0x13d,
	0x13e,0x13f,0x140,0x141,0x142,0x143,0x144,0x145,
	0x146,0x147,0x148,0x149,0x14a,0x14b,0x14c,0x14d,
	0x14e,0x14f,0x150,0x151,0x152,0x153,0x154,0x155,
	0x156,0x157,0x158,0x159,0x15a,0x15b,0x15c,0x15d,
	0x15e,0x15f,0x160,0x161,0x162,0x163,0x164,0x165,
	/* sub/sbc results */
	0x000,0x001,0x002,0x003,0x004,0x005,0x006,0x007,
	0x008,0x009,0x00a,0x00b,0x00c,0x00d,0x00e,0x00f,
	0x010,0x011,0x012,0x013,0x014,0x015,0x016,0x017,
	0x018,0x019,0x01a,0x01b,0x01c,0x01d,0x01e,0x01f,
	0x020,0x021,0x022,0x023,0x024,0x025,0x026,0x027,
	0x028,0x029,0x02a,0x02b,0x02c,0x02d,0x02e,0x02f,
	0x030,0x031,0x032,0x033,0x034,0x035,0x036,0x037,
	0x038,0x039,0x03a,0x03b,0x03c,0x03d,0x03e,0x03f,
	0x040,0x041,0x042,0x043,0x044,0x045,0x046,0x047,
	0x048,0x049,0x04a,0x04b,0x04c,0x04d,0x04e,0x04f,
	0x050,0x051,0x052,0x053,0x054,0x055,0x056,0x057,
	0x058,0x059,0x05a,0x05b,0x05c,0x05d,0x05e,0x05f,
	0x060,0x061,0x062,0x063,0x064,0x065,0x066,0x067,
	0x068,0x069,0x06a,0x06b,0x06c,0x06d,0x06e,0x06f,
	0x070,0x071,0x072,0x073,0x074,0x075,0x076,0x077,
	0x078,0x079,0x07a,0x07b,0x07c,0x07d,0x07e,0x07f,
	0x080,0x081,0x082,0x083,0x084,0x085,0x086,0x087,
	0x088,0x089,0x08a,0x08b,0x08c,0x08d,0x08e,0x08f,
	0x090,0x091,0x092,0x093,0x094,0x095,0x096,0x097,
	0x098,0x099,0x09a,0x09b,0x09c,0x09d,0x09e,0x09f,
	0x0a0,0x0a1,0x0a2,0x0a3,0x0a4,0x0a5,0x0a6,0x0a7,
	0x0a8,0x0a9,0x0aa,0x0ab,0x0ac,0x0ad,0x0ae,0x0af,
	0x0b0,0x0b1,0x0b2,0x0b3,0x0b4,0x0b5,0x0b6,0x0b7,
	0x0b8,0x0b9,0x0ba,0x0bb,0x0bc,0x0bd,0x0be,0x0bf,
	0x0c0,0x0c1,0x0c2,0x0c3,0x0c4,0x0c5,0x0c6,0x0c7,
	0x0c8,0x0c9,0x0ca,0x0cb,0x0cc,0x0cd,0x0ce,0x0cf,
	0x0d0,0x0d1,0x0d2,0x0d3,0x0d4,0x0d5,0x0d6,0x0d7,
	0x0d8,0x0d9,0x0da,0x0db,0x0dc,0x0dd,0x0de,0x0df,
	0x0e0,0x0e1,0x0e2,0x0e3,0x0e4,0x0e5,0x0e6,0x0e7,
	0x0e8,0x0e9,0x0ea,0x0eb,0x0ec,0x0ed,0x0ee,0x0ef,
	0x0f0,0x0f1,0x0f2,0x0f3,0x0f4,0x0f5,0x0f6,0x0f7,
	0x0f8,0x0f9,0x0fa,0x0fb,0x0fc,0x0fd,0x0fe,0x0ff,
	0x1a0,0x1a1,0x1a2,0x1a3,0x1a4,0x1a5,0x1a6,0x1a7,
	0x1a8,0x1a9,0x1aa,0x1ab,0x1ac,0x1ad,0x1ae,0x1af,
	0x1b0,0x1b1,0x1b2,0x1b3,0x1b4,0x1b5,0x1b6,0x1b7,
	0x1b8,0x1b9,0x1ba,0x1bb,0x1bc,0x1bd,0x1be,0x1bf,
	0x1c0,0x1c1,0x1c2,0x1c3,0x1c4,0x1c5,0x1c6,0x1c7,
	0x1c8,0x1c9,0x1ca,0x1cb,0x1cc,0x1cd,0x1ce,0x1cf,
	0x1d0,0x1d1,0x1d2,0x1d3,0x1d4,0x1d5,0x1d6,0x1d7,
	0x1d8,0x1d9,0x1da,0x1db,0x1dc,0x1dd,0x1de,0x1df,
	0x1e0,0x1e1,0x1e2,0x1e3,0x1e4,0x1e5,0x1e6,0x1e7,
	0x1e8,0x1e9,0x1ea,0x1eb,0x1ec,0x1ed,0x1ee,0x1ef,
	0x1f0,0x1f1,0x1f2,0x1f3,0x1f4,0x1f5,0x1f6,0x1f7,
	0x1f8,0x1f9,0x1fa,0x1fb,0x1fc,0x1fd,0x1fe,0x1ff,
	0x100,0x101,0x102,0x103,0x104,0x105,0x106,0x107,
	0x108,0x109,0x10a,0x10b,0x10c,0x10d,0x10e,0x10f,
	0x110,0x111,0x112,0x113,0x114,0x115,0x116,0x117,
	0x118,0x119,0x11a,0x11b,0x11c,0x11d,0x11e,0x11f,
	0x120,0x121,0x122,0x123,0x124,0x125,0x126,0x127,
	0x128,0x129,0x12a,0x12b,0x12c,0x12d,0x12e,0x12f,
	0x130,0x131,0x132,0x133,0x134,0x135,0x136,0x137,
	0x138,0x139,0x13a,0x13b,0x13c,0x13d,0x13e,0x13f,
	0x140,0x141,0x142,0x143,0x144,0x145,0x146,0x147,
	0x148,0x149,0x14a,0x14b,0x14c,0x14d,0x14e,0x14f,
	0x150,0x151,0x152,0x153,0x154,0x155,0x156,0x157,
	0x158,0x159,0x15a,0x15b,0x15c,0x15d,0x15e,0x15f,
	0x160,0x161,0x162,0x163,0x164,0x165,0x166,0x167,
	0x168,0x169,0x16a,0x16b,0x16c,0x16d,0x16e,0x16f,
	0x170,0x171,0x172,0x173,0x174,0x175,0x176,0x177,
	0x178,0x179,0x17a,0x17b,0x17c,0x17d,0x17e,0x17f,
	0x180,0x181,0x182,0x183,0x184,0x185,0x186,0x187,
	0x188,0x189,0x18a,0x18b,0x18c,0x18d,0x18e,0x18f,
	0x190,0x191,0x192,0x193,0x194,0x195,0x196,0x197,
	0x198,0x199,0x19a,0x19b,0x19c,0x19d,0x19e,0x19f,
	0x1fa,0x1fb,0x1fc,0x1fd,0x1fe,0x1ff,0x100,0x101,
	0x102,0x103,0x104,0x105,0x106,0x107,0x108,0x109,
	0x10a,0x10b,0x10c,0x10d,0x10e,0x10f,0x110,0x111,
	0x112,0x113,0x114,0x115,0x116,0x117,0x118,0x119,
	0x11a,0x11b,0x11c,0x11d,0x11e,0x11f,0x120,0x121,
	0x122,0x123,0x124,0x125,0x126,0x127,0x128,0x129,
	0x12a,0x12b,0x12c,0x12d,0x12e,0x12f,0x130,0x131,
	0x132,0x133,0x134,0x135,0x136,0x137,0x138,0x139,
	0x13a,0x13b,0x13c,0x13d,0x13e,0x13f,0x140,0x141,
	0x142,0x143,0x144,0x145,0x146,0x147,0x148,0x149,
	0x14a,0x14b,0x14c,0x14d,0x14e,0x14f,0x150,0x151,
	0x152,0x153,0x154,0x155,0x156,0x157,0x158,0x159,
	0x15a,0x15b,0x15c,0x15d,0x15e,0x15f,0x160,0x161,
	0x162,0x163,0x164,0x165,0x166,0x167,0x168,0x169,
	0x16a,0x16b,0x16c,0x16d,0x16e,0x16f,0x170,0x171,
	0x172,0x173,0x174,0x175,0x176,0x177,0x178,0x179,
	0x17a,0x17b,0x17c,0x17d,0x17e,0x17f,0x180,0x181,
	0x182,0x183,0x184,0x185,0x186,0x187,0x188,0x189,
	0x18a,0x18b,0x18c,0x18d,0x18e,0x18f,0x190,0x191,
	0x192,0x193,0x194,0x195,0x196,0x197,0x198,0x199,
	0x19a,0x19b,0x19c,0x19d,0x19e,0x19f,0x1a0,0x1a1,
	0x1a2,0x1a3,0x1a4,0x1a5,0x1a6,0x1a7,0x1a8,0x1a9,
	0x1aa,0x1ab,0x1ac,0x1ad,0x1ae,0x1af,0x1b0,0x1b1,
	0x1b2,0x1b3,0x1b4,0x1b5,0x1b6,0x1b7,0x1b8,0x1b9,
	0x1ba,0x1bb,0x1bc,0x1bd,0x1be,0x1bf,0x1c0,0x1c1,
	0x1c2,0x1c3,0x1c4,0x1c5,0x1c6,0x1c7,0x1c8,0x1c9,
	0x1ca,0x1cb,0x1cc,0x1cd,0x1ce,0x1cf,0x1d0,0x1d1,
	0x1d2,0x1d3,0x1d4,0x1d5,0x1d6,0x1d7,0x1d8,0x1d9,
	0x1da,0x1db,0x1dc,0x1dd,0x1de,0x1df,0x1e0,0x1e1,
	0x1e2,0x1e3,0x1e4,0x1e5,0x1e6,0x1e7,0x1e8,0x1e9,
	0x1ea,0x1eb,0x1ec,0x1ed,0x1ee,0x1ef,0x1f0,0x1f1,
	0x1f2,0x1f3,0x1f4,0x1f5,0x1f6,0x1f7,0x1f8,0x1f9,
	0x19a,0x19b,0x19c,0x19d,0x19e,0x19f,0x1a0,0x1a1,
	0x1a2,0x1a3,0x1a4,0x1a5,0x1a6,0x1a7,0x1a8,0x1a9,
	0x1aa,0x1ab,0x1ac,0x1ad,0x1ae,0x1af,0x1b0,0x1b1,
	0x1b2,0x1b3,0x1b4,0x1b5,0x1b6,0x1b7,0x1b8,0x1b9,
	0x1ba,0x1bb,0x1bc,0x1bd,0x1be,0x1bf,0x1c0,0x1c1,
	0x1c2,0x1c3,0x1c4,0x1c5,0x1c6,0x1c7,0x1c8,0x1c9,
	0x1ca,0x1cb,0x1cc,0x1cd,0x1ce,0x1cf,0x1d0,0x1d1,
	0x1d2,0x1d3,0x1d4,0x1d5,0x1d6,0x1d7,0x1d8,0x1d9,
	0x1da,0x1db,0x1dc,0x1dd,0x1de,0x1df,0x1e0,0x1e1,
	0x1e2,0x1e3,0x1e4,0x1e5,0x1e6,0x1e7,0x1e8,0x1e9,
	0x1ea,0x1eb,0x1ec,0x1ed,0x1ee,0x1ef,0x1f0,0x1f1,
	0x1f2,0x1f3,0x1f4,0x1f5,0x1f6,0x1f7,0x1f8,0x1f9,
	0x1fa,0x1fb,0x1fc,0x1fd,0x1fe,0x1ff,0x100,0x101,
	0x102,0x103,0x104,0x105,0x106,0x107,0x108,0x109,
	0x10a,0x10b,0x10c,0x10d,0x10e,0x10f,0x110,0x111,
	0x112,0x113,0x114,0x115,0x116,0x117,0x118,0x119,
	0x11a,0x11b,0x11c,0x11d,0x11e,0x11f,0x120,0x121,
	0x122,0x123,0x124,0x125,0x126,0x127,0x128,0x129,
	0x12a,0x12b,0x12c,0x12d,0x12e,0x12f,0x130,0x131,
	0x132,0x133,0x134,0x135,0x136,0x137,0x138,0x139,
	0x13a,0x13b,0x13c,0x13d,0x13e,0x13f,0x140,0x141,
	0x142,0x143,0x144,0x145,0x146,0x147,0x148,0x149,
	0x14a,0x14b,0x14c,0x14d,0x14e,0x14f,0x150,0x151,
	0x152,0x153,0x154,0x155,0x156,0x157,0x158,0x159,
	0x15a,0x15b,0x15c,0x15d,0x15e,0x15f,0x160,0x161,
	0x162,0x163,0x164,0x165,0x166,0x167,0x168,0x169,
	0x16a,0x16b,0x16c,0x16d,0x16e,0x16f,0x170,0x171,
	0x172,0x173,0x174,0x175,0x176,0x177,0x178,0x179,
	0x17a,0x17b,0x17c,0x17d,0x17e,0x17f,0x180,0x181,
	0x182,0x183,0x184,0x185,0x186,0x187,0x188,0x189,
	0x18a,0x18b,0x18c,0x18d,0x18e,0x18f,0x190,0x191,
	0x192,0x193,0x194,0x195,0x196,0x197,0x198,0x199,
};

#define CHK_XXXB_ZS m_fcw |= Z8K_ZERO(result) << 6 | (uint32_t)((result) >> 7 & 1) << 5   /* CHK_XXXB_ZS if (!result) SET_Z; else if ((int8_t) result < 0) SET_S */
#define CHK_XXXW_ZS m_fcw |= Z8K_ZERO(result) << 6 | (uint32_t)((result) >> 15 & 1) << 5   /* CHK_XXXW_ZS if (!result) SET_Z; else if ((int16_t)result < 0) SET_S */
#define CHK_XXXL_ZS m_fcw |= Z8K_ZERO(result) << 6 | (uint32_t)((result) >> 31 & 1) << 5   /* CHK_XXXL_ZS if (!result) SET_Z; else if ((int32_t)result < 0) SET_S */
#define CHK_XXXQ_ZS m_fcw |= (uint32_t)(result == 0) << 6 | (uint32_t)((result) >> 63 & 1) << 5   /* CHK_XXXQ_ZS if (!result) SET_Z; else if ((int64_t)result < 0) SET_S */
#define CHK_XXXB_ZSP m_fcw |= z8000_zsp[result]
#define CHK_ADDX_C if (result < dest) SET_C
#define CHK_ADCX_C if (result < dest || (result == dest && value)) SET_C
#define CHK_SUBX_C if (result > dest) SET_C
#define CHK_SBCX_C if (result > dest || (result == dest && value)) SET_C
#define CHK_ADDB_H  if ((result & 15) < (dest & 15)) SET_H
#define CHK_ADCB_H  if ((result & 15) < (dest & 15) || ((result & 15) == (dest & 15) && (value & 15))) SET_H
#define CHK_SUBB_H  if ((result & 15) > (dest & 15)) SET_H
#define CHK_SBCB_H  if ((result & 15) > (dest & 15) || ((result & 15) == (dest & 15) && (value & 15))) SET_H
#define CHK_ADDB_V m_fcw |= (uint32_t)(((value & dest & ~result) | (~value & ~dest & result)) >> 7 & 1) << 4   /* CHK_ADDB_V if (((value & dest & ~result) | (~value & ~dest & result)) & S08) SET_V */
#define CHK_ADDW_V m_fcw |= (uint32_t)(((value & dest & ~result) | (~value & ~dest & result)) >> 15 & 1) << 4   /* CHK_ADDW_V if (((value & dest & ~result) | (~value & ~dest & result)) & S16) SET_V */
#define CHK_ADDL_V m_fcw |= (uint32_t)(((value & dest & ~result) | (~value & ~dest & result)) >> 31 & 1) << 4   /* CHK_ADDL_V if (((value & dest & ~result) | (~value & ~dest & result)) & S32) SET_V */
#define CHK_SUBB_V m_fcw |= (uint32_t)(((~value & dest & ~result) | (value & ~dest & result)) >> 7 & 1) << 4   /* CHK_SUBB_V if (((~value & dest & ~result) | (value & ~dest & result)) & S08) SET_V */
#define CHK_SUBW_V m_fcw |= (uint32_t)(((~value & dest & ~result) | (value & ~dest & result)) >> 15 & 1) << 4   /* CHK_SUBW_V if (((~value & dest & ~result) | (value & ~dest & result)) & S16) SET_V */
#define CHK_SUBL_V m_fcw |= (uint32_t)(((~value & dest & ~result) | (value & ~dest & result)) >> 31 & 1) << 4   /* CHK_SUBL_V if (((~value & dest & ~result) | (value & ~dest & result)) & S32) SET_V */
#define CHECK_PRIVILEGED_INSTR() if (!(m_fcw & F_S_N)) { m_irq_req |= Z8000_TRAP; return; }
#define CHECK_EXT_INSTR()  if (!(m_fcw & F_EPU)) { m_irq_req |= Z8000_EPU; return; }

// the helpers take the state first (NAME(...) is NAME_(S, ...))
#define CHANGE_FCW(...) CHANGE_FCW_(S, ##__VA_ARGS__)
#define make_segmented_addr(...) make_segmented_addr_(S, ##__VA_ARGS__)
#define segmented_addr(...) segmented_addr_(S, ##__VA_ARGS__)
#define addr_from_reg(...) addr_from_reg_(S, ##__VA_ARGS__)
#define addr_to_reg(...) addr_to_reg_(S, ##__VA_ARGS__)
#define add_to_addr_reg(...) add_to_addr_reg_(S, ##__VA_ARGS__)
#define sub_from_addr_reg(...) sub_from_addr_reg_(S, ##__VA_ARGS__)
#define set_pc(...) set_pc_(S, ##__VA_ARGS__)
#define RDIR_B(...) RDIR_B_(S, ##__VA_ARGS__)
#define RDIR_W(...) RDIR_W_(S, ##__VA_ARGS__)
#define RDIR_L(...) RDIR_L_(S, ##__VA_ARGS__)
#define WRIR_B(...) WRIR_B_(S, ##__VA_ARGS__)
#define WRIR_W(...) WRIR_W_(S, ##__VA_ARGS__)
#define WRIR_L(...) WRIR_L_(S, ##__VA_ARGS__)
#define RDBX_B(...) RDBX_B_(S, ##__VA_ARGS__)
#define RDBX_W(...) RDBX_W_(S, ##__VA_ARGS__)
#define RDBX_L(...) RDBX_L_(S, ##__VA_ARGS__)
#define WRBX_B(...) WRBX_B_(S, ##__VA_ARGS__)
#define WRBX_W(...) WRBX_W_(S, ##__VA_ARGS__)
#define WRBX_L(...) WRBX_L_(S, ##__VA_ARGS__)
#define PUSHW(...) PUSHW_(S, ##__VA_ARGS__)
#define POPW(...) POPW_(S, ##__VA_ARGS__)
#define PUSHL(...) PUSHL_(S, ##__VA_ARGS__)
#define POPL(...) POPL_(S, ##__VA_ARGS__)
#define ADDB(...) ADDB_(S, ##__VA_ARGS__)
#define ADDW(...) ADDW_(S, ##__VA_ARGS__)
#define ADDL(...) ADDL_(S, ##__VA_ARGS__)
#define ADCB(...) ADCB_(S, ##__VA_ARGS__)
#define ADCW(...) ADCW_(S, ##__VA_ARGS__)
#define SUBB(...) SUBB_(S, ##__VA_ARGS__)
#define SUBW(...) SUBW_(S, ##__VA_ARGS__)
#define SUBL(...) SUBL_(S, ##__VA_ARGS__)
#define SBCB(...) SBCB_(S, ##__VA_ARGS__)
#define SBCW(...) SBCW_(S, ##__VA_ARGS__)
#define ORB(...) ORB_(S, ##__VA_ARGS__)
#define ORW(...) ORW_(S, ##__VA_ARGS__)
#define ANDB(...) ANDB_(S, ##__VA_ARGS__)
#define ANDW(...) ANDW_(S, ##__VA_ARGS__)
#define XORB(...) XORB_(S, ##__VA_ARGS__)
#define XORW(...) XORW_(S, ##__VA_ARGS__)
#define CPB(...) CPB_(S, ##__VA_ARGS__)
#define CPW(...) CPW_(S, ##__VA_ARGS__)
#define CPL(...) CPL_(S, ##__VA_ARGS__)
#define COMB(...) COMB_(S, ##__VA_ARGS__)
#define COMW(...) COMW_(S, ##__VA_ARGS__)
#define NEGB(...) NEGB_(S, ##__VA_ARGS__)
#define NEGW(...) NEGW_(S, ##__VA_ARGS__)
#define TESTB(...) TESTB_(S, ##__VA_ARGS__)
#define TESTW(...) TESTW_(S, ##__VA_ARGS__)
#define TESTL(...) TESTL_(S, ##__VA_ARGS__)
#define INCB(...) INCB_(S, ##__VA_ARGS__)
#define INCW(...) INCW_(S, ##__VA_ARGS__)
#define DECB(...) DECB_(S, ##__VA_ARGS__)
#define DECW(...) DECW_(S, ##__VA_ARGS__)
#define MULTW(...) MULTW_(S, ##__VA_ARGS__)
#define MULTL(...) MULTL_(S, ##__VA_ARGS__)
#define DIVW(...) DIVW_(S, ##__VA_ARGS__)
#define DIVL(...) DIVL_(S, ##__VA_ARGS__)
#define RLB(...) RLB_(S, ##__VA_ARGS__)
#define RLW(...) RLW_(S, ##__VA_ARGS__)
#define RLCB(...) RLCB_(S, ##__VA_ARGS__)
#define RLCW(...) RLCW_(S, ##__VA_ARGS__)
#define RRB(...) RRB_(S, ##__VA_ARGS__)
#define RRW(...) RRW_(S, ##__VA_ARGS__)
#define RRCB(...) RRCB_(S, ##__VA_ARGS__)
#define RRCW(...) RRCW_(S, ##__VA_ARGS__)
#define SDAB(...) SDAB_(S, ##__VA_ARGS__)
#define SDAW(...) SDAW_(S, ##__VA_ARGS__)
#define SDAL(...) SDAL_(S, ##__VA_ARGS__)
#define SDLB(...) SDLB_(S, ##__VA_ARGS__)
#define SDLW(...) SDLW_(S, ##__VA_ARGS__)
#define SDLL(...) SDLL_(S, ##__VA_ARGS__)
#define SLAB(...) SLAB_(S, ##__VA_ARGS__)
#define SLAW(...) SLAW_(S, ##__VA_ARGS__)
#define SLAL(...) SLAL_(S, ##__VA_ARGS__)
#define SLLB(...) SLLB_(S, ##__VA_ARGS__)
#define SLLW(...) SLLW_(S, ##__VA_ARGS__)
#define SLLL(...) SLLL_(S, ##__VA_ARGS__)
#define SRAB(...) SRAB_(S, ##__VA_ARGS__)
#define SRAW(...) SRAW_(S, ##__VA_ARGS__)
#define SRAL(...) SRAL_(S, ##__VA_ARGS__)
#define SRLB(...) SRLB_(S, ##__VA_ARGS__)
#define SRLW(...) SRLW_(S, ##__VA_ARGS__)
#define SRLL(...) SRLL_(S, ##__VA_ARGS__)

static Z8K_AI void CHANGE_FCW_(z8k_st_t *S, uint16_t fcw);
static Z8K_AI uint32_t make_segmented_addr_(z8k_st_t *S, uint32_t addr);
static Z8K_AI uint32_t segmented_addr_(z8k_st_t *S, uint32_t addr);
static Z8K_AI uint32_t addr_from_reg_(z8k_st_t *S, int regno);
static Z8K_AI void addr_to_reg_(z8k_st_t *S, int regno, uint32_t addr);
static Z8K_AI void add_to_addr_reg_(z8k_st_t *S, int regno, uint16_t addend);
static Z8K_AI void sub_from_addr_reg_(z8k_st_t *S, int regno, uint16_t subtrahend);
static Z8K_AI void set_pc_(z8k_st_t *S, uint32_t addr);
static Z8K_AI uint8_t RDIR_B_(z8k_st_t *S, uint8_t reg);
static Z8K_AI uint16_t RDIR_W_(z8k_st_t *S, uint8_t reg);
static Z8K_AI uint32_t RDIR_L_(z8k_st_t *S, uint8_t reg);
static Z8K_AI void WRIR_B_(z8k_st_t *S, uint8_t reg, uint8_t value);
static Z8K_AI void WRIR_W_(z8k_st_t *S, uint8_t reg, uint16_t value);
static Z8K_AI void WRIR_L_(z8k_st_t *S, uint8_t reg, uint32_t value);
static Z8K_AI uint8_t RDBX_B_(z8k_st_t *S, uint8_t reg, uint16_t idx);
static Z8K_AI uint16_t RDBX_W_(z8k_st_t *S, uint8_t reg, uint16_t idx);
static Z8K_AI uint32_t RDBX_L_(z8k_st_t *S, uint8_t reg, uint16_t idx);
static Z8K_AI void WRBX_B_(z8k_st_t *S, uint8_t reg, uint16_t idx, uint8_t value);
static Z8K_AI void WRBX_W_(z8k_st_t *S, uint8_t reg, uint16_t idx, uint16_t value);
static Z8K_AI void WRBX_L_(z8k_st_t *S, uint8_t reg, uint16_t idx, uint32_t value);
static Z8K_AI void PUSHW_(z8k_st_t *S, uint8_t dst, uint16_t value);
static Z8K_AI uint16_t POPW_(z8k_st_t *S, uint8_t src);
static Z8K_AI void PUSHL_(z8k_st_t *S, uint8_t dst, uint32_t value);
static Z8K_AI uint32_t POPL_(z8k_st_t *S, uint8_t src);
static Z8K_AI uint8_t ADDB_(z8k_st_t *S, uint8_t dest, uint8_t value);
static Z8K_AI uint16_t ADDW_(z8k_st_t *S, uint16_t dest, uint16_t value);
static Z8K_AI uint32_t ADDL_(z8k_st_t *S, uint32_t dest, uint32_t value);
static Z8K_AI uint8_t ADCB_(z8k_st_t *S, uint8_t dest, uint8_t value);
static Z8K_AI uint16_t ADCW_(z8k_st_t *S, uint16_t dest, uint16_t value);
static Z8K_AI uint8_t SUBB_(z8k_st_t *S, uint8_t dest, uint8_t value);
static Z8K_AI uint16_t SUBW_(z8k_st_t *S, uint16_t dest, uint16_t value);
static Z8K_AI uint32_t SUBL_(z8k_st_t *S, uint32_t dest, uint32_t value);
static Z8K_AI uint8_t SBCB_(z8k_st_t *S, uint8_t dest, uint8_t value);
static Z8K_AI uint16_t SBCW_(z8k_st_t *S, uint16_t dest, uint16_t value);
static Z8K_AI uint8_t ORB_(z8k_st_t *S, uint8_t dest, uint8_t value);
static Z8K_AI uint16_t ORW_(z8k_st_t *S, uint16_t dest, uint16_t value);
static Z8K_AI uint8_t ANDB_(z8k_st_t *S, uint8_t dest, uint8_t value);
static Z8K_AI uint16_t ANDW_(z8k_st_t *S, uint16_t dest, uint16_t value);
static Z8K_AI uint8_t XORB_(z8k_st_t *S, uint8_t dest, uint8_t value);
static Z8K_AI uint16_t XORW_(z8k_st_t *S, uint16_t dest, uint16_t value);
static Z8K_AI void CPB_(z8k_st_t *S, uint8_t dest, uint8_t value);
static Z8K_AI void CPW_(z8k_st_t *S, uint16_t dest, uint16_t value);
static Z8K_AI void CPL_(z8k_st_t *S, uint32_t dest, uint32_t value);
static Z8K_AI uint8_t COMB_(z8k_st_t *S, uint8_t dest);
static Z8K_AI uint16_t COMW_(z8k_st_t *S, uint16_t dest);
static Z8K_AI uint8_t NEGB_(z8k_st_t *S, uint8_t dest);
static Z8K_AI uint16_t NEGW_(z8k_st_t *S, uint16_t dest);
static Z8K_AI void TESTB_(z8k_st_t *S, uint8_t result);
static Z8K_AI void TESTW_(z8k_st_t *S, uint16_t dest);
static Z8K_AI void TESTL_(z8k_st_t *S, uint32_t dest);
static Z8K_AI uint8_t INCB_(z8k_st_t *S, uint8_t dest, uint8_t value);
static Z8K_AI uint16_t INCW_(z8k_st_t *S, uint16_t dest, uint16_t value);
static Z8K_AI uint8_t DECB_(z8k_st_t *S, uint8_t dest, uint8_t value);
static Z8K_AI uint16_t DECW_(z8k_st_t *S, uint16_t dest, uint16_t value);
static Z8K_AI uint32_t MULTW_(z8k_st_t *S, uint16_t dest, uint16_t value);
static Z8K_AI uint64_t MULTL_(z8k_st_t *S, uint32_t dest, uint32_t value);
static Z8K_AI uint32_t DIVW_(z8k_st_t *S, uint32_t dest, uint16_t value);
static Z8K_AI uint64_t DIVL_(z8k_st_t *S, uint64_t dest, uint32_t value);
static Z8K_AI uint8_t RLB_(z8k_st_t *S, uint8_t dest, uint8_t twice);
static Z8K_AI uint16_t RLW_(z8k_st_t *S, uint16_t dest, uint8_t twice);
static Z8K_AI uint8_t RLCB_(z8k_st_t *S, uint8_t dest, uint8_t twice);
static Z8K_AI uint16_t RLCW_(z8k_st_t *S, uint16_t dest, uint8_t twice);
static Z8K_AI uint8_t RRB_(z8k_st_t *S, uint8_t dest, uint8_t twice);
static Z8K_AI uint16_t RRW_(z8k_st_t *S, uint16_t dest, uint8_t twice);
static Z8K_AI uint8_t RRCB_(z8k_st_t *S, uint8_t dest, uint8_t twice);
static Z8K_AI uint16_t RRCW_(z8k_st_t *S, uint16_t dest, uint8_t twice);
static Z8K_AI uint8_t SDAB_(z8k_st_t *S, uint8_t dest, int8_t count);
static Z8K_AI uint16_t SDAW_(z8k_st_t *S, uint16_t dest, int8_t count);
static Z8K_AI uint32_t SDAL_(z8k_st_t *S, uint32_t dest, int8_t count);
static Z8K_AI uint8_t SDLB_(z8k_st_t *S, uint8_t dest, int8_t count);
static Z8K_AI uint16_t SDLW_(z8k_st_t *S, uint16_t dest, int8_t count);
static Z8K_AI uint32_t SDLL_(z8k_st_t *S, uint32_t dest, int8_t count);
static Z8K_AI uint8_t SLAB_(z8k_st_t *S, uint8_t dest, uint8_t count);
static Z8K_AI uint16_t SLAW_(z8k_st_t *S, uint16_t dest, uint8_t count);
static Z8K_AI uint32_t SLAL_(z8k_st_t *S, uint32_t dest, uint8_t count);
static Z8K_AI uint8_t SLLB_(z8k_st_t *S, uint8_t dest, uint8_t count);
static Z8K_AI uint16_t SLLW_(z8k_st_t *S, uint16_t dest, uint8_t count);
static Z8K_AI uint32_t SLLL_(z8k_st_t *S, uint32_t dest, uint8_t count);
static Z8K_AI uint8_t SRAB_(z8k_st_t *S, uint8_t dest, uint8_t count);
static Z8K_AI uint16_t SRAW_(z8k_st_t *S, uint16_t dest, uint8_t count);
static Z8K_AI uint32_t SRAL_(z8k_st_t *S, uint32_t dest, uint8_t count);
static Z8K_AI uint8_t SRLB_(z8k_st_t *S, uint8_t dest, uint8_t count);
static Z8K_AI uint16_t SRLW_(z8k_st_t *S, uint16_t dest, uint8_t count);
static Z8K_AI uint32_t SRLL_(z8k_st_t *S, uint32_t dest, uint8_t count);
static Z8K_AI void zinvalid(z8k_st_t *S);
static Z8K_AI void Z00_0000_dddd_imm8(z8k_st_t *S);
static Z8K_AI void Z00_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z01_0000_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z01_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z02_0000_dddd_imm8(z8k_st_t *S);
static Z8K_AI void Z02_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z03_0000_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z03_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z04_0000_dddd_imm8(z8k_st_t *S);
static Z8K_AI void Z04_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z05_0000_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z05_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z06_0000_dddd_imm8(z8k_st_t *S);
static Z8K_AI void Z06_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z07_0000_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z07_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z08_0000_dddd_imm8(z8k_st_t *S);
static Z8K_AI void Z08_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z09_0000_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z09_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z0A_0000_dddd_imm8(z8k_st_t *S);
static Z8K_AI void Z0A_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z0B_0000_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z0B_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z0C_ddN0_0000(z8k_st_t *S);
static Z8K_AI void Z0C_ddN0_0001_imm8(z8k_st_t *S);
static Z8K_AI void Z0C_ddN0_0010(z8k_st_t *S);
static Z8K_AI void Z0C_ddN0_0100(z8k_st_t *S);
static Z8K_AI void Z0C_ddN0_0101_imm8(z8k_st_t *S);
static Z8K_AI void Z0C_ddN0_0110(z8k_st_t *S);
static Z8K_AI void Z0C_ddN0_1000(z8k_st_t *S);
static Z8K_AI void Z0D_ddN0_0000(z8k_st_t *S);
static Z8K_AI void Z0D_ddN0_0001_imm16(z8k_st_t *S);
static Z8K_AI void Z0D_ddN0_0010(z8k_st_t *S);
static Z8K_AI void Z0D_ddN0_0100(z8k_st_t *S);
static Z8K_AI void Z0D_ddN0_0101_imm16(z8k_st_t *S);
static Z8K_AI void Z0D_ddN0_0110(z8k_st_t *S);
static Z8K_AI void Z0D_ddN0_1000(z8k_st_t *S);
static Z8K_AI void Z0D_ddN0_1001_imm16(z8k_st_t *S);
static Z8K_AI void Z0E_imm8(z8k_st_t *S);
static Z8K_AI void Z0F_imm8(z8k_st_t *S);
static Z8K_AI void Z10_0000_dddd_imm32(z8k_st_t *S);
static Z8K_AI void Z10_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z11_ddN0_ssN0(z8k_st_t *S);
static Z8K_AI void Z12_0000_dddd_imm32(z8k_st_t *S);
static Z8K_AI void Z12_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z13_ddN0_ssN0(z8k_st_t *S);
static Z8K_AI void Z14_0000_dddd_imm32(z8k_st_t *S);
static Z8K_AI void Z14_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z15_ssN0_ddN0(z8k_st_t *S);
static Z8K_AI void Z16_0000_dddd_imm32(z8k_st_t *S);
static Z8K_AI void Z16_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z17_ssN0_ddN0(z8k_st_t *S);
static Z8K_AI void Z18_00N0_dddd_imm32(z8k_st_t *S);
static Z8K_AI void Z18_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z19_0000_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z19_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z1A_0000_dddd_imm32(z8k_st_t *S);
static Z8K_AI void Z1A_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z1B_0000_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z1B_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z1C_ddN0_1000(z8k_st_t *S);
static Z8K_AI void Z1C_ddN0_1001_0000_ssss_0000_nmin1(z8k_st_t *S);
static Z8K_AI void Z1C_ssN0_0001_0000_dddd_0000_nmin1(z8k_st_t *S);
static Z8K_AI void Z1D_ddN0_ssss(z8k_st_t *S);
static Z8K_AI void Z1E_ddN0_cccc(z8k_st_t *S);
static Z8K_AI void Z1F_ddN0_0000(z8k_st_t *S);
static Z8K_AI void Z20_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z21_0000_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z21_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z22_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S);
static Z8K_AI void Z22_ddN0_imm4(z8k_st_t *S);
static Z8K_AI void Z23_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S);
static Z8K_AI void Z23_ddN0_imm4(z8k_st_t *S);
static Z8K_AI void Z24_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S);
static Z8K_AI void Z24_ddN0_imm4(z8k_st_t *S);
static Z8K_AI void Z25_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S);
static Z8K_AI void Z25_ddN0_imm4(z8k_st_t *S);
static Z8K_AI void Z26_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S);
static Z8K_AI void Z26_ddN0_imm4(z8k_st_t *S);
static Z8K_AI void Z27_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S);
static Z8K_AI void Z27_ddN0_imm4(z8k_st_t *S);
static Z8K_AI void Z28_ddN0_imm4m1(z8k_st_t *S);
static Z8K_AI void Z29_ddN0_imm4m1(z8k_st_t *S);
static Z8K_AI void Z2A_ddN0_imm4m1(z8k_st_t *S);
static Z8K_AI void Z2B_ddN0_imm4m1(z8k_st_t *S);
static Z8K_AI void Z2C_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z2D_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z2E_ddN0_ssss(z8k_st_t *S);
static Z8K_AI void Z2F_ddN0_ssss(z8k_st_t *S);
static Z8K_AI void Z30_0000_dddd_dsp16(z8k_st_t *S);
static Z8K_AI void Z30_ssN0_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z31_0000_dddd_dsp16(z8k_st_t *S);
static Z8K_AI void Z31_ssN0_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z32_0000_ssss_dsp16(z8k_st_t *S);
static Z8K_AI void Z32_ddN0_ssss_imm16(z8k_st_t *S);
static Z8K_AI void Z33_0000_ssss_dsp16(z8k_st_t *S);
static Z8K_AI void Z33_ddN0_ssss_imm16(z8k_st_t *S);
static Z8K_AI void Z34_0000_dddd_dsp16(z8k_st_t *S);
static Z8K_AI void Z34_ssN0_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z35_0000_dddd_dsp16(z8k_st_t *S);
static Z8K_AI void Z35_ssN0_dddd_imm16(z8k_st_t *S);
static Z8K_AI void Z36_0000_0000(z8k_st_t *S);
static Z8K_AI void Z36_imm8(z8k_st_t *S);
static Z8K_AI void Z37_0000_ssss_dsp16(z8k_st_t *S);
static Z8K_AI void Z37_ddN0_ssss_imm16(z8k_st_t *S);
static Z8K_AI void Z38_imm8(z8k_st_t *S);
static Z8K_AI void Z39_ssN0_0000(z8k_st_t *S);
static Z8K_AI void Z3A_ssss_0000_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3A_ssss_0001_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3A_ssss_0010_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3A_ssss_0011_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3A_dddd_0100_imm16(z8k_st_t *S);
static Z8K_AI void Z3A_dddd_0101_imm16(z8k_st_t *S);
static Z8K_AI void Z3A_ssss_0110_imm16(z8k_st_t *S);
static Z8K_AI void Z3A_ssss_0111_imm16(z8k_st_t *S);
static Z8K_AI void Z3A_ssss_1000_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3A_ssss_1001_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3A_ssss_1010_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3A_ssss_1011_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3B_ssss_0000_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3B_ssss_0001_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3B_ssss_0010_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3B_ssss_0011_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3B_dddd_0100_imm16(z8k_st_t *S);
static Z8K_AI void Z3B_dddd_0101_imm16(z8k_st_t *S);
static Z8K_AI void Z3B_ssss_0110_imm16(z8k_st_t *S);
static Z8K_AI void Z3B_ssss_0111_imm16(z8k_st_t *S);
static Z8K_AI void Z3B_ssss_1000_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3B_ssss_1001_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3B_ssss_1010_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3B_ssss_1011_0000_aaaa_dddd_x000(z8k_st_t *S);
static Z8K_AI void Z3C_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z3D_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z3E_dddd_ssss(z8k_st_t *S);
static Z8K_AI void Z3F_dddd_ssss(z8k_st_t *S);
static Z8K_AI void Z40_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z40_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z41_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z41_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z42_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z42_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z43_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z43_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z44_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z44_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z45_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z45_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z46_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z46_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z47_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z47_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z48_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z48_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z49_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z49_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z4A_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z4A_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z4B_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z4B_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z4C_0000_0000_addr(z8k_st_t *S);
static Z8K_AI void Z4C_0000_0001_addr_imm8(z8k_st_t *S);
static Z8K_AI void Z4C_0000_0010_addr(z8k_st_t *S);
static Z8K_AI void Z4C_0000_0100_addr(z8k_st_t *S);
static Z8K_AI void Z4C_0000_0101_addr_imm8(z8k_st_t *S);
static Z8K_AI void Z4C_0000_0110_addr(z8k_st_t *S);
static Z8K_AI void Z4C_0000_1000_addr(z8k_st_t *S);
static Z8K_AI void Z4C_ddN0_0000_addr(z8k_st_t *S);
static Z8K_AI void Z4C_ddN0_0001_addr_imm8(z8k_st_t *S);
static Z8K_AI void Z4C_ddN0_0010_addr(z8k_st_t *S);
static Z8K_AI void Z4C_ddN0_0100_addr(z8k_st_t *S);
static Z8K_AI void Z4C_ddN0_0101_addr_imm8(z8k_st_t *S);
static Z8K_AI void Z4C_ddN0_0110_addr(z8k_st_t *S);
static Z8K_AI void Z4C_ddN0_1000_addr(z8k_st_t *S);
static Z8K_AI void Z4D_0000_0000_addr(z8k_st_t *S);
static Z8K_AI void Z4D_0000_0001_addr_imm16(z8k_st_t *S);
static Z8K_AI void Z4D_0000_0010_addr(z8k_st_t *S);
static Z8K_AI void Z4D_0000_0100_addr(z8k_st_t *S);
static Z8K_AI void Z4D_0000_0101_addr_imm16(z8k_st_t *S);
static Z8K_AI void Z4D_0000_0110_addr(z8k_st_t *S);
static Z8K_AI void Z4D_0000_1000_addr(z8k_st_t *S);
static Z8K_AI void Z4D_ddN0_0000_addr(z8k_st_t *S);
static Z8K_AI void Z4D_ddN0_0001_addr_imm16(z8k_st_t *S);
static Z8K_AI void Z4D_ddN0_0010_addr(z8k_st_t *S);
static Z8K_AI void Z4D_ddN0_0100_addr(z8k_st_t *S);
static Z8K_AI void Z4D_ddN0_0101_addr_imm16(z8k_st_t *S);
static Z8K_AI void Z4D_ddN0_0110_addr(z8k_st_t *S);
static Z8K_AI void Z4D_ddN0_1000_addr(z8k_st_t *S);
static Z8K_AI void Z4E_ddN0_ssN0_addr(z8k_st_t *S);
static Z8K_AI void Z50_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z50_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z51_ddN0_0000_addr(z8k_st_t *S);
static Z8K_AI void Z51_ddN0_ssN0_addr(z8k_st_t *S);
static Z8K_AI void Z52_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z52_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z53_ddN0_0000_addr(z8k_st_t *S);
static Z8K_AI void Z53_ddN0_ssN0_addr(z8k_st_t *S);
static Z8K_AI void Z54_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z54_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z55_ssN0_0000_addr(z8k_st_t *S);
static Z8K_AI void Z55_ssN0_ddN0_addr(z8k_st_t *S);
static Z8K_AI void Z56_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z56_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z57_ssN0_0000_addr(z8k_st_t *S);
static Z8K_AI void Z57_ssN0_ddN0_addr(z8k_st_t *S);
static Z8K_AI void Z58_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z58_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z59_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z59_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z5A_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z5A_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z5B_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z5B_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z5C_0000_0001_0000_dddd_0000_nmin1_addr(z8k_st_t *S);
static Z8K_AI void Z5C_0000_1000_addr(z8k_st_t *S);
static Z8K_AI void Z5C_0000_1001_0000_ssss_0000_nmin1_addr(z8k_st_t *S);
static Z8K_AI void Z5C_ddN0_1000_addr(z8k_st_t *S);
static Z8K_AI void Z5C_ddN0_1001_0000_ssN0_0000_nmin1_addr(z8k_st_t *S);
static Z8K_AI void Z5C_ssN0_0001_0000_dddd_0000_nmin1_addr(z8k_st_t *S);
static Z8K_AI void Z5D_0000_ssss_addr(z8k_st_t *S);
static Z8K_AI void Z5D_ddN0_ssss_addr(z8k_st_t *S);
static Z8K_AI void Z5E_0000_cccc_addr(z8k_st_t *S);
static Z8K_AI void Z5E_ddN0_cccc_addr(z8k_st_t *S);
static Z8K_AI void Z5F_0000_0000_addr(z8k_st_t *S);
static Z8K_AI void Z5F_ddN0_0000_addr(z8k_st_t *S);
static Z8K_AI void Z60_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z60_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z61_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z61_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z62_0000_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z62_ddN0_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z63_0000_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z63_ddN0_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z64_0000_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z64_ddN0_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z65_0000_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z65_ddN0_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z66_0000_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z66_ddN0_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z67_0000_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z67_ddN0_imm4_addr(z8k_st_t *S);
static Z8K_AI void Z68_0000_imm4m1_addr(z8k_st_t *S);
static Z8K_AI void Z68_ddN0_imm4m1_addr(z8k_st_t *S);
static Z8K_AI void Z69_0000_imm4m1_addr(z8k_st_t *S);
static Z8K_AI void Z69_ddN0_imm4m1_addr(z8k_st_t *S);
static Z8K_AI void Z6A_0000_imm4m1_addr(z8k_st_t *S);
static Z8K_AI void Z6A_ddN0_imm4m1_addr(z8k_st_t *S);
static Z8K_AI void Z6B_0000_imm4m1_addr(z8k_st_t *S);
static Z8K_AI void Z6B_ddN0_imm4m1_addr(z8k_st_t *S);
static Z8K_AI void Z6C_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z6C_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z6D_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z6D_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z6E_0000_ssss_addr(z8k_st_t *S);
static Z8K_AI void Z6E_ddN0_ssss_addr(z8k_st_t *S);
static Z8K_AI void Z6F_0000_ssss_addr(z8k_st_t *S);
static Z8K_AI void Z6F_ddN0_ssss_addr(z8k_st_t *S);
static Z8K_AI void Z70_ssN0_dddd_0000_xxxx_0000_0000(z8k_st_t *S);
static Z8K_AI void Z71_ssN0_dddd_0000_xxxx_0000_0000(z8k_st_t *S);
static Z8K_AI void Z72_ddN0_ssss_0000_xxxx_0000_0000(z8k_st_t *S);
static Z8K_AI void Z73_ddN0_ssss_0000_xxxx_0000_0000(z8k_st_t *S);
static Z8K_AI void Z74_ssN0_dddd_0000_xxxx_0000_0000(z8k_st_t *S);
static Z8K_AI void Z75_ssN0_dddd_0000_xxxx_0000_0000(z8k_st_t *S);
static Z8K_AI void Z76_0000_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z76_ssN0_dddd_addr(z8k_st_t *S);
static Z8K_AI void Z77_ddN0_ssss_0000_xxxx_0000_0000(z8k_st_t *S);
static Z8K_AI void Z78_imm8(z8k_st_t *S);
static Z8K_AI void Z79_0000_0000_addr(z8k_st_t *S);
static Z8K_AI void Z79_ssN0_0000_addr(z8k_st_t *S);
static Z8K_AI void Z7A_0000_0000(z8k_st_t *S);
static Z8K_AI void Z7B_0000_0000(z8k_st_t *S);
static Z8K_AI void Z7B_0000_1000(z8k_st_t *S);
static Z8K_AI void Z7B_0000_1001(z8k_st_t *S);
static Z8K_AI void Z7B_0000_1010(z8k_st_t *S);
static Z8K_AI void Z7B_dddd_1101(z8k_st_t *S);
static Z8K_AI void Z7C_0000_00ii(z8k_st_t *S);
static Z8K_AI void Z7C_0000_01ii(z8k_st_t *S);
static Z8K_AI void Z7D_dddd_0ccc(z8k_st_t *S);
static Z8K_AI void Z7D_ssss_1ccc(z8k_st_t *S);
static Z8K_AI void Z7E_imm8(z8k_st_t *S);
static Z8K_AI void Z7F_imm8(z8k_st_t *S);
static Z8K_AI void Z80_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z81_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z82_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z83_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z84_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z85_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z86_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z87_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z88_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z89_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z8A_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z8B_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z8C_dddd_0000(z8k_st_t *S);
static Z8K_AI void Z8C_dddd_0010(z8k_st_t *S);
static Z8K_AI void Z8C_dddd_0100(z8k_st_t *S);
static Z8K_AI void Z8C_dddd_0110(z8k_st_t *S);
static Z8K_AI void Z8C_dddd_0001(z8k_st_t *S);
static Z8K_AI void Z8C_dddd_1000(z8k_st_t *S);
static Z8K_AI void Z8C_dddd_1001(z8k_st_t *S);
static Z8K_AI void Z8D_0000_0111(z8k_st_t *S);
static Z8K_AI void Z8D_dddd_0000(z8k_st_t *S);
static Z8K_AI void Z8D_dddd_0010(z8k_st_t *S);
static Z8K_AI void Z8D_dddd_0100(z8k_st_t *S);
static Z8K_AI void Z8D_dddd_0110(z8k_st_t *S);
static Z8K_AI void Z8D_dddd_1000(z8k_st_t *S);
static Z8K_AI void Z8D_imm4_0001(z8k_st_t *S);
static Z8K_AI void Z8D_imm4_0011(z8k_st_t *S);
static Z8K_AI void Z8D_imm4_0101(z8k_st_t *S);
static Z8K_AI void Z8E_imm8(z8k_st_t *S);
static Z8K_AI void Z8F_imm8(z8k_st_t *S);
static Z8K_AI void Z90_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z91_ddN0_ssss(z8k_st_t *S);
static Z8K_AI void Z92_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z93_ddN0_ssss(z8k_st_t *S);
static Z8K_AI void Z94_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z95_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z96_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z97_ssN0_dddd(z8k_st_t *S);
static Z8K_AI void Z98_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z99_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z9A_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z9B_ssss_dddd(z8k_st_t *S);
static Z8K_AI void Z9C_dddd_1000(z8k_st_t *S);
static Z8K_AI void Z9D_imm8(z8k_st_t *S);
static Z8K_AI void Z9E_0000_cccc(z8k_st_t *S);
static Z8K_AI void Z9F_imm8(z8k_st_t *S);
static Z8K_AI void ZA0_ssss_dddd(z8k_st_t *S);
static Z8K_AI void ZA1_ssss_dddd(z8k_st_t *S);
static Z8K_AI void ZA2_dddd_imm4(z8k_st_t *S);
static Z8K_AI void ZA3_dddd_imm4(z8k_st_t *S);
static Z8K_AI void ZA4_dddd_imm4(z8k_st_t *S);
static Z8K_AI void ZA5_dddd_imm4(z8k_st_t *S);
static Z8K_AI void ZA6_dddd_imm4(z8k_st_t *S);
static Z8K_AI void ZA7_dddd_imm4(z8k_st_t *S);
static Z8K_AI void ZA8_dddd_imm4m1(z8k_st_t *S);
static Z8K_AI void ZA9_dddd_imm4m1(z8k_st_t *S);
static Z8K_AI void ZAA_dddd_imm4m1(z8k_st_t *S);
static Z8K_AI void ZAB_dddd_imm4m1(z8k_st_t *S);
static Z8K_AI void ZAC_ssss_dddd(z8k_st_t *S);
static Z8K_AI void ZAD_ssss_dddd(z8k_st_t *S);
static Z8K_AI void ZAE_dddd_cccc(z8k_st_t *S);
static Z8K_AI void ZAF_dddd_cccc(z8k_st_t *S);
static Z8K_AI void ZB0_dddd_0000(z8k_st_t *S);
static Z8K_AI void ZB1_dddd_0000(z8k_st_t *S);
static Z8K_AI void ZB1_dddd_0111(z8k_st_t *S);
static Z8K_AI void ZB1_dddd_1010(z8k_st_t *S);
static Z8K_AI void ZB2_dddd_0001_imm8(z8k_st_t *S);
static Z8K_AI void ZB2_dddd_0011_0000_ssss_0000_0000(z8k_st_t *S);
static Z8K_AI void ZB2_dddd_00I0(z8k_st_t *S);
static Z8K_AI void ZB2_dddd_01I0(z8k_st_t *S);
static Z8K_AI void ZB2_dddd_1001_imm8(z8k_st_t *S);
static Z8K_AI void ZB2_dddd_1011_0000_ssss_0000_0000(z8k_st_t *S);
static Z8K_AI void ZB2_dddd_10I0(z8k_st_t *S);
static Z8K_AI void ZB2_dddd_11I0(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_0001_imm8(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_0011_0000_ssss_0000_0000(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_00I0(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_0101_imm8(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_0111_0000_ssss_0000_0000(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_01I0(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_1001_imm8(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_1011_0000_ssss_0000_0000(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_10I0(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_1101_imm8(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_1111_0000_ssss_0000_0000(z8k_st_t *S);
static Z8K_AI void ZB3_dddd_11I0(z8k_st_t *S);
static Z8K_AI void ZB4_ssss_dddd(z8k_st_t *S);
static Z8K_AI void ZB5_ssss_dddd(z8k_st_t *S);
static Z8K_AI void ZB6_ssss_dddd(z8k_st_t *S);
static Z8K_AI void ZB7_ssss_dddd(z8k_st_t *S);
static Z8K_AI void ZB8_ddN0_0010_0000_rrrr_ssN0_0000(z8k_st_t *S);
static Z8K_AI void ZB8_ddN0_0110_0000_rrrr_ssN0_1110(z8k_st_t *S);
static Z8K_AI void ZB8_ddN0_1010_0000_rrrr_ssN0_0000(z8k_st_t *S);
static Z8K_AI void ZB8_ddN0_1110_0000_rrrr_ssN0_1110(z8k_st_t *S);
static Z8K_AI void ZB8_ddN0_0000_0000_rrrr_ssN0_0000(z8k_st_t *S);
static Z8K_AI void ZB8_ddN0_0100_0000_rrrr_ssN0_0000(z8k_st_t *S);
static Z8K_AI void ZB8_ddN0_1000_0000_rrrr_ssN0_0000(z8k_st_t *S);
static Z8K_AI void ZB8_ddN0_1100_0000_rrrr_ssN0_0000(z8k_st_t *S);
static Z8K_AI void ZB9_imm8(z8k_st_t *S);
static Z8K_AI void ZBA_ssN0_0000_0000_rrrr_dddd_cccc(z8k_st_t *S);
static Z8K_AI void ZBA_ssN0_0001_0000_rrrr_ddN0_x000(z8k_st_t *S);
static Z8K_AI void ZBA_ssN0_0010_0000_rrrr_ddN0_cccc(z8k_st_t *S);
static Z8K_AI void ZBA_ssN0_0100_0000_rrrr_dddd_cccc(z8k_st_t *S);
static Z8K_AI void ZBA_ssN0_0110_0000_rrrr_ddN0_cccc(z8k_st_t *S);
static Z8K_AI void ZBA_ssN0_1000_0000_rrrr_dddd_cccc(z8k_st_t *S);
static Z8K_AI void ZBA_ssN0_1001_0000_rrrr_ddN0_x000(z8k_st_t *S);
static Z8K_AI void ZBA_ssN0_1010_0000_rrrr_ddN0_cccc(z8k_st_t *S);
static Z8K_AI void ZBA_ssN0_1100_0000_rrrr_dddd_cccc(z8k_st_t *S);
static Z8K_AI void ZBA_ssN0_1110_0000_rrrr_ddN0_cccc(z8k_st_t *S);
static Z8K_AI void ZBB_ssN0_0000_0000_rrrr_dddd_cccc(z8k_st_t *S);
static Z8K_AI void ZBB_ssN0_0001_0000_rrrr_ddN0_x000(z8k_st_t *S);
static Z8K_AI void ZBB_ssN0_0010_0000_rrrr_ddN0_cccc(z8k_st_t *S);
static Z8K_AI void ZBB_ssN0_0100_0000_rrrr_dddd_cccc(z8k_st_t *S);
static Z8K_AI void ZBB_ssN0_0110_0000_rrrr_ddN0_cccc(z8k_st_t *S);
static Z8K_AI void ZBB_ssN0_1000_0000_rrrr_dddd_cccc(z8k_st_t *S);
static Z8K_AI void ZBB_ssN0_1001_0000_rrrr_ddN0_x000(z8k_st_t *S);
static Z8K_AI void ZBB_ssN0_1010_0000_rrrr_ddN0_cccc(z8k_st_t *S);
static Z8K_AI void ZBB_ssN0_1100_0000_rrrr_dddd_cccc(z8k_st_t *S);
static Z8K_AI void ZBB_ssN0_1110_0000_rrrr_ddN0_cccc(z8k_st_t *S);
static Z8K_AI void ZBC_aaaa_bbbb(z8k_st_t *S);
static Z8K_AI void ZBD_dddd_imm4(z8k_st_t *S);
static Z8K_AI void ZBE_aaaa_bbbb(z8k_st_t *S);
static Z8K_AI void ZBF_imm8(z8k_st_t *S);
static Z8K_AI void Z20_0000_dddd_imm8(z8k_st_t *S);
static Z8K_AI void ZC_dddd_imm8(z8k_st_t *S);
static Z8K_AI void ZD_dsp12(z8k_st_t *S);
static Z8K_AI void ZE_cccc_dsp8(z8k_st_t *S);
static Z8K_AI void ZF_dddd_0dsp7(z8k_st_t *S);
static Z8K_AI void ZF_dddd_1dsp7(z8k_st_t *S);

static Z8K_AI void CHANGE_FCW_(z8k_st_t *S, uint16_t fcw)
{
	uint16_t tmp;
	if ((fcw ^ m_fcw) & F_S_N)            /* system/user mode change? */
	{
		tmp = RW(15);
		RW(15) = m_nspoff;
		m_nspoff = tmp;
	}

	fcw &= ~F_SEG;  /* never set segmented mode bit on Z8002 */

	if (!(m_fcw & F_NVIE) && (fcw & F_NVIE) && (m_irq_state[0] != CLEAR_LINE))
	{
		m_irq_req |= Z8000_NVI;
	}
	if (!(m_fcw & F_VIE) && (fcw & F_VIE) && (m_irq_state[1] != CLEAR_LINE))
	{
		m_irq_req |= Z8000_VI;
	}
	m_fcw = fcw;  /* set new m_fcw */
}

static Z8K_AI uint32_t make_segmented_addr_(z8k_st_t *S, uint32_t addr)
{
	return ((addr & 0x007f0000) << 8) | 0x80000000 | (addr & 0xffff);
}

static Z8K_AI uint32_t segmented_addr_(z8k_st_t *S, uint32_t addr)
{
	return ((addr & 0x7f000000) >> 8) | (addr & 0xffff);
}

static Z8K_AI uint32_t addr_from_reg_(z8k_st_t *S, int regno)
{
	if (get_segmented_mode())
		return segmented_addr(RL(regno));
	else
		return RW(regno);
}

static Z8K_AI void addr_to_reg_(z8k_st_t *S, int regno, uint32_t addr)
{
	if (get_segmented_mode()) {
		uint32_t segaddr = make_segmented_addr(addr);
		RW(regno) = (RW(regno) & 0x80ff) | ((segaddr >> 16) & 0x7f00);
		RW(regno | 1) = segaddr & 0xffff;
	}
	else
		RW(regno) = addr;
}

static Z8K_AI void add_to_addr_reg_(z8k_st_t *S, int regno, uint16_t addend)
{
	if (get_segmented_mode())
		regno |= 1;
	RW(regno) += addend;
}

static Z8K_AI void sub_from_addr_reg_(z8k_st_t *S, int regno, uint16_t subtrahend)
{
	if (get_segmented_mode())
		regno |= 1;
	RW(regno) -= subtrahend;
}

static Z8K_AI void set_pc_(z8k_st_t *S, uint32_t addr)
{
	if (get_segmented_mode())
		m_pc = addr;
	else
		m_pc = (m_pc & 0xffff0000) | (addr & 0xffff);
}

static Z8K_AI uint8_t RDIR_B_(z8k_st_t *S, uint8_t reg)
{
	return RDMEM_B(reg == SP ? m_stack : m_data, addr_from_reg(reg));
}

static Z8K_AI uint16_t RDIR_W_(z8k_st_t *S, uint8_t reg)
{
	return RDMEM_W(reg == SP ? m_stack : m_data, addr_from_reg(reg));
}

static Z8K_AI uint32_t RDIR_L_(z8k_st_t *S, uint8_t reg)
{
	return RDMEM_L(reg == SP ? m_stack : m_data, addr_from_reg(reg));
}

static Z8K_AI void WRIR_B_(z8k_st_t *S, uint8_t reg, uint8_t value)
{
	WRMEM_B(reg == SP ? m_stack : m_data, addr_from_reg(reg), value);
}

static Z8K_AI void WRIR_W_(z8k_st_t *S, uint8_t reg, uint16_t value)
{
	WRMEM_W(reg == SP ? m_stack : m_data, addr_from_reg(reg), value);
}

static Z8K_AI void WRIR_L_(z8k_st_t *S, uint8_t reg, uint32_t value)
{
	WRMEM_L(reg == SP ? m_stack : m_data, addr_from_reg(reg), value);
}

static Z8K_AI uint8_t RDBX_B_(z8k_st_t *S, uint8_t reg, uint16_t idx)
{
	return RDMEM_B(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx));
}

static Z8K_AI uint16_t RDBX_W_(z8k_st_t *S, uint8_t reg, uint16_t idx)
{
	return RDMEM_W(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx));
}

static Z8K_AI uint32_t RDBX_L_(z8k_st_t *S, uint8_t reg, uint16_t idx)
{
	return RDMEM_L(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx));
}

static Z8K_AI void WRBX_B_(z8k_st_t *S, uint8_t reg, uint16_t idx, uint8_t value)
{
	WRMEM_B(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx), value);
}

static Z8K_AI void WRBX_W_(z8k_st_t *S, uint8_t reg, uint16_t idx, uint16_t value)
{
	WRMEM_W(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx), value);
}

static Z8K_AI void WRBX_L_(z8k_st_t *S, uint8_t reg, uint16_t idx, uint32_t value)
{
	WRMEM_L(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx), value);
}

static Z8K_AI void PUSHW_(z8k_st_t *S, uint8_t dst, uint16_t value)
{
	if (get_segmented_mode())
		RW(dst | 1) -= 2;
	else
		RW(dst) -= 2;
	WRIR_W(dst, value);
}

static Z8K_AI uint16_t POPW_(z8k_st_t *S, uint8_t src)
{
	uint16_t result = RDIR_W(src);
	if (get_segmented_mode())
		RW(src | 1) += 2;
	else
		RW(src) += 2;
	return result;
}

static Z8K_AI void PUSHL_(z8k_st_t *S, uint8_t dst, uint32_t value)
{
	if (get_segmented_mode())
		RW(dst | 1) -= 4;
	else
		RW(dst) -= 4;
	WRIR_L(dst, value);
}

static Z8K_AI uint32_t POPL_(z8k_st_t *S, uint8_t src)
{
	uint32_t result = RDIR_L(src);
	if (get_segmented_mode())
		RW(src | 1) += 4;
	else
		RW(src) += 4;
	return result;
}

static Z8K_AI uint8_t ADDB_(z8k_st_t *S, uint8_t dest, uint8_t value)
{
	uint8_t result = dest + value;
	CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
	CLR_DA;         /* clear DA (decimal adjust) flag for addb */
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_ADDX_C;     /* set C if result overflowed              */
	CHK_ADDB_V;     /* set V if result has incorrect sign      */
	CHK_ADDB_H;     /* set H if lower nibble overflowed        */
	return result;
}

static Z8K_AI uint16_t ADDW_(z8k_st_t *S, uint16_t dest, uint16_t value)
{
	uint16_t result = dest + value;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_ADDX_C;     /* set C if result overflowed              */
	CHK_ADDW_V;     /* set V if result has incorrect sign      */
	return result;
}

static Z8K_AI uint32_t ADDL_(z8k_st_t *S, uint32_t dest, uint32_t value)
{
	uint32_t result = dest + value;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	CHK_ADDX_C;     /* set C if result overflowed              */
	CHK_ADDL_V;     /* set V if result has incorrect sign      */
	return result;
}

static Z8K_AI uint8_t ADCB_(z8k_st_t *S, uint8_t dest, uint8_t value)
{
	uint8_t result = dest + value + GET_C;
	CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
	CLR_DA;         /* clear DA (decimal adjust) flag for adcb */
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_ADCX_C;     /* set C if result overflowed              */
	CHK_ADDB_V;     /* set V if result has incorrect sign      */
	CHK_ADCB_H;     /* set H if lower nibble overflowed        */
	return result;
}

static Z8K_AI uint16_t ADCW_(z8k_st_t *S, uint16_t dest, uint16_t value)
{
	uint16_t result = dest + value + GET_C;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_ADCX_C;     /* set C if result overflowed              */
	CHK_ADDW_V;     /* set V if result has incorrect sign      */
	return result;
}

static Z8K_AI uint8_t SUBB_(z8k_st_t *S, uint8_t dest, uint8_t value)
{
	uint8_t result = dest - value;
	CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
	SET_DA;         /* set DA (decimal adjust) flag for subb   */
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBB_V;     /* set V if result has incorrect sign      */
	CHK_SUBB_H;     /* set H if lower nibble underflowed       */
	return result;
}

static Z8K_AI uint16_t SUBW_(z8k_st_t *S, uint16_t dest, uint16_t value)
{
	uint16_t result = dest - value;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBW_V;     /* set V if result has incorrect sign      */
	return result;
}

static Z8K_AI uint32_t SUBL_(z8k_st_t *S, uint32_t dest, uint32_t value)
{
	uint32_t result = dest - value;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBL_V;     /* set V if result has incorrect sign      */
	return result;
}

static Z8K_AI uint8_t SBCB_(z8k_st_t *S, uint8_t dest, uint8_t value)
{
	uint8_t result = dest - value - GET_C;
	CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
	SET_DA;         /* set DA (decimal adjust) flag for sbcb   */
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SBCX_C;     /* set C if result underflowed             */
	CHK_SUBB_V;     /* set V if result has incorrect sign      */
	CHK_SBCB_H;     /* set H if lower nibble underflowed       */
	return result;
}

static Z8K_AI uint16_t SBCW_(z8k_st_t *S, uint16_t dest, uint16_t value)
{
	uint16_t result = dest - value - GET_C;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SBCX_C;     /* set C if result underflowed             */
	CHK_SUBW_V;     /* set V if result has incorrect sign      */
	return result;
}

static Z8K_AI uint8_t ORB_(z8k_st_t *S, uint8_t dest, uint8_t value)
{
	uint8_t result = dest | value;
	CLR_ZSP;        /* first clear Z, S, P/V flags             */
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

static Z8K_AI uint16_t ORW_(z8k_st_t *S, uint16_t dest, uint16_t value)
{
	uint16_t result = dest | value;
	CLR_ZS;         /* first clear Z, and S flags              */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}

static Z8K_AI uint8_t ANDB_(z8k_st_t *S, uint8_t dest, uint8_t value)
{
	uint8_t result = dest & value;
	CLR_ZSP;        /* first clear Z,S and P/V flags           */
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

static Z8K_AI uint16_t ANDW_(z8k_st_t *S, uint16_t dest, uint16_t value)
{
	uint16_t result = dest & value;
	CLR_ZS;         /* first clear Z and S flags               */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}

static Z8K_AI uint8_t XORB_(z8k_st_t *S, uint8_t dest, uint8_t value)
{
	uint8_t result = dest ^ value;
	CLR_ZSP;        /* first clear Z, S and P/V flags          */
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

static Z8K_AI uint16_t XORW_(z8k_st_t *S, uint16_t dest, uint16_t value)
{
	uint16_t result = dest ^ value;
	CLR_ZS;         /* first clear Z and S flags               */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}

static Z8K_AI void CPB_(z8k_st_t *S, uint8_t dest, uint8_t value)
{
	uint8_t result = dest - value;
	CLR_CZSV;       /* first clear C, Z, S and P/V flags       */
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBB_V;
}

static Z8K_AI void CPW_(z8k_st_t *S, uint16_t dest, uint16_t value)
{
	uint16_t result = dest - value;
	CLR_CZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBW_V;
}

static Z8K_AI void CPL_(z8k_st_t *S, uint32_t dest, uint32_t value)
{
	uint32_t result = dest - value;
	CLR_CZSV;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBL_V;
}

static Z8K_AI uint8_t COMB_(z8k_st_t *S, uint8_t dest)
{
	uint8_t result = ~dest;
	CLR_ZSP;
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

static Z8K_AI uint16_t COMW_(z8k_st_t *S, uint16_t dest)
{
	uint16_t result = ~dest;
	CLR_ZS;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}

static Z8K_AI uint8_t NEGB_(z8k_st_t *S, uint8_t dest)
{
	uint8_t result = (uint8_t) -dest;
	CLR_CZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (result > 0) SET_C;
	if (result == S08) SET_V;
	return result;
}

static Z8K_AI uint16_t NEGW_(z8k_st_t *S, uint16_t dest)
{
	uint16_t result = (uint16_t) -dest;
	CLR_CZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (result > 0) SET_C;
	if (result == S16) SET_V;
	return result;
}

static Z8K_AI void TESTB_(z8k_st_t *S, uint8_t result)
{
	CLR_ZSP;
	CHK_XXXB_ZSP;   /* set Z and S flags for result byte       */
}

static Z8K_AI void TESTW_(z8k_st_t *S, uint16_t dest)
{
	CLR_ZS;
	if (!dest) SET_Z; else if (dest & S16) SET_S;
}

static Z8K_AI void TESTL_(z8k_st_t *S, uint32_t dest)
{
	CLR_ZS;
	if (!dest) SET_Z; else if (dest & S32) SET_S;
}

static Z8K_AI uint8_t INCB_(z8k_st_t *S, uint8_t dest, uint8_t value)
{
	uint8_t result = dest + value;
	CLR_ZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_ADDB_V;     /* set V if result overflowed              */
	return result;
}

static Z8K_AI uint16_t INCW_(z8k_st_t *S, uint16_t dest, uint16_t value)
{
	uint16_t result = dest + value;
	CLR_ZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result byte       */
	CHK_ADDW_V;     /* set V if result overflowed              */
	return result;
}

static Z8K_AI uint8_t DECB_(z8k_st_t *S, uint8_t dest, uint8_t value)
{
	uint8_t result = dest - value;
	CLR_ZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SUBB_V;     /* set V if result overflowed              */
	return result;
}

static Z8K_AI uint16_t DECW_(z8k_st_t *S, uint16_t dest, uint16_t value)
{
	uint16_t result = dest - value;
	CLR_ZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SUBW_V;     /* set V if result overflowed              */
	return result;
}

static Z8K_AI uint32_t MULTW_(z8k_st_t *S, uint16_t dest, uint16_t value)
{
	uint32_t result = (int32_t)(int16_t)dest * (int16_t)value;
	CLR_CZSV;
	CHK_XXXL_ZS;
	if(!value)
	{
		/* multiplication with zero is faster */
		m_icount += (70-18);
	}
	if((int32_t)result < -0x7fff || (int32_t)result >= 0x7fff) SET_C;
	return result;
}

static Z8K_AI uint64_t MULTL_(z8k_st_t *S, uint32_t dest, uint32_t value)
{
	uint64_t result = (int64_t)(int32_t)dest * (int32_t)value;
	if(!value)
	{
		/* multiplication with zero is faster */
		m_icount += (282 - 30);
	}
	else
	{
		int n;
		for(n = 0; n < 32; n++)
			if(dest & (1L << n)) m_icount -= 7;
	}
	CLR_CZSV;
	CHK_XXXQ_ZS;
	if((int64_t)result < -0x7fffffffL || (int64_t)result >= 0x7fffffffL) SET_C;
	return result;
}

static Z8K_AI uint32_t DIVW_(z8k_st_t *S, uint32_t dest, uint16_t value)
{
	uint32_t result = dest;
	uint16_t remainder = 0;
	CLR_CZSV;
	if (value)
	{
		uint16_t qsign = ((dest >> 16) ^ value) & S16;
		uint16_t rsign = (dest >> 16) & S16;
		if ((int32_t)dest < 0) dest = -dest;
		if ((int16_t)value < 0) value = -value;
		result = dest / value;
		remainder = dest % value;
		if (qsign) result = -result;
		if (rsign) remainder = -remainder;
		if ((int32_t)result < -0x8000 || (int32_t)result > 0x7fff)
		{
			int32_t temp = (int32_t)result >> 1;
			SET_V;
			if (temp >= -0x8000 && temp <= 0x7fff)
			{
				result = (temp < 0) ? -1 : 0;
				CHK_XXXW_ZS;
				SET_C;
			}
		}
		else
		{
			CHK_XXXW_ZS;
		}
		result = ((uint32_t)remainder << 16) | (result & 0xffff);
	}
	else
	{
		SET_Z;
		SET_V;
	}
	return result;
}

static Z8K_AI uint64_t DIVL_(z8k_st_t *S, uint64_t dest, uint32_t value)
{
	uint64_t result = dest;
	uint32_t remainder = 0;
	CLR_CZSV;
	if (value)
	{
		uint32_t qsign = ((dest >> 32) ^ value) & S32;
		uint32_t rsign = (dest >> 32) & S32;
		if ((int64_t)dest < 0) dest = -dest;
		if ((int32_t)value < 0) value = -value;
		result = dest / value;
		remainder = dest % value;
		if (qsign) result = -result;
		if (rsign) remainder = -remainder;
		if ((int64_t)result < -0x80000000LL || (int64_t)result > 0x7fffffff)
		{
			int64_t temp = (int64_t)result >> 1;
			SET_V;
			if (temp >= -0x80000000LL && temp <= 0x7fffffff)
			{
				result = (temp < 0) ? -1 : 0;
				CHK_XXXL_ZS;
				SET_C;
			}
		}
		else
		{
			CHK_XXXL_ZS;
		}
		result = ((uint64_t)remainder << 32) | (result & 0xffffffff);
	}
	else
	{
		SET_Z;
		SET_V;
	}
	return result;
}

static Z8K_AI uint8_t RLB_(z8k_st_t *S, uint8_t dest, uint8_t twice)
{
	uint8_t result = (dest << 1) | (dest >> 7);
	CLR_CZSV;
	if (twice) result = (result << 1) | (result >> 7);
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (result & 0x01) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

static Z8K_AI uint16_t RLW_(z8k_st_t *S, uint16_t dest, uint8_t twice)
{
	uint16_t result = (dest << 1) | (dest >> 15);
	CLR_CZSV;
	if (twice) result = (result << 1) | (result >> 15);
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (result & 0x0001) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

static Z8K_AI uint8_t RLCB_(z8k_st_t *S, uint8_t dest, uint8_t twice)
{
	uint8_t c = dest & S08;
	uint8_t result = (dest << 1) | GET_C;
	CLR_CZSV;
	if (twice) {
		uint8_t c1 = c >> 7;
		c = result & S08;
		result = (result << 1) | c1;
	}
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

static Z8K_AI uint16_t RLCW_(z8k_st_t *S, uint16_t dest, uint8_t twice)
{
	uint16_t c = dest & S16;
	uint16_t result = (dest << 1) | GET_C;
	CLR_CZSV;
	if (twice) {
		uint16_t c1 = c >> 15;
		c = result & S16;
		result = (result << 1) | c1;
	}
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

static Z8K_AI uint8_t RRB_(z8k_st_t *S, uint8_t dest, uint8_t twice)
{
	uint8_t result = (dest >> 1) | (dest << 7);
	CLR_CZSV;
	if (twice) result = (result >> 1) | (result << 7);
	if (!result) SET_Z; else if (result & S08) SET_SC;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

static Z8K_AI uint16_t RRW_(z8k_st_t *S, uint16_t dest, uint8_t twice)
{
	uint16_t result = (dest >> 1) | (dest << 15);
	CLR_CZSV;
	if (twice) result = (result >> 1) | (result << 15);
	if (!result) SET_Z; else if (result & S16) SET_SC;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

static Z8K_AI uint8_t RRCB_(z8k_st_t *S, uint8_t dest, uint8_t twice)
{
	uint8_t c = dest & 1;
	uint8_t result = (dest >> 1) | (GET_C << 7);
	CLR_CZSV;
	if (twice) {
		uint8_t c1 = c << 7;
		c = result & 1;
		result = (result >> 1) | c1;
	}
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

static Z8K_AI uint16_t RRCW_(z8k_st_t *S, uint16_t dest, uint8_t twice)
{
	uint16_t c = dest & 1;
	uint16_t result = (dest >> 1) | (GET_C << 15);
	CLR_CZSV;
	if (twice) {
		uint16_t c1 = c << 15;
		c = result & 1;
		result = (result >> 1) | c1;
	}
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

static Z8K_AI uint8_t SDAB_(z8k_st_t *S, uint8_t dest, int8_t count)
{
	int8_t result = (int8_t) dest;
	uint8_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S08;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x01;
		result >>= 1;
		count++;
	}
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return (uint8_t)result;
}

static Z8K_AI uint16_t SDAW_(z8k_st_t *S, uint16_t dest, int8_t count)
{
	int16_t result = (int16_t) dest;
	uint16_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S16;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x0001;
		result >>= 1;
		count++;
	}
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return (uint16_t)result;
}

static Z8K_AI uint32_t SDAL_(z8k_st_t *S, uint32_t dest, int8_t count)
{
	int32_t result = (int32_t) dest;
	uint32_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S32;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x00000001;
		result >>= 1;
		count++;
	}
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	if ((result ^ dest) & S32) SET_V;
	return (uint32_t) result;
}

static Z8K_AI uint8_t SDLB_(z8k_st_t *S, uint8_t dest, int8_t count)
{
	uint8_t result = dest;
	uint8_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S08;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x01;
		result >>= 1;
		count++;
	}
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

static Z8K_AI uint16_t SDLW_(z8k_st_t *S, uint16_t dest, int8_t count)
{
	uint16_t result = dest;
	uint16_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S16;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x0001;
		result >>= 1;
		count++;
	}
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

static Z8K_AI uint32_t SDLL_(z8k_st_t *S, uint32_t dest, int8_t count)
{
	uint32_t result = dest;
	uint32_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S32;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x00000001;
		result >>= 1;
		count++;
	}
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	if ((result ^ dest) & S32) SET_V;
	return result;
}

static Z8K_AI uint8_t SLAB_(z8k_st_t *S, uint8_t dest, uint8_t count)
{
	uint8_t c = (count) ? (dest << (count - 1)) & S08 : 0;
	uint8_t result = (uint8_t)((int8_t)dest << count);
	CLR_CZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

static Z8K_AI uint16_t SLAW_(z8k_st_t *S, uint16_t dest, uint8_t count)
{
	uint16_t c = (count) ? (dest << (count - 1)) & S16 : 0;
	uint16_t result = (uint16_t)((int16_t)dest << count);
	CLR_CZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

static Z8K_AI uint32_t SLAL_(z8k_st_t *S, uint32_t dest, uint8_t count)
{
	uint32_t c = (count) ? (dest << (count - 1)) & S32 : 0;
	uint32_t result = (uint32_t)((int32_t)dest << count);
	CLR_CZSV;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	if ((result ^ dest) & S32) SET_V;
	return result;
}

static Z8K_AI uint8_t SLLB_(z8k_st_t *S, uint8_t dest, uint8_t count)
{
	uint8_t c = (count) ? (dest << (count - 1)) & S08 : 0;
	uint8_t result = dest << count;
	CLR_CZS;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	return result;
}

static Z8K_AI uint16_t SLLW_(z8k_st_t *S, uint16_t dest, uint8_t count)
{
	uint16_t c = (count) ? (dest << (count - 1)) & S16 : 0;
	uint16_t result = dest << count;
	CLR_CZS;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	return result;
}

static Z8K_AI uint32_t SLLL_(z8k_st_t *S, uint32_t dest, uint8_t count)
{
	uint32_t c = (count) ? (dest << (count - 1)) & S32 : 0;
	uint32_t result = dest << count;
	CLR_CZS;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	return result;
}

static Z8K_AI uint8_t SRAB_(z8k_st_t *S, uint8_t dest, uint8_t count)
{
	uint8_t c = (count) ? ((int8_t)dest >> (count - 1)) & 1 : 0;
	uint8_t result = (uint8_t)((int8_t)dest >> count);
	CLR_CZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	return result;
}

static Z8K_AI uint16_t SRAW_(z8k_st_t *S, uint16_t dest, uint8_t count)
{
	uint8_t c = (count) ? ((int16_t)dest >> (count - 1)) & 1 : 0;
	uint16_t result = (uint16_t)((int16_t)dest >> count);
	CLR_CZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	return result;
}

static Z8K_AI uint32_t SRAL_(z8k_st_t *S, uint32_t dest, uint8_t count)
{
	uint8_t c = (count) ? ((int32_t)dest >> (count - 1)) & 1 : 0;
	uint32_t result = (uint32_t)((int32_t)dest >> count);
	CLR_CZSV;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	return result;
}

static Z8K_AI uint8_t SRLB_(z8k_st_t *S, uint8_t dest, uint8_t count)
{
	uint8_t c = (count) ? (dest >> (count - 1)) & 1 : 0;
	uint8_t result = dest >> count;
	CLR_CZS;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	return result;
}

static Z8K_AI uint16_t SRLW_(z8k_st_t *S, uint16_t dest, uint8_t count)
{
	uint8_t c = (count) ? (dest >> (count - 1)) & 1 : 0;
	uint16_t result = dest >> count;
	CLR_CZS;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	return result;
}

static Z8K_AI uint32_t SRLL_(z8k_st_t *S, uint32_t dest, uint8_t count)
{
	uint8_t c = (count) ? (dest >> (count - 1)) & 1 : 0;
	uint32_t result = dest >> count;
	CLR_CZS;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	return result;
}

static Z8K_AI void zinvalid(z8k_st_t *S)
{
	Z8K_NOLOG("Z8000 invalid opcode %04x: %04x\n", m_pc, m_op0);
}

static Z8K_AI void Z00_0000_dddd_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = ADDB(RB(dst), imm8);
}

static Z8K_AI void Z00_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ADDB(RB(dst), RDIR_B(src));
}

static Z8K_AI void Z01_0000_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = ADDW(RW(dst), imm16);
}

static Z8K_AI void Z01_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ADDW(RW(dst), RDIR_W(src));
}

static Z8K_AI void Z02_0000_dddd_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = SUBB(RB(dst), imm8);
}

static Z8K_AI void Z02_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = SUBB(RB(dst), RDIR_B(src)); /* EHC */
}

static Z8K_AI void Z03_0000_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = SUBW(RW(dst), imm16);
}

static Z8K_AI void Z03_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = SUBW(RW(dst), RDIR_W(src));
}

static Z8K_AI void Z04_0000_dddd_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = ORB(RB(dst), imm8);
}

static Z8K_AI void Z04_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ORB(RB(dst), RDIR_B(src));
}

static Z8K_AI void Z05_0000_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = ORW(RW(dst), imm16);
}

static Z8K_AI void Z05_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ORW(RW(dst), RDIR_W(src));
}

static Z8K_AI void Z06_0000_dddd_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = ANDB(RB(dst), imm8);
}

static Z8K_AI void Z06_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ANDB(RB(dst), RDIR_B(src));
}

static Z8K_AI void Z07_0000_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = ANDW(RW(dst), imm16);
}

static Z8K_AI void Z07_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ANDW(RW(dst), RDIR_W(src));
}

static Z8K_AI void Z08_0000_dddd_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = XORB(RB(dst), imm8);
}

static Z8K_AI void Z08_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = XORB(RB(dst), RDIR_B(src));
}

static Z8K_AI void Z09_0000_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = XORW(RW(dst), imm16);
}

static Z8K_AI void Z09_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = XORW(RW(dst), RDIR_W(src));
}

static Z8K_AI void Z0A_0000_dddd_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	CPB(RB(dst), imm8);
}

static Z8K_AI void Z0A_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPB(RB(dst), RDIR_B(src));
}

static Z8K_AI void Z0B_0000_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	CPW(RW(dst), imm16);
}

static Z8K_AI void Z0B_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPW(RW(dst), RDIR_W(src));
}

static Z8K_AI void Z0C_ddN0_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, COMB(RDMEM_B(space, addr)));
}

static Z8K_AI void Z0C_ddN0_0001_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	CPB(RDIR_B(dst), imm8); // @@@done
}

static Z8K_AI void Z0C_ddN0_0010(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, NEGB(RDMEM_B(space, addr)));
}

static Z8K_AI void Z0C_ddN0_0100(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	TESTB(RDIR_B(dst));
}

static Z8K_AI void Z0C_ddN0_0101_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	WRIR_B(dst, imm8);
}

static Z8K_AI void Z0C_ddN0_0110(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	if (RDMEM_B(space, addr) & S08) SET_S; else CLR_S;
	WRMEM_B(space, addr, 0xff);
}

static Z8K_AI void Z0C_ddN0_1000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	WRIR_B(dst, 0);
}

static Z8K_AI void Z0D_ddN0_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, COMW(RDMEM_W(space, addr)));
}

static Z8K_AI void Z0D_ddN0_0001_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	CPW(RDIR_W(dst), imm16);
}

static Z8K_AI void Z0D_ddN0_0010(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, NEGW(RDMEM_W(space, addr)));
}

static Z8K_AI void Z0D_ddN0_0100(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	TESTW(RDIR_W(dst));
}

static Z8K_AI void Z0D_ddN0_0101_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	WRIR_W(dst, imm16);
}

static Z8K_AI void Z0D_ddN0_0110(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	if (RDMEM_W(space, addr) & S16) SET_S; else CLR_S;
	WRMEM_W(space, addr, 0xffff);
}

static Z8K_AI void Z0D_ddN0_1000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	WRIR_W(dst, 0);
}

static Z8K_AI void Z0D_ddN0_1001_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	PUSHW(dst, imm16);
}

static Z8K_AI void Z0E_imm8(z8k_st_t *S)
{
	CHECK_EXT_INSTR();
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: ext0e  $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

static Z8K_AI void Z0F_imm8(z8k_st_t *S)
{
	CHECK_EXT_INSTR();
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: ext0f  $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

static Z8K_AI void Z10_0000_dddd_imm32(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	CPL(RL(dst), imm32);
}

static Z8K_AI void Z10_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPL(RL(dst), RDIR_L(src));
}

static Z8K_AI void Z11_ddN0_ssN0(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHL(dst, RDIR_L(src));
}

static Z8K_AI void Z12_0000_dddd_imm32(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RL(dst) = SUBL(RL(dst), imm32);
}

static Z8K_AI void Z12_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = SUBL(RL(dst), RDIR_L(src));
}

static Z8K_AI void Z13_ddN0_ssN0(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHW(dst, RDIR_W(src));
}

static Z8K_AI void Z14_0000_dddd_imm32(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RL(dst) = imm32;
}

static Z8K_AI void Z14_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = RDIR_L(src);
}

static Z8K_AI void Z15_ssN0_ddN0(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = POPL(src);
}

static Z8K_AI void Z16_0000_dddd_imm32(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RL(dst) = ADDL(RL(dst), imm32);
}

static Z8K_AI void Z16_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = ADDL(RL(dst), RDIR_L(src));
}

static Z8K_AI void Z17_ssN0_ddN0(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	WRIR_W(dst, POPW(src));
}

static Z8K_AI void Z18_00N0_dddd_imm32(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RQ(dst) = MULTL(RQ(dst), imm32);
}

static Z8K_AI void Z18_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RQ(dst) = MULTL(RQ(dst), RL(src)); //@@@
}

static Z8K_AI void Z19_0000_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RL(dst) = MULTW(RL(dst), imm16);
}

static Z8K_AI void Z19_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = MULTW(RL(dst), RDIR_W(src));
}

static Z8K_AI void Z1A_0000_dddd_imm32(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RQ(dst) = DIVL(RQ(dst), imm32);
}

static Z8K_AI void Z1A_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RQ(dst) = DIVL(RQ(dst), RDIR_L(src));
}

static Z8K_AI void Z1B_0000_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RL(dst) = DIVW(RL(dst), imm16);
}

static Z8K_AI void Z1B_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = DIVW(RL(dst), RDIR_W(src));
}

static Z8K_AI void Z1C_ddN0_1000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	TESTL(RDIR_L(dst));
}

static Z8K_AI void Z1C_ddN0_1001_0000_ssss_0000_nmin1(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_CNT(OP1,NIB3);
	GET_SRC(OP1,NIB1);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	while (cnt-- >= 0) {
		WRMEM_W(space, addr, RW(src));
		addr = addr_add(addr, 2);
		src = (src+1) & 15;
	}
}

static Z8K_AI void Z1C_ssN0_0001_0000_dddd_0000_nmin1(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB3);
	GET_DST(OP1,NIB1);
	int space = src == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(src);
	while (cnt-- >= 0) {
		RW(dst) = RDMEM_W(space, addr);
		addr = addr_add(addr, 2);
		dst = (dst+1) & 15;
	}
}

static Z8K_AI void Z1D_ddN0_ssss(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRIR_L(dst, RL(src));
}

static Z8K_AI void Z1E_ddN0_cccc(z8k_st_t *S)
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	switch (cc) {
		case  0: if (CC0) set_pc(addr_from_reg(dst)); break;
		case  1: if (CC1) set_pc(addr_from_reg(dst)); break;
		case  2: if (CC2) set_pc(addr_from_reg(dst)); break;
		case  3: if (CC3) set_pc(addr_from_reg(dst)); break;
		case  4: if (CC4) set_pc(addr_from_reg(dst)); break;
		case  5: if (CC5) set_pc(addr_from_reg(dst)); break;
		case  6: if (CC6) set_pc(addr_from_reg(dst)); break;
		case  7: if (CC7) set_pc(addr_from_reg(dst)); break;
		case  8: if (CC8) set_pc(addr_from_reg(dst)); break;
		case  9: if (CC9) set_pc(addr_from_reg(dst)); break;
		case 10: if (CCA) set_pc(addr_from_reg(dst)); break;
		case 11: if (CCB) set_pc(addr_from_reg(dst)); break;
		case 12: if (CCC) set_pc(addr_from_reg(dst)); break;
		case 13: if (CCD) set_pc(addr_from_reg(dst)); break;
		case 14: if (CCE) set_pc(addr_from_reg(dst)); break;
		case 15: if (CCF) set_pc(addr_from_reg(dst)); break;
	}
}

static Z8K_AI void Z1F_ddN0_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	if (get_segmented_mode())
		PUSHL(SP, make_segmented_addr(m_pc));
	else
		PUSHW(SP, m_pc);
	set_pc(addr_from_reg(dst));
}

static Z8K_AI void Z20_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = RDIR_B(src);
}

static Z8K_AI void Z21_0000_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = imm16;
}

static Z8K_AI void Z21_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = RDIR_W(src);
}

static Z8K_AI void Z22_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RB(dst) = RB(dst) & ~(1 << (RW(src) & 7));
}

static Z8K_AI void Z22_ddN0_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, RDMEM_B(space, addr) & ~bit);
}

static Z8K_AI void Z23_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RW(dst) = RW(dst) & ~(1 << (RW(src) & 15));
}

static Z8K_AI void Z23_ddN0_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, RDMEM_W(space, addr) & ~bit);
}

static Z8K_AI void Z24_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RB(dst) = RB(dst) | (1 << (RW(src) & 7));
}

static Z8K_AI void Z24_ddN0_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, RDMEM_B(space, addr) | bit);
}

static Z8K_AI void Z25_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RW(dst) = RW(dst) | (1 << (RW(src) & 15));
}

static Z8K_AI void Z25_ddN0_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, RDMEM_W(space, addr) | bit);
}

static Z8K_AI void Z26_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	if (RB(dst) & (1 << (RW(src) & 7))) CLR_Z; else SET_Z;
}

static Z8K_AI void Z26_ddN0_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RDIR_B(dst) & bit) CLR_Z; else SET_Z;
}

static Z8K_AI void Z27_0000_ssss_0000_dddd_0000_0000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	if (RW(dst) & (1 << (RW(src) & 15))) CLR_Z; else SET_Z;
}

static Z8K_AI void Z27_ddN0_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RDIR_W(dst) & bit) CLR_Z; else SET_Z;
}

static Z8K_AI void Z28_ddN0_imm4m1(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, INCB(RDMEM_B(space, addr), i4p1));
}

static Z8K_AI void Z29_ddN0_imm4m1(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, INCW(RDMEM_W(space, addr), i4p1));
}

static Z8K_AI void Z2A_ddN0_imm4m1(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, DECB(RDMEM_B(space, addr), i4p1));
}

static Z8K_AI void Z2B_ddN0_imm4m1(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	int space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, DECW(RDMEM_W(space, addr), i4p1));
}

static Z8K_AI void Z2C_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	int space = src == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(src);
	uint8_t tmp = RDMEM_B(space, addr);
	WRMEM_B(space, addr, RB(dst));
	RB(dst) = tmp;
}

static Z8K_AI void Z2D_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	int space = src == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(src);
	uint16_t tmp = RDMEM_W(space, addr);
	WRMEM_W(space, addr, RW(dst));
	RW(dst) = tmp;
}

static Z8K_AI void Z2E_ddN0_ssss(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRIR_B(dst, RB(src));
}

static Z8K_AI void Z2F_ddN0_ssss(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRIR_W(dst, RW(src));
}

static Z8K_AI void Z30_0000_dddd_dsp16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RB(dst) = RDMEM_B(m_program, dsp16);
}

static Z8K_AI void Z30_ssN0_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	RB(dst) = RDBX_B(src, idx16);
}

static Z8K_AI void Z31_0000_dddd_dsp16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RW(dst) = RDMEM_W(m_program, dsp16);
}

static Z8K_AI void Z31_ssN0_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	RW(dst) = RDBX_W(src, idx16);
}

static Z8K_AI void Z32_0000_ssss_dsp16(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_B(m_program, dsp16, RB(src));
}

static Z8K_AI void Z32_ddN0_ssss_imm16(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX16(OP1);
	WRBX_B(dst, idx16, RB(src));
}

static Z8K_AI void Z33_0000_ssss_dsp16(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_W(m_program, dsp16, RW(src));
}

static Z8K_AI void Z33_ddN0_ssss_imm16(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX16(OP1);
	WRBX_W(dst, idx16, RW(src));
}

static Z8K_AI void Z34_0000_dddd_dsp16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	addr_to_reg(dst, dsp16);
}

static Z8K_AI void Z34_ssN0_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	if (get_segmented_mode()) {
		RL(dst) = RL(src);
	}
	else {
		RW(dst) = RW(src);
	}
	add_to_addr_reg(dst, idx16);
}

static Z8K_AI void Z35_0000_dddd_dsp16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RL(dst) = RDMEM_L(m_program, dsp16);
}

static Z8K_AI void Z35_ssN0_dddd_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	RL(dst) = RDBX_L(src, idx16);
}

static Z8K_AI void Z36_0000_0000(z8k_st_t *S)
{
	/* execute break point trap m_irq_req */
	m_irq_req |= Z8000_TRAP;
}

static Z8K_AI void Z36_imm8(z8k_st_t *S)
{
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: rsvd36 $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

static Z8K_AI void Z37_0000_ssss_dsp16(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_L(m_program,  dsp16, RL(src));
}

static Z8K_AI void Z37_ddN0_ssss_imm16(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX16(OP1);
	WRBX_L(dst, idx16, RL(src));
}

static Z8K_AI void Z38_imm8(z8k_st_t *S)
{
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: rsvd38 $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

static Z8K_AI void Z39_ssN0_0000(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	uint16_t fcw;
	int space = src == SP ? m_stack : m_data;
	if (get_segmented_mode()) {
		uint32_t addr = addr_from_reg(src);
		fcw = RDMEM_W(space, addr + 2);
		set_pc(segmented_addr(RDMEM_L(space, addr + 4)));
	}
	else {
		fcw = RDMEM_W(space, RW(src));
		set_pc(RDMEM_W(space, (uint16_t)(RW(src) + 2)));
	}
	if ((fcw ^ m_fcw) & F_SEG) Z8K_NOLOG("ldps 1 (0x%05x): changing from %ssegmented mode to %ssegmented mode\n", m_pc, (m_fcw & F_SEG) ? "non-" : "", (fcw & F_SEG) ? "" : "non-");
	CHANGE_FCW(fcw); /* check for user/system mode change */
}

static Z8K_AI void Z3A_ssss_0000_0000_aaaa_dddd_x000(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_B(dst, RDPORT_B( 0, RW(src)));
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3A_ssss_0001_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_B(dst, RDPORT_B( 1, RW(src)));
	RW(dst)++;
	RW(src)++;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3A_ssss_0010_0000_aaaa_dddd_x000(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B( 0, RW(dst), RDIR_B(src));
	add_to_addr_reg(src, 1);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3A_ssss_0011_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B( 1, RW(dst), RDIR_W(src));
	RW(dst)++;
	RW(src)++;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3A_dddd_0100_imm16(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	RB(dst) = RDPORT_B( 0, imm16);
}

static Z8K_AI void Z3A_dddd_0101_imm16(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	RB(dst) = RDPORT_B( 1, imm16);
}

static Z8K_AI void Z3A_ssss_0110_imm16(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	WRPORT_B( 0, imm16, RB(src));
}

static Z8K_AI void Z3A_ssss_0111_imm16(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	WRPORT_B( 1, imm16, RB(src));
}

static Z8K_AI void Z3A_ssss_1000_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_B(dst, RDPORT_B( 0, RW(src)));
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3A_ssss_1001_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_B(dst, RDPORT_B( 1, RW(src)));
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3A_ssss_1010_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B( 0, RW(dst), RDIR_B(src));
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3A_ssss_1011_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B( 1, RW(dst), RDIR_B(src));
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3B_ssss_0000_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDPORT_W( 0, RW(src)));
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3B_ssss_0001_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDPORT_W( 1, RW(src)));
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3B_ssss_0010_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 0, RW(dst), RDIR_W(src));
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3B_ssss_0011_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 1, RW(dst), RDIR_W(src));
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3B_dddd_0100_imm16(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	RW(dst) = RDPORT_W( 0, imm16);
}

static Z8K_AI void Z3B_dddd_0101_imm16(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	RW(dst) = RDPORT_W( 1, imm16);
}

static Z8K_AI void Z3B_ssss_0110_imm16(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	WRPORT_W( 0, imm16, RW(src));
}

static Z8K_AI void Z3B_ssss_0111_imm16(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	WRPORT_W( 1, imm16, RW(src));
}

static Z8K_AI void Z3B_ssss_1000_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDPORT_W( 0, RW(src)));
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3B_ssss_1001_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDPORT_W( 1, RW(src)));
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3B_ssss_1010_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 0, RW(dst), RDIR_W(src));
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3B_ssss_1011_0000_aaaa_dddd_x000(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 1, RW(dst), RDIR_W(src));
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void Z3C_ssss_dddd(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_DST(OP0,NIB3);
	RB(dst) = RDPORT_B( 0, RW(src));
}

static Z8K_AI void Z3D_ssss_dddd(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_DST(OP0,NIB3);
	RW(dst) = RDPORT_W( 0, RW(src));
}

static Z8K_AI void Z3E_dddd_ssss(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	WRPORT_B( 0, RW(dst), RB(src));
}

static Z8K_AI void Z3F_dddd_ssss(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	WRPORT_W( 0, RW(dst), RW(src));
}

static Z8K_AI void Z40_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = ADDB(RB(dst), RDMEM_B(m_data, addr));
}

static Z8K_AI void Z40_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = ADDB(RB(dst), RDMEM_B(m_data, addr));
}

static Z8K_AI void Z41_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ADDW(RW(dst), RDMEM_W(m_data, addr)); /* EHC */
}

static Z8K_AI void Z41_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = ADDW(RW(dst), RDMEM_W(m_data, addr));    /* ASG */
}

static Z8K_AI void Z42_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = SUBB(RB(dst), RDMEM_B(m_data, addr)); /* EHC */
}

static Z8K_AI void Z42_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = SUBB(RB(dst), RDMEM_B(m_data, addr));
}

static Z8K_AI void Z43_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = SUBW(RW(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z43_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = SUBW(RW(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z44_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = ORB(RB(dst), RDMEM_B(m_data, addr));
}

static Z8K_AI void Z44_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = ORB(RB(dst), RDMEM_B(m_data, addr));
}

static Z8K_AI void Z45_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ORW(RW(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z45_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = ORW(RW(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z46_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = ANDB(RB(dst), RDMEM_B(m_data, addr));
}

static Z8K_AI void Z46_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = ANDB(RB(dst), RDMEM_B(m_data, addr));
}

static Z8K_AI void Z47_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ANDW(RW(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z47_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = ANDW(RW(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z48_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = XORB(RB(dst), RDMEM_B(m_data, addr));
}

static Z8K_AI void Z48_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = XORB(RB(dst), RDMEM_B(m_data, addr));
}

static Z8K_AI void Z49_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = XORW(RW(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z49_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = XORW(RW(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z4A_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPB(RB(dst), RDMEM_B(m_data, addr));
}

static Z8K_AI void Z4A_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	CPB(RB(dst), RDMEM_B(m_data, addr));
}

static Z8K_AI void Z4B_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPW(RW(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z4B_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	CPW(RW(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z4C_0000_0000_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	WRMEM_B(m_data,  addr, COMB(RDMEM_W(m_data, addr)));
}

static Z8K_AI void Z4C_0000_0001_addr_imm8(z8k_st_t *S)
{
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	CPB(RDMEM_B(m_data, addr), imm8);
}

static Z8K_AI void Z4C_0000_0010_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	WRMEM_B(m_data,  addr, NEGB(RDMEM_B(m_data, addr)));
}

static Z8K_AI void Z4C_0000_0100_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	TESTB(RDMEM_B(m_data, addr));
}

static Z8K_AI void Z4C_0000_0101_addr_imm8(z8k_st_t *S)
{
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	WRMEM_B(m_data,  addr, imm8);
}

static Z8K_AI void Z4C_0000_0110_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	if (RDMEM_B(m_data, addr) & S08) SET_S; else CLR_S;
	WRMEM_B(m_data, addr, 0xff);
}

static Z8K_AI void Z4C_0000_1000_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	WRMEM_B(m_data,  addr, 0);
}

static Z8K_AI void Z4C_ddN0_0000_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data,  addr, COMB(RDMEM_B(m_data, addr)));
}

static Z8K_AI void Z4C_ddN0_0001_addr_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	addr = addr_add(addr, RW(dst));
	CPB(RDMEM_B(m_data, addr), imm8);
}

static Z8K_AI void Z4C_ddN0_0010_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, NEGB(RDMEM_B(m_data, addr)));
}

static Z8K_AI void Z4C_ddN0_0100_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	TESTB(RDMEM_B(m_data, addr));
}

static Z8K_AI void Z4C_ddN0_0101_addr_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, imm8);
}

static Z8K_AI void Z4C_ddN0_0110_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	if (RDMEM_B(m_data, addr) & S08) SET_S; else CLR_S;
	WRMEM_B(m_data, addr, 0xff);
}

static Z8K_AI void Z4C_ddN0_1000_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, 0);
}

static Z8K_AI void Z4D_0000_0000_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	WRMEM_W(m_data,  addr, COMW(RDMEM_W(m_data, addr)));
}

static Z8K_AI void Z4D_0000_0001_addr_imm16(z8k_st_t *S)
{
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	CPW(RDMEM_W(m_data, addr), imm16);
}

static Z8K_AI void Z4D_0000_0010_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	WRMEM_W(m_data,  addr, NEGW(RDMEM_W(m_data, addr)));
}

static Z8K_AI void Z4D_0000_0100_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	TESTW(RDMEM_W(m_data, addr));
}

static Z8K_AI void Z4D_0000_0101_addr_imm16(z8k_st_t *S)
{
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	WRMEM_W(m_data,  addr, imm16);
}

static Z8K_AI void Z4D_0000_0110_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	if (RDMEM_W(m_data, addr) & S16) SET_S; else CLR_S;
	WRMEM_W(m_data, addr, 0xffff);
}

static Z8K_AI void Z4D_0000_1000_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	WRMEM_W(m_data,  addr, 0);
}

static Z8K_AI void Z4D_ddN0_0000_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, COMW(RDMEM_W(m_data, addr)));
}

static Z8K_AI void Z4D_ddN0_0001_addr_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	addr = addr_add(addr, RW(dst));
	CPW(RDMEM_W(m_data, addr), imm16);
}

static Z8K_AI void Z4D_ddN0_0010_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, NEGW(RDMEM_W(m_data, addr)));
}

static Z8K_AI void Z4D_ddN0_0100_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	TESTW(RDMEM_W(m_data, addr));
}

static Z8K_AI void Z4D_ddN0_0101_addr_imm16(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, imm16);
}

static Z8K_AI void Z4D_ddN0_0110_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	if (RDMEM_W(m_data, addr) & S16) SET_S; else CLR_S;
	WRMEM_W(m_data, addr, 0xffff);
}

static Z8K_AI void Z4D_ddN0_1000_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, 0);
}

static Z8K_AI void Z4E_ddN0_ssN0_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, RB(src));
}

static Z8K_AI void Z50_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPL(RL(dst), RDMEM_L(m_data, addr));
}

static Z8K_AI void Z50_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	CPL(RL(dst), RDMEM_L(m_data, addr));
}

static Z8K_AI void Z51_ddN0_0000_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	PUSHL(dst, RDMEM_L(m_data, addr));
}

static Z8K_AI void Z51_ddN0_ssN0_addr(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	PUSHL(dst, RDMEM_L(m_data, addr));
}

static Z8K_AI void Z52_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = SUBL(RL(dst), RDMEM_L(m_data, addr));
}

static Z8K_AI void Z52_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RL(dst) = SUBL(RL(dst), RDMEM_L(m_data, addr));
}

static Z8K_AI void Z53_ddN0_0000_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	PUSHW(dst, RDMEM_W(m_data, addr));
}

static Z8K_AI void Z53_ddN0_ssN0_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	PUSHW(dst, RDMEM_W(m_data, addr));
}

static Z8K_AI void Z54_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = RDMEM_L(m_data, addr);
}

static Z8K_AI void Z54_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RL(dst) = RDMEM_L(m_data, addr);
}

static Z8K_AI void Z55_ssN0_0000_addr(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	WRMEM_L(m_data, addr, POPL(src));
}

static Z8K_AI void Z55_ssN0_ddN0_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_L(m_data, addr, POPL(src));
}

static Z8K_AI void Z56_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = ADDL(RL(dst), RDMEM_L(m_data, addr));
}

static Z8K_AI void Z56_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RL(dst) = ADDL(RL(dst), RDMEM_L(m_data, addr));
}

static Z8K_AI void Z57_ssN0_0000_addr(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	WRMEM_W(m_data, addr, POPW(src));
}

static Z8K_AI void Z57_ssN0_ddN0_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, POPW(src));
}

static Z8K_AI void Z58_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RQ(dst) = MULTL(RQ(dst), RDMEM_L(m_data, addr));
}

static Z8K_AI void Z58_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RQ(dst) = MULTL(RQ(dst), RDMEM_L(m_data, addr));
}

static Z8K_AI void Z59_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = MULTW(RL(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z59_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RL(dst) = MULTW(RL(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z5A_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RQ(dst) = DIVL(RQ(dst), RDMEM_L(m_data, addr));
}

static Z8K_AI void Z5A_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RQ(dst) = DIVL(RQ(dst), RDMEM_L(m_data, addr));
}

static Z8K_AI void Z5B_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = DIVW(RL(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z5B_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RL(dst) = DIVW(RL(dst), RDMEM_W(m_data, addr));
}

static Z8K_AI void Z5C_0000_0001_0000_dddd_0000_nmin1_addr(z8k_st_t *S)
{
	GET_DST(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	while (cnt-- >= 0) {
		RW(dst) = RDMEM_W(m_data, addr);
		dst = (dst+1) & 15;
		addr = addr_add (addr, 2);
	}
}

static Z8K_AI void Z5C_0000_1000_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	TESTL(RDMEM_L(m_data, addr));
}

static Z8K_AI void Z5C_0000_1001_0000_ssss_0000_nmin1_addr(z8k_st_t *S)
{
	GET_SRC(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	while (cnt-- >= 0) {
		WRMEM_W(m_data, addr, RW(src));
		src = (src+1) & 15;
		addr = addr_add (addr, 2);
	}
}

static Z8K_AI void Z5C_ddN0_1000_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	TESTL(RDMEM_L(m_data, addr));
}

static Z8K_AI void Z5C_ddN0_1001_0000_ssN0_0000_nmin1_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	addr = addr_add(addr, RW(dst));
	while (cnt-- >= 0) {
		WRMEM_W(m_data, addr, RW(src));
		src = (src+1) & 15;
		addr = addr_add(addr, 2);
	}
}

static Z8K_AI void Z5C_ssN0_0001_0000_dddd_0000_nmin1_addr(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_DST(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	addr = addr_add(addr, RW(src));
	while (cnt-- >= 0) {
		RW(dst) = RDMEM_W(m_data, addr);
		dst = (dst+1) & 15;
		addr = addr_add(addr, 2);
	}
}

static Z8K_AI void Z5D_0000_ssss_addr(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_L(m_data, addr, RL(src));
}

static Z8K_AI void Z5D_ddN0_ssss_addr(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_L(m_data, addr, RL(src));
}

static Z8K_AI void Z5E_0000_cccc_addr(z8k_st_t *S)
{
	GET_CCC(OP0,NIB3);
	GET_ADDR(OP1);
	switch (cc) {
		case  0: if (CC0) set_pc(addr); break;
		case  1: if (CC1) set_pc(addr); break;
		case  2: if (CC2) set_pc(addr); break;
		case  3: if (CC3) set_pc(addr); break;
		case  4: if (CC4) set_pc(addr); break;
		case  5: if (CC5) set_pc(addr); break;
		case  6: if (CC6) set_pc(addr); break;
		case  7: if (CC7) set_pc(addr); break;
		case  8: if (CC8) set_pc(addr); break;
		case  9: if (CC9) set_pc(addr); break;
		case 10: if (CCA) set_pc(addr); break;
		case 11: if (CCB) set_pc(addr); break;
		case 12: if (CCC) set_pc(addr); break;
		case 13: if (CCD) set_pc(addr); break;
		case 14: if (CCE) set_pc(addr); break;
		case 15: if (CCF) set_pc(addr); break;
	}
}

static Z8K_AI void Z5E_ddN0_cccc_addr(z8k_st_t *S)
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	switch (cc) {
		case  0: if (CC0) set_pc(addr); break;
		case  1: if (CC1) set_pc(addr); break;
		case  2: if (CC2) set_pc(addr); break;
		case  3: if (CC3) set_pc(addr); break;
		case  4: if (CC4) set_pc(addr); break;
		case  5: if (CC5) set_pc(addr); break;
		case  6: if (CC6) set_pc(addr); break;
		case  7: if (CC7) set_pc(addr); break;
		case  8: if (CC8) set_pc(addr); break;
		case  9: if (CC9) set_pc(addr); break;
		case 10: if (CCA) set_pc(addr); break;
		case 11: if (CCB) set_pc(addr); break;
		case 12: if (CCC) set_pc(addr); break;
		case 13: if (CCD) set_pc(addr); break;
		case 14: if (CCE) set_pc(addr); break;
		case 15: if (CCF) set_pc(addr); break;
	}
}

static Z8K_AI void Z5F_0000_0000_addr(z8k_st_t *S)
{
	GET_ADDR(OP1);
	if (get_segmented_mode())
		PUSHL(SP, make_segmented_addr(m_pc));
	else
		PUSHW(SP, m_pc);
	set_pc(addr);
}

static Z8K_AI void Z5F_ddN0_0000_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	if (get_segmented_mode())
		PUSHL(SP, make_segmented_addr(m_pc));
	else
		PUSHW(SP, m_pc);
	addr = addr_add(addr, RW(dst));
	set_pc(addr);
}

static Z8K_AI void Z60_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = RDMEM_B(m_data, addr);
}

static Z8K_AI void Z60_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = RDMEM_B(m_data, addr);
}

static Z8K_AI void Z61_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = RDMEM_W(m_data, addr);
}

static Z8K_AI void Z61_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = RDMEM_W(m_data, addr);
}

static Z8K_AI void Z62_0000_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_B(m_data, addr, RDMEM_B(m_data, addr) & ~bit);
}

static Z8K_AI void Z62_ddN0_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, RDMEM_B(m_data, addr) & ~bit);
}

static Z8K_AI void Z63_0000_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_W(m_data, addr, RDMEM_W(m_data, addr) & ~bit);
}

static Z8K_AI void Z63_ddN0_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, RDMEM_W(m_data, addr) & ~bit);
}

static Z8K_AI void Z64_0000_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_B(m_data, addr, RDMEM_B(m_data, addr) | bit);
}

static Z8K_AI void Z64_ddN0_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, RDMEM_B(m_data, addr) | bit);
}

static Z8K_AI void Z65_0000_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_W(m_data, addr, RDMEM_W(m_data, addr) | bit);
}

static Z8K_AI void Z65_ddN0_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, RDMEM_W(m_data, addr) | bit);
}

static Z8K_AI void Z66_0000_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	if (RDMEM_B(m_data, addr) & bit) CLR_Z; else SET_Z;
}

static Z8K_AI void Z66_ddN0_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	if (RDMEM_B(m_data, addr) & bit) CLR_Z; else SET_Z;
}

static Z8K_AI void Z67_0000_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	if (RDMEM_W(m_data, addr) & bit) CLR_Z; else SET_Z;
}

static Z8K_AI void Z67_ddN0_imm4_addr(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	if (RDMEM_W(m_data, addr) & bit) CLR_Z; else SET_Z;
}

static Z8K_AI void Z68_0000_imm4m1_addr(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B(m_data, addr, INCB(RDMEM_B(m_data, addr), i4p1));
}

static Z8K_AI void Z68_ddN0_imm4m1_addr(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, INCB(RDMEM_B(m_data, addr), i4p1));
}

static Z8K_AI void Z69_0000_imm4m1_addr(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(m_data, addr, INCW(RDMEM_W(m_data, addr), i4p1));
}

static Z8K_AI void Z69_ddN0_imm4m1_addr(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, INCW(RDMEM_W(m_data, addr), i4p1));
}

static Z8K_AI void Z6A_0000_imm4m1_addr(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B(m_data, addr, DECB(RDMEM_B(m_data, addr), i4p1));
}

static Z8K_AI void Z6A_ddN0_imm4m1_addr(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, DECB(RDMEM_B(m_data, addr), i4p1));
}

static Z8K_AI void Z6B_0000_imm4m1_addr(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(m_data, addr, DECW(RDMEM_W(m_data, addr), i4p1));
}

static Z8K_AI void Z6B_ddN0_imm4m1_addr(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, DECW(RDMEM_W(m_data, addr), i4p1));
}

static Z8K_AI void Z6C_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	uint8_t tmp = RDMEM_B(m_data, addr);
	WRMEM_B(m_data, addr, RB(dst));
	RB(dst) = tmp;
}

static Z8K_AI void Z6C_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	uint8_t tmp;
	addr = addr_add(addr, RW(src));
	tmp = RDMEM_B(m_data, addr);
	WRMEM_B(m_data, addr, RB(dst));
	RB(dst) = tmp;
}

static Z8K_AI void Z6D_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	uint16_t tmp = RDMEM_W(m_data, addr);
	WRMEM_W(m_data, addr, RW(dst));
	RW(dst) = tmp;
}

static Z8K_AI void Z6D_ssN0_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	uint16_t tmp;
	addr = addr_add(addr, RW(src));
	tmp = RDMEM_W(m_data, addr);
	WRMEM_W(m_data, addr, RW(dst));
	RW(dst) = tmp;
}

static Z8K_AI void Z6E_0000_ssss_addr(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B(m_data,  addr, RB(src));
}

static Z8K_AI void Z6E_ddN0_ssss_addr(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, RB(src));
}

static Z8K_AI void Z6F_0000_ssss_addr(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(m_data,  addr, RW(src));
}

static Z8K_AI void Z6F_ddN0_ssss_addr(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, RW(src));
}

static Z8K_AI void Z70_ssN0_dddd_0000_xxxx_0000_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	RB(dst) = RDBX_B(src, RW(idx));
}

static Z8K_AI void Z71_ssN0_dddd_0000_xxxx_0000_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	RW(dst) = RDBX_W(src, RW(idx));
}

static Z8K_AI void Z72_ddN0_ssss_0000_xxxx_0000_0000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRBX_B(dst, RW(idx), RB(src));
}

static Z8K_AI void Z73_ddN0_ssss_0000_xxxx_0000_0000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRBX_W(dst, RW(idx), RW(src));
}

static Z8K_AI void Z74_ssN0_dddd_0000_xxxx_0000_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	if (get_segmented_mode()) {
		RL(dst) = RL(src);
	}
	else {
		RW(dst) = RW(src);
	}
	add_to_addr_reg(dst, RW(idx));
}

static Z8K_AI void Z75_ssN0_dddd_0000_xxxx_0000_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	RL(dst) = RDBX_L(src, RW(idx));
}

static Z8K_AI void Z76_0000_dddd_addr(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_ADDR_RAW(OP1);
	if (get_segmented_mode()) {
		RL(dst) = addr;
	}
	else {
		RW(dst) = addr;
	}
}

static Z8K_AI void Z76_ssN0_dddd_addr(z8k_st_t *S)
{//@@@
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR_RAW(OP1);
	uint16_t temp = RW(src);  // store src in case dst == src
	if (get_segmented_mode()) {
		RL(dst) = addr;
	}
	else {
		RW(dst) = addr;
	}
	add_to_addr_reg(dst, temp);
}

static Z8K_AI void Z77_ddN0_ssss_0000_xxxx_0000_0000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRBX_L(dst, RW(idx), RL(src));
}

static Z8K_AI void Z78_imm8(z8k_st_t *S)
{
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: rsvd78 $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

static Z8K_AI void Z79_0000_0000_addr(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_ADDR(OP1);
	uint16_t fcw;
	if (get_segmented_mode()) {
		fcw = RDMEM_W(m_data, addr + 2);
		set_pc(segmented_addr(RDMEM_L(m_data, addr + 4)));
	}
	else {
		fcw = RDMEM_W(m_data, addr);
		set_pc(RDMEM_W(m_data, (uint16_t)(addr + 2)));
	}
	CHANGE_FCW(fcw); /* check for user/system mode change */
}

static Z8K_AI void Z79_ssN0_0000_addr(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	uint16_t fcw;
	addr = addr_add(addr, RW(src));
	if (get_segmented_mode()) {
		fcw = RDMEM_W(m_data, addr + 2);
		set_pc(segmented_addr(RDMEM_L(m_data, addr + 4)));
	}
	else {
		fcw = RDMEM_W(m_data, addr);
		m_pc = RDMEM_W(m_data, (uint16_t)(addr + 2));
	}
	if ((fcw ^ m_fcw) & F_SEG) Z8K_NOLOG("ldps 3 (0x%05x): changing from %ssegmented mode to %ssegmented mode\n", m_pc, (fcw & F_SEG) ? "non-" : "", (fcw & F_SEG) ? "" : "non-");
	CHANGE_FCW(fcw); /* check for user/system mode change */
}

static Z8K_AI void Z7A_0000_0000(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	m_halt = true;
	m_icount_end();
}

static Z8K_AI void Z7B_0000_0000(z8k_st_t *S)
{
	uint16_t tag, fcw;
	CHECK_PRIVILEGED_INSTR();
	tag = POPW(SP);   /* get type tag */
	fcw = POPW(SP);   /* get m_fcw  */
	if (get_segmented_mode())
		set_pc(segmented_addr(POPL(SP)));
	else
		m_pc    = POPW(SP);   /* get m_pc   */
	CHANGE_FCW(fcw);       /* check for user/system mode change */
	Z8K_NOLOG("Z8K IRET tag $%04x, fcw $%04x, pc $%04x\n", tag, fcw, m_pc);
}

static Z8K_AI void Z7B_0000_1000(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	/* set mu-0 line */
}

static Z8K_AI void Z7B_0000_1001(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	/* reset mu-0 line */
}

static Z8K_AI void Z7B_0000_1010(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	/* test mu-I line */
}

static Z8K_AI void Z7B_dddd_1101(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	/* test mu-I line, invert cascade to mu-0  */
	if (m_mi)
	{
		CLR_Z;
		CLR_S;
		Z8K_NOLOG(CLEAR_LINE);
		return;
	}
	SET_Z;
	Z8K_NOLOG(ASSERT_LINE);
	if (m_mi)
	{
		SET_S;
	}
	else
	{
		CLR_S;
		Z8K_NOLOG(CLEAR_LINE);
	}
}

static Z8K_AI void Z7C_0000_00ii(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_IMM2(OP0,NIB3);
	uint16_t fcw = m_fcw;
	fcw &= (imm2 << 11) | 0xe7ff;
	CHANGE_FCW(fcw);
}

static Z8K_AI void Z7C_0000_01ii(z8k_st_t *S)
{
	CHECK_PRIVILEGED_INSTR();
	GET_IMM2(OP0,NIB3);
	uint16_t fcw = m_fcw;
	fcw |= ((~imm2) << 11) & 0x1800;
	CHANGE_FCW(fcw);
}

static Z8K_AI void Z7D_dddd_0ccc(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_IMM3(OP0,NIB3);
	GET_DST(OP0,NIB2);
	switch (imm3) {
		case 2:
			RW(dst) = m_fcw;
			break;
		case 3:
			RW(dst) = m_refresh;
			break;
		case 4:
			RW(dst) = m_psapseg;
			break;
		case 5:
			RW(dst) = m_psapoff;
			break;
		case 6:
			RW(dst) = m_nspseg;
			break;
		case 7:
			RW(dst) = m_nspoff;
			break;
		default:
			Z8K_NOLOG("Z8K LDCTL R%d,%d\n", dst, imm3);
	}
}

static Z8K_AI void Z7D_ssss_1ccc(z8k_st_t *S)
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_IMM3(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	switch (imm3) {
		case 2:
			{
				uint16_t fcw;
				fcw = RW(src);
				CHANGE_FCW(fcw); /* check for user/system mode change */
			}
			break;
		case 3:
			m_refresh = RW(src);
			break;
		case 4:
			m_psapseg = RW(src);
			break;
		case 5:
			m_psapoff = RW(src);
			break;
		case 6:
			m_nspseg = RW(src);
			break;
		case 7:
			m_nspoff = RW(src);
			break;
		default:
			Z8K_NOLOG("Z8K LDCTL %d,R%d\n", imm3, src);
	}
}

static Z8K_AI void Z7E_imm8(z8k_st_t *S)
{
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: rsvd7e $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

static Z8K_AI void Z7F_imm8(z8k_st_t *S)
{
	GET_IMM8(0);
	/* execute system call via IRQ */
	m_irq_req |= Z8000_SYSCALL;
	(void)imm8;
}

static Z8K_AI void Z80_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ADDB(RB(dst), RB(src));
}

static Z8K_AI void Z81_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ADDW(RW(dst), RW(src));
}

static Z8K_AI void Z82_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = SUBB(RB(dst), RB(src));
}

static Z8K_AI void Z83_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = SUBW(RW(dst), RW(src));
}

static Z8K_AI void Z84_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ORB(RB(dst), RB(src));
}

static Z8K_AI void Z85_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ORW(RW(dst), RW(src));
}

static Z8K_AI void Z86_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ANDB(RB(dst), RB(src));
}

static Z8K_AI void Z87_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ANDW(RW(dst), RW(src));
}

static Z8K_AI void Z88_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = XORB(RB(dst), RB(src));
}

static Z8K_AI void Z89_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = XORW(RW(dst), RW(src));
}

static Z8K_AI void Z8A_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPB(RB(dst), RB(src));
}

static Z8K_AI void Z8B_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPW(RW(dst), RW(src));
}

static Z8K_AI void Z8C_dddd_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	RB(dst) = COMB(RB(dst));
}

static Z8K_AI void Z8C_dddd_0010(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	RB(dst) = NEGB(RB(dst));
}

static Z8K_AI void Z8C_dddd_0100(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	TESTB(RB(dst));
}

static Z8K_AI void Z8C_dddd_0110(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	if (RB(dst) & S08) SET_S; else CLR_S;
	RB(dst) = 0xff;
}

static Z8K_AI void Z8C_dddd_0001(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	RB(dst) = m_fcw & 0xfc;
}

static Z8K_AI void Z8C_dddd_1000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	RB(dst) = 0;
}

static Z8K_AI void Z8C_dddd_1001(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	m_fcw &= ~0x00fc;
	m_fcw |= (RB(dst) & 0xfc);
}

static Z8K_AI void Z8D_0000_0111(z8k_st_t *S)
{
	/* nothing */
}

static Z8K_AI void Z8D_dddd_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	RW(dst) = COMW(RW(dst));
}

static Z8K_AI void Z8D_dddd_0010(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	RW(dst) = NEGW(RW(dst));
}

static Z8K_AI void Z8D_dddd_0100(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	TESTW(RW(dst));
}

static Z8K_AI void Z8D_dddd_0110(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	if (RW(dst) & S16) SET_S; else CLR_S;
	RW(dst) = 0xffff;
}

static Z8K_AI void Z8D_dddd_1000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	RW(dst) = 0;
}

static Z8K_AI void Z8D_imm4_0001(z8k_st_t *S)
{
	m_fcw |= m_op0 & 0x00f0;
}

static Z8K_AI void Z8D_imm4_0011(z8k_st_t *S)
{
	m_fcw &= ~(m_op0 & 0x00f0);
}

static Z8K_AI void Z8D_imm4_0101(z8k_st_t *S)
{
	m_fcw ^= (m_op0 & 0x00f0);
}

static Z8K_AI void Z8E_imm8(z8k_st_t *S)
{
	CHECK_EXT_INSTR();
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: ext8e  $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

static Z8K_AI void Z8F_imm8(z8k_st_t *S)
{
	CHECK_EXT_INSTR();
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: ext8f  $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

static Z8K_AI void Z90_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPL(RL(dst), RL(src));
}

static Z8K_AI void Z91_ddN0_ssss(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHL(dst, RL(src));
}

static Z8K_AI void Z92_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = SUBL(RL(dst), RL(src));
}

static Z8K_AI void Z93_ddN0_ssss(z8k_st_t *S)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHW(dst, RW(src));
}

static Z8K_AI void Z94_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = RL(src);
}

static Z8K_AI void Z95_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = POPL(src);
}

static Z8K_AI void Z96_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = ADDL(RL(dst), RL(src));
}

static Z8K_AI void Z97_ssN0_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = POPW(src);
}

static Z8K_AI void Z98_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RQ(dst) = MULTL(RQ(dst), RL(src));
}

static Z8K_AI void Z99_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = MULTW(RL(dst), RW(src));
}

static Z8K_AI void Z9A_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RQ(dst) = DIVL(RQ(dst), RL(src));
}

static Z8K_AI void Z9B_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = DIVW(RL(dst), RW(src));
}

static Z8K_AI void Z9C_dddd_1000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	CLR_ZS;
	if (!RL(dst)) SET_Z;
	else if (RL(dst) & S32) SET_S;
}

static Z8K_AI void Z9D_imm8(z8k_st_t *S)
{
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: rsvd9d $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

static Z8K_AI void Z9E_0000_cccc(z8k_st_t *S)
{
	GET_CCC(OP0,NIB3);
	if (get_segmented_mode()) {
		switch (cc) {
			case  0: if (CC0) set_pc(segmented_addr(POPL(SP))); break;
			case  1: if (CC1) set_pc(segmented_addr(POPL(SP))); break;
			case  2: if (CC2) set_pc(segmented_addr(POPL(SP))); break;
			case  3: if (CC3) set_pc(segmented_addr(POPL(SP))); break;
			case  4: if (CC4) set_pc(segmented_addr(POPL(SP))); break;
			case  5: if (CC5) set_pc(segmented_addr(POPL(SP))); break;
			case  6: if (CC6) set_pc(segmented_addr(POPL(SP))); break;
			case  7: if (CC7) set_pc(segmented_addr(POPL(SP))); break;
			case  8: if (CC8) set_pc(segmented_addr(POPL(SP))); break;
			case  9: if (CC9) set_pc(segmented_addr(POPL(SP))); break;
			case 10: if (CCA) set_pc(segmented_addr(POPL(SP))); break;
			case 11: if (CCB) set_pc(segmented_addr(POPL(SP))); break;
			case 12: if (CCC) set_pc(segmented_addr(POPL(SP))); break;
			case 13: if (CCD) set_pc(segmented_addr(POPL(SP))); break;
			case 14: if (CCE) set_pc(segmented_addr(POPL(SP))); break;
			case 15: if (CCF) set_pc(segmented_addr(POPL(SP))); break;
		}
	}
	else {
		switch (cc) {
			case  0: if (CC0) set_pc(POPW(SP)); break;
			case  1: if (CC1) set_pc(POPW(SP)); break;
			case  2: if (CC2) set_pc(POPW(SP)); break;
			case  3: if (CC3) set_pc(POPW(SP)); break;
			case  4: if (CC4) set_pc(POPW(SP)); break;
			case  5: if (CC5) set_pc(POPW(SP)); break;
			case  6: if (CC6) set_pc(POPW(SP)); break;
			case  7: if (CC7) set_pc(POPW(SP)); break;
			case  8: if (CC8) set_pc(POPW(SP)); break;
			case  9: if (CC9) set_pc(POPW(SP)); break;
			case 10: if (CCA) set_pc(POPW(SP)); break;
			case 11: if (CCB) set_pc(POPW(SP)); break;
			case 12: if (CCC) set_pc(POPW(SP)); break;
			case 13: if (CCD) set_pc(POPW(SP)); break;
			case 14: if (CCE) set_pc(POPW(SP)); break;
			case 15: if (CCF) set_pc(POPW(SP)); break;
		}
	}
}

static Z8K_AI void Z9F_imm8(z8k_st_t *S)
{
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: rsvd9f $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

static Z8K_AI void ZA0_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = RB(src);
}

static Z8K_AI void ZA1_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = RW(src);
}

static Z8K_AI void ZA2_dddd_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RB(dst) &= ~bit;
}

static Z8K_AI void ZA3_dddd_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RW(dst) &= ~bit;
}

static Z8K_AI void ZA4_dddd_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RB(dst) |= bit;
}

static Z8K_AI void ZA5_dddd_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RW(dst) |= bit;
}

static Z8K_AI void ZA6_dddd_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RB(dst) & bit) CLR_Z; else SET_Z;
}

static Z8K_AI void ZA7_dddd_imm4(z8k_st_t *S)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RW(dst) & bit) CLR_Z; else SET_Z;
}

static Z8K_AI void ZA8_dddd_imm4m1(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RB(dst) = INCB(RB(dst), i4p1);
}

static Z8K_AI void ZA9_dddd_imm4m1(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RW(dst) = INCW(RW(dst), i4p1);
}

static Z8K_AI void ZAA_dddd_imm4m1(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RB(dst) = DECB(RB(dst), i4p1);
}

static Z8K_AI void ZAB_dddd_imm4m1(z8k_st_t *S)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RW(dst) = DECW(RW(dst), i4p1);
}

static Z8K_AI void ZAC_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	uint8_t tmp = RB(src);
	RB(src) = RB(dst);
	RB(dst) = tmp;
}

static Z8K_AI void ZAD_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	uint16_t tmp = RW(src);
	RW(src) = RW(dst);
	RW(dst) = tmp;
}

static Z8K_AI void ZAE_dddd_cccc(z8k_st_t *S)
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	uint8_t tmp = RB(dst) & ~1;
	switch (cc) {
		case  0: if (CC0) tmp |= 1; break;
		case  1: if (CC1) tmp |= 1; break;
		case  2: if (CC2) tmp |= 1; break;
		case  3: if (CC3) tmp |= 1; break;
		case  4: if (CC4) tmp |= 1; break;
		case  5: if (CC5) tmp |= 1; break;
		case  6: if (CC6) tmp |= 1; break;
		case  7: if (CC7) tmp |= 1; break;
		case  8: if (CC8) tmp |= 1; break;
		case  9: if (CC9) tmp |= 1; break;
		case 10: if (CCA) tmp |= 1; break;
		case 11: if (CCB) tmp |= 1; break;
		case 12: if (CCC) tmp |= 1; break;
		case 13: if (CCD) tmp |= 1; break;
		case 14: if (CCE) tmp |= 1; break;
		case 15: if (CCF) tmp |= 1; break;
	}
	RB(dst) = tmp;
}

static Z8K_AI void ZAF_dddd_cccc(z8k_st_t *S)
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	uint16_t tmp = RW(dst) & ~1;
	switch (cc) {
		case  0: if (CC0) tmp |= 1; break;
		case  1: if (CC1) tmp |= 1; break;
		case  2: if (CC2) tmp |= 1; break;
		case  3: if (CC3) tmp |= 1; break;
		case  4: if (CC4) tmp |= 1; break;
		case  5: if (CC5) tmp |= 1; break;
		case  6: if (CC6) tmp |= 1; break;
		case  7: if (CC7) tmp |= 1; break;
		case  8: if (CC8) tmp |= 1; break;
		case  9: if (CC9) tmp |= 1; break;
		case 10: if (CCA) tmp |= 1; break;
		case 11: if (CCB) tmp |= 1; break;
		case 12: if (CCC) tmp |= 1; break;
		case 13: if (CCD) tmp |= 1; break;
		case 14: if (CCE) tmp |= 1; break;
		case 15: if (CCF) tmp |= 1; break;
	}
	RW(dst) = tmp;
}

static Z8K_AI void ZB0_dddd_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	uint8_t result;
	uint16_t idx = RB(dst);
	if (m_fcw & F_C)    idx |= 0x100;
	if (m_fcw & F_H)    idx |= 0x200;
	if (m_fcw & F_DA) idx |= 0x400;
	result = Z8000_dab[idx];
	CLR_CZS;
	CHK_XXXB_ZS;
	if (Z8000_dab[idx] & 0x100) SET_C;
	RB(dst) = result;
}

static Z8K_AI void ZB1_dddd_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	RW(dst) = (int16_t)(int8_t)RW(dst);
}

static Z8K_AI void ZB1_dddd_0111(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	RQ(dst) = (int64_t)(int32_t)RQ(dst);
}

static Z8K_AI void ZB1_dddd_1010(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	RL(dst) = (int32_t)(int16_t)RL(dst);
}

static Z8K_AI void ZB2_dddd_0001_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	if (imm8 & S08)
		RB(dst) = SRLB(RB(dst), -(int8_t)imm8);
	else
		RB(dst) = SLLB(RB(dst), imm8);
}

static Z8K_AI void ZB2_dddd_0011_0000_ssss_0000_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RB(dst) = SRLB(RB(dst), (int8_t)RW(src));
}

static Z8K_AI void ZB2_dddd_00I0(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RLB(RB(dst), imm1);
}

static Z8K_AI void ZB2_dddd_01I0(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RRB(RB(dst), imm1);
}

static Z8K_AI void ZB2_dddd_1001_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	if (imm8 & S08)
		RB(dst) = SRAB(RB(dst), -(int8_t)imm8);
	else
		RB(dst) = SLAB(RB(dst), imm8);
}

static Z8K_AI void ZB2_dddd_1011_0000_ssss_0000_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RB(dst) = SDAB(RB(dst), (int8_t) RW(src));
}

static Z8K_AI void ZB2_dddd_10I0(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RLCB(RB(dst), imm1);
}

static Z8K_AI void ZB2_dddd_11I0(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RRCB(RB(dst), imm1);
}

static Z8K_AI void ZB3_dddd_0001_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RW(dst) = SRLW(RW(dst), -(int16_t)imm16);
	else
		RW(dst) = SLLW(RW(dst), imm16);
}

static Z8K_AI void ZB3_dddd_0011_0000_ssss_0000_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RW(dst) = SDLW(RW(dst), (int8_t)RW(src));
}

static Z8K_AI void ZB3_dddd_00I0(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RLW(RW(dst), imm1);
}

static Z8K_AI void ZB3_dddd_0101_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RL(dst) = SRLL(RL(dst), -(int16_t)imm16);
	else
		RL(dst) = SLLL(RL(dst), imm16);
}

static Z8K_AI void ZB3_dddd_0111_0000_ssss_0000_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RL(dst) = SDLL(RL(dst), RW(src) & 0xff);
}

static Z8K_AI void ZB3_dddd_01I0(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RRW(RW(dst), imm1);
}

static Z8K_AI void ZB3_dddd_1001_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RW(dst) = SRAW(RW(dst), -(int16_t)imm16);
	else
		RW(dst) = SLAW(RW(dst), imm16);
}

static Z8K_AI void ZB3_dddd_1011_0000_ssss_0000_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RW(dst) = SDAW(RW(dst), (int8_t)RW(src));
}

static Z8K_AI void ZB3_dddd_10I0(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RLCW(RW(dst), imm1);
}

static Z8K_AI void ZB3_dddd_1101_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RL(dst) = SRAL(RL(dst), -(int16_t)imm16);
	else
		RL(dst) = SLAL(RL(dst), imm16);
}

static Z8K_AI void ZB3_dddd_1111_0000_ssss_0000_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RL(dst) = SDAL(RL(dst), RW(src) & 0xff);
}

static Z8K_AI void ZB3_dddd_11I0(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RRCW(RW(dst), imm1);
}

static Z8K_AI void ZB4_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ADCB(RB(dst), RB(src));
}

static Z8K_AI void ZB5_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ADCW(RW(dst), RW(src));
}

static Z8K_AI void ZB6_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = SBCB(RB(dst), RB(src));
}

static Z8K_AI void ZB7_ssss_dddd(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = SBCW(RW(dst), RW(src));
}

static Z8K_AI void ZB8_ddN0_0010_0000_rrrr_ssN0_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	uint8_t xlt = RDBX_B(src, RDIR_B(dst));
	RB(1) = xlt;  /* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

static Z8K_AI void ZB8_ddN0_0110_0000_rrrr_ssN0_1110(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	uint8_t xlt = RDBX_B(src, RDIR_B(dst));
	RB(1) = xlt;  /* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) {
		CLR_V;
		if (!xlt)
		m_pc -= 4;
	}
	else SET_V;
}

static Z8K_AI void ZB8_ddN0_1010_0000_rrrr_ssN0_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	uint8_t xlt = RDBX_B(src, RDIR_B(dst));
	RB(1) = xlt;  /* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

static Z8K_AI void ZB8_ddN0_1110_0000_rrrr_ssN0_1110(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	uint8_t xlt = RDBX_B(src, RDIR_B(dst));
	RB(1) = xlt;  /* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) {
		CLR_V;
		if (!xlt)
		m_pc -= 4;
	}
	else SET_V;
}

static Z8K_AI void ZB8_ddN0_0000_0000_rrrr_ssN0_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	int dstspace = dst == SP ? m_stack : m_data;
	uint32_t dstaddr = addr_from_reg(dst);
	uint8_t xlt = RDBX_B(src, RDMEM_B(dstspace, dstaddr));
	WRMEM_B(dstspace, dstaddr, xlt);
	RB(1) = xlt;  /* destroy RH1 */
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

static Z8K_AI void ZB8_ddN0_0100_0000_rrrr_ssN0_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	int dstspace = dst == SP ? m_stack : m_data;
	uint32_t dstaddr = addr_from_reg(dst);
	uint8_t xlt = RDBX_B(src, RDMEM_B(dstspace, dstaddr));
	WRMEM_B(dstspace, dstaddr, xlt);
	RB(1) = xlt;  /* destroy RH1 */
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZB8_ddN0_1000_0000_rrrr_ssN0_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	int dstspace = dst == SP ? m_stack : m_data;
	uint32_t dstaddr = addr_from_reg(dst);
	uint8_t xlt = RDBX_B(src, RDMEM_B(dstspace, dstaddr));
	WRMEM_B(dstspace, dstaddr, xlt);
	RB(1) = xlt;  /* destroy RH1 */
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

static Z8K_AI void ZB8_ddN0_1100_0000_rrrr_ssN0_0000(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	int dstspace = dst == SP ? m_stack : m_data;
	uint32_t dstaddr = addr_from_reg(dst);
	uint8_t xlt = RDBX_B(src, RDMEM_B(dstspace, dstaddr));
	WRMEM_B(dstspace, dstaddr, xlt);
	RB(1) = xlt;  /* destroy RH1 */
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZB9_imm8(z8k_st_t *S)
{
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: rsvdb9 $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
	(void)imm8;
}

static Z8K_AI void ZBA_ssN0_0000_0000_rrrr_dddd_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RB(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

static Z8K_AI void ZBA_ssN0_0001_0000_rrrr_ddN0_x000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);  /* repeat? */
	WRIR_B(dst, RDIR_B(src));
	add_to_addr_reg(src, 1);
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBA_ssN0_0010_0000_rrrr_ddN0_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RDIR_B(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 1);
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBA_ssN0_0100_0000_rrrr_dddd_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RB(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 1);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBA_ssN0_0110_0000_rrrr_ddN0_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RDIR_B(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 1);
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBA_ssN0_1000_0000_rrrr_dddd_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RB(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

static Z8K_AI void ZBA_ssN0_1001_0000_rrrr_ddN0_x000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_B(dst, RDIR_B(src));
	sub_from_addr_reg(src, 1);
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBA_ssN0_1010_0000_rrrr_ddN0_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RDIR_B(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 1);
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

static Z8K_AI void ZBA_ssN0_1100_0000_rrrr_dddd_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RB(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 1);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBA_ssN0_1110_0000_rrrr_ddN0_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RDIR_B(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 1);
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBB_ssN0_0000_0000_rrrr_dddd_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RW(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 2);
	if (--RW(cnt)) CLR_V; else SET_V;
}

static Z8K_AI void ZBB_ssN0_0001_0000_rrrr_ddN0_x000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDIR_W(src));
	add_to_addr_reg(src, 2);
	add_to_addr_reg(dst, 2);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBB_ssN0_0010_0000_rrrr_ddN0_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RDIR_W(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 2);
	add_to_addr_reg(dst, 2);
	if (--RW(cnt)) CLR_V; else SET_V;
}

static Z8K_AI void ZBB_ssN0_0100_0000_rrrr_dddd_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RW(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 2);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBB_ssN0_0110_0000_rrrr_ddN0_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RDIR_W(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 2);
	add_to_addr_reg(dst, 2);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBB_ssN0_1000_0000_rrrr_dddd_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RW(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 2);
	if (--RW(cnt)) CLR_V; else SET_V;
}

static Z8K_AI void ZBB_ssN0_1001_0000_rrrr_ddN0_x000(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDIR_W(src));
	sub_from_addr_reg(src, 2);
	sub_from_addr_reg(dst, 2);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBB_ssN0_1010_0000_rrrr_ddN0_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RDIR_W(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 2);
	sub_from_addr_reg(dst, 2);
	if (--RW(cnt)) CLR_V; else SET_V;
}

static Z8K_AI void ZBB_ssN0_1100_0000_rrrr_dddd_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RW(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 2);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBB_ssN0_1110_0000_rrrr_ddN0_cccc(z8k_st_t *S)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RDIR_W(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 2);
	sub_from_addr_reg(dst, 2);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

static Z8K_AI void ZBC_aaaa_bbbb(z8k_st_t *S)
{
	uint8_t b = m_op0 & 15;
	uint8_t a = (m_op0 >> 4) & 15;
	uint8_t tmp = RB(b);
	RB(a) = (RB(a) >> 4) | (RB(b) << 4);
	RB(b) = (RB(b) & 0xf0) | (tmp & 0x0f);
	if (RB(b)) CLR_Z; else SET_Z;
}

static Z8K_AI void ZBD_dddd_imm4(z8k_st_t *S)
{
	GET_DST(OP0,NIB2);
	GET_IMM4(OP0,NIB3);
	RW(dst) = imm4;
}

static Z8K_AI void ZBE_aaaa_bbbb(z8k_st_t *S)
{
	uint8_t b = m_op0 & 15;
	uint8_t a = (m_op0 >> 4) & 15;
	uint8_t tmp = RB(a);
	RB(a) = (RB(a) << 4) | (RB(b) & 0x0f);
	RB(b) = (RB(b) & 0xf0) | (tmp >> 4);
	if (RB(b)) CLR_Z; else SET_Z;
}

static Z8K_AI void ZBF_imm8(z8k_st_t *S)
{
	GET_IMM8(0);
	Z8K_NOLOG("Z8K %04x: rsvdbf $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
	(void)imm8;
}

static Z8K_AI void Z20_0000_dddd_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = imm8;
}

static Z8K_AI void ZC_dddd_imm8(z8k_st_t *S)
{
	GET_DST(OP0,NIB1);
	GET_IMM8(0);
	RB(dst) = imm8;
}

static Z8K_AI void ZD_dsp12(z8k_st_t *S)
{
	int16_t dsp12 = m_op0 & 0xfff;
	if (get_segmented_mode())
		PUSHL(SP, make_segmented_addr(m_pc));
	else
		PUSHW(SP, m_pc);
	dsp12 = (dsp12 & 2048) ? 4096 - 2 * (dsp12 & 2047) : -2 * (dsp12 & 2047);
	set_pc(addr_add(m_pc, dsp12));
}

static Z8K_AI void ZE_cccc_dsp8(z8k_st_t *S)
{
	GET_DSP8;
	GET_CCC(OP0,NIB1);
	switch (cc) {
		case  0: if (CC0) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  1: if (CC1) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  2: if (CC2) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  3: if (CC3) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  4: if (CC4) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  5: if (CC5) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  6: if (CC6) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  7: if (CC7) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  8: if (CC8) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  9: if (CC9) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  10: if (CCA) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  11: if (CCB) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  12: if (CCC) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  13: if (CCD) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  14: if (CCE) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  15: if (CCF) set_pc(addr_add(m_pc, dsp8 * 2)); break;
	}
}

static Z8K_AI void ZF_dddd_0dsp7(z8k_st_t *S)
{
	GET_DST(OP0,NIB1);
	GET_DSP7;
	RB(dst) -= 1;
	if (RB(dst)) {
		set_pc(addr_sub(m_pc, 2 * dsp7));
	}
}

static Z8K_AI void ZF_dddd_1dsp7(z8k_st_t *S)
{
	GET_DST(OP0,NIB1);
	GET_DSP7;
	RW(dst) -= 1;
	if (RW(dst)) {
		set_pc(addr_sub(m_pc, 2 * dsp7));
	}
}

// the instructions: X(number, function, cycles, H or C: Pole Position runs it
// or not); 0, the first row's (every opcode), is Z8K_OP0, the others are Z8K_OPS
#define Z8K_NOPS 411
#define Z8K_OP0(X) X(0, zinvalid, 4, C)
#define Z8K_OPS(X) \
	X(1, Z00_0000_dddd_imm8, 7, H) \
	X(2, Z00_ssN0_dddd, 7, H) \
	X(3, Z01_0000_dddd_imm16, 7, H) \
	X(4, Z01_ssN0_dddd, 7, H) \
	X(5, Z02_0000_dddd_imm8, 7, H) \
	X(6, Z02_ssN0_dddd, 7, C) \
	X(7, Z03_0000_dddd_imm16, 7, H) \
	X(8, Z03_ssN0_dddd, 7, H) \
	X(9, Z04_0000_dddd_imm8, 7, H) \
	X(10, Z04_ssN0_dddd, 7, C) \
	X(11, Z05_0000_dddd_imm16, 7, H) \
	X(12, Z05_ssN0_dddd, 7, C) \
	X(13, Z06_0000_dddd_imm8, 7, H) \
	X(14, Z06_ssN0_dddd, 7, C) \
	X(15, Z07_0000_dddd_imm16, 7, H) \
	X(16, Z07_ssN0_dddd, 7, C) \
	X(17, Z08_0000_dddd_imm8, 7, C) \
	X(18, Z08_ssN0_dddd, 7, C) \
	X(19, Z09_0000_dddd_imm16, 7, C) \
	X(20, Z09_ssN0_dddd, 7, H) \
	X(21, Z0A_0000_dddd_imm8, 7, H) \
	X(22, Z0A_ssN0_dddd, 7, C) \
	X(23, Z0B_0000_dddd_imm16, 7, H) \
	X(24, Z0B_ssN0_dddd, 7, H) \
	X(25, Z0C_ddN0_0000, 12, C) \
	X(26, Z0C_ddN0_0001_imm8, 11, C) \
	X(27, Z0C_ddN0_0010, 12, C) \
	X(28, Z0C_ddN0_0100, 8, H) \
	X(29, Z0C_ddN0_0101_imm8, 7, C) \
	X(30, Z0C_ddN0_0110, 11, C) \
	X(31, Z0C_ddN0_1000, 8, C) \
	X(32, Z0D_ddN0_0000, 12, C) \
	X(33, Z0D_ddN0_0001_imm16, 11, H) \
	X(34, Z0D_ddN0_0010, 12, C) \
	X(35, Z0D_ddN0_0100, 8, C) \
	X(36, Z0D_ddN0_0101_imm16, 11, H) \
	X(37, Z0D_ddN0_0110, 11, C) \
	X(38, Z0D_ddN0_1000, 8, C) \
	X(39, Z0D_ddN0_1001_imm16, 12, C) \
	X(40, Z0E_imm8, 10, C) \
	X(41, Z0F_imm8, 10, C) \
	X(42, Z10_0000_dddd_imm32, 14, H) \
	X(43, Z10_ssN0_dddd, 14, C) \
	X(44, Z11_ddN0_ssN0, 20, C) \
	X(45, Z12_0000_dddd_imm32, 14, H) \
	X(46, Z12_ssN0_dddd, 14, C) \
	X(47, Z13_ddN0_ssN0, 13, C) \
	X(48, Z14_0000_dddd_imm32, 11, H) \
	X(49, Z14_ssN0_dddd, 11, H) \
	X(50, Z15_ssN0_ddN0, 19, C) \
	X(51, Z16_0000_dddd_imm32, 14, H) \
	X(52, Z16_ssN0_dddd, 14, C) \
	X(53, Z17_ssN0_ddN0, 12, C) \
	X(54, Z18_00N0_dddd_imm32, 282, C) \
	X(55, Z18_ssN0_dddd, 282, C) \
	X(56, Z19_0000_dddd_imm16, 70, H) \
	X(57, Z19_ssN0_dddd, 70, C) \
	X(58, Z1A_0000_dddd_imm32, 744, C) \
	X(59, Z1A_ssN0_dddd, 744, C) \
	X(60, Z1B_0000_dddd_imm16, 107, H) \
	X(61, Z1B_ssN0_dddd, 107, C) \
	X(62, Z1C_ssN0_0001_0000_dddd_0000_nmin1, 11, H) \
	X(63, Z1C_ddN0_1000, 13, C) \
	X(64, Z1C_ddN0_1001_0000_ssss_0000_nmin1, 11, H) \
	X(65, Z1D_ddN0_ssss, 11, H) \
	X(66, Z1E_ddN0_cccc, 10, H) \
	X(67, Z1F_ddN0_0000, 10, H) \
	X(68, Z20_0000_dddd_imm8, 7, C) \
	X(69, Z20_ssN0_dddd, 7, H) \
	X(70, Z21_0000_dddd_imm16, 7, H) \
	X(71, Z21_ssN0_dddd, 7, H) \
	X(72, Z22_0000_ssss_0000_dddd_0000_0000, 10, C) \
	X(73, Z22_ddN0_imm4, 11, C) \
	X(74, Z23_0000_ssss_0000_dddd_0000_0000, 10, C) \
	X(75, Z23_ddN0_imm4, 11, H) \
	X(76, Z24_0000_ssss_0000_dddd_0000_0000, 10, C) \
	X(77, Z24_ddN0_imm4, 11, C) \
	X(78, Z25_0000_ssss_0000_dddd_0000_0000, 10, C) \
	X(79, Z25_ddN0_imm4, 11, C) \
	X(80, Z26_0000_ssss_0000_dddd_0000_0000, 10, C) \
	X(81, Z26_ddN0_imm4, 8, C) \
	X(82, Z27_0000_ssss_0000_dddd_0000_0000, 10, C) \
	X(83, Z27_ddN0_imm4, 8, H) \
	X(84, Z28_ddN0_imm4m1, 11, C) \
	X(85, Z29_ddN0_imm4m1, 11, C) \
	X(86, Z2A_ddN0_imm4m1, 11, C) \
	X(87, Z2B_ddN0_imm4m1, 11, H) \
	X(88, Z2C_ssN0_dddd, 12, C) \
	X(89, Z2D_ssN0_dddd, 12, H) \
	X(90, Z2E_ddN0_ssss, 8, H) \
	X(91, Z2F_ddN0_ssss, 8, H) \
	X(92, Z30_0000_dddd_dsp16, 14, C) \
	X(93, Z30_ssN0_dddd_imm16, 14, C) \
	X(94, Z31_0000_dddd_dsp16, 14, C) \
	X(95, Z31_ssN0_dddd_imm16, 14, C) \
	X(96, Z32_0000_ssss_dsp16, 14, C) \
	X(97, Z32_ddN0_ssss_imm16, 14, C) \
	X(98, Z33_0000_ssss_dsp16, 14, C) \
	X(99, Z33_ddN0_ssss_imm16, 14, C) \
	X(100, Z34_0000_dddd_dsp16, 15, C) \
	X(101, Z34_ssN0_dddd_imm16, 15, C) \
	X(102, Z35_0000_dddd_dsp16, 17, C) \
	X(103, Z35_ssN0_dddd_imm16, 17, C) \
	X(104, Z36_0000_0000, 2, C) \
	X(105, Z36_imm8, 10, C) \
	X(106, Z37_0000_ssss_dsp16, 17, C) \
	X(107, Z37_ddN0_ssss_imm16, 17, C) \
	X(108, Z38_imm8, 10, C) \
	X(109, Z39_ssN0_0000, 12, C) \
	X(110, Z3A_ssss_0000_0000_aaaa_dddd_x000, 21, C) \
	X(111, Z3A_ssss_0001_0000_aaaa_dddd_x000, 21, C) \
	X(112, Z3A_ssss_0010_0000_aaaa_dddd_x000, 21, C) \
	X(113, Z3A_ssss_0011_0000_aaaa_dddd_x000, 21, C) \
	X(114, Z3A_dddd_0100_imm16, 10, C) \
	X(115, Z3A_dddd_0101_imm16, 10, C) \
	X(116, Z3A_ssss_0110_imm16, 12, C) \
	X(117, Z3A_ssss_0111_imm16, 12, C) \
	X(118, Z3A_ssss_1000_0000_aaaa_dddd_x000, 21, C) \
	X(119, Z3A_ssss_1001_0000_aaaa_dddd_x000, 21, C) \
	X(120, Z3A_ssss_1010_0000_aaaa_dddd_x000, 21, C) \
	X(121, Z3A_ssss_1011_0000_aaaa_dddd_x000, 21, C) \
	X(122, Z3B_ssss_0000_0000_aaaa_dddd_x000, 21, C) \
	X(123, Z3B_ssss_0001_0000_aaaa_dddd_x000, 21, C) \
	X(124, Z3B_ssss_0010_0000_aaaa_dddd_x000, 21, C) \
	X(125, Z3B_ssss_0011_0000_aaaa_dddd_x000, 21, C) \
	X(126, Z3B_dddd_0100_imm16, 12, C) \
	X(127, Z3B_dddd_0101_imm16, 12, C) \
	X(128, Z3B_ssss_0110_imm16, 12, C) \
	X(129, Z3B_ssss_0111_imm16, 12, C) \
	X(130, Z3B_ssss_1000_0000_aaaa_dddd_x000, 21, C) \
	X(131, Z3B_ssss_1001_0000_aaaa_dddd_x000, 21, C) \
	X(132, Z3B_ssss_1010_0000_aaaa_dddd_x000, 21, C) \
	X(133, Z3B_ssss_1011_0000_aaaa_dddd_x000, 21, C) \
	X(134, Z3C_ssss_dddd, 10, C) \
	X(135, Z3D_ssss_dddd, 10, C) \
	X(136, Z3E_dddd_ssss, 12, C) \
	X(137, Z3F_dddd_ssss, 12, C) \
	X(138, Z40_0000_dddd_addr, 9, C) \
	X(139, Z40_ssN0_dddd_addr, 10, C) \
	X(140, Z41_0000_dddd_addr, 9, H) \
	X(141, Z41_ssN0_dddd_addr, 10, H) \
	X(142, Z42_0000_dddd_addr, 9, C) \
	X(143, Z42_ssN0_dddd_addr, 10, C) \
	X(144, Z43_0000_dddd_addr, 9, H) \
	X(145, Z43_ssN0_dddd_addr, 10, H) \
	X(146, Z44_0000_dddd_addr, 9, C) \
	X(147, Z44_ssN0_dddd_addr, 10, C) \
	X(148, Z45_0000_dddd_addr, 9, C) \
	X(149, Z45_ssN0_dddd_addr, 10, C) \
	X(150, Z46_0000_dddd_addr, 9, C) \
	X(151, Z46_ssN0_dddd_addr, 10, C) \
	X(152, Z47_0000_dddd_addr, 9, C) \
	X(153, Z47_ssN0_dddd_addr, 10, C) \
	X(154, Z48_0000_dddd_addr, 9, C) \
	X(155, Z48_ssN0_dddd_addr, 10, C) \
	X(156, Z49_0000_dddd_addr, 9, C) \
	X(157, Z49_ssN0_dddd_addr, 10, C) \
	X(158, Z4A_0000_dddd_addr, 9, C) \
	X(159, Z4A_ssN0_dddd_addr, 10, C) \
	X(160, Z4B_0000_dddd_addr, 9, H) \
	X(161, Z4B_ssN0_dddd_addr, 10, C) \
	X(162, Z4C_0000_0000_addr, 15, C) \
	X(163, Z4C_0000_0001_addr_imm8, 14, C) \
	X(164, Z4C_0000_0010_addr, 15, C) \
	X(165, Z4C_0000_0100_addr, 11, C) \
	X(166, Z4C_0000_0101_addr_imm8, 14, H) \
	X(167, Z4C_0000_0110_addr, 14, C) \
	X(168, Z4C_0000_1000_addr, 11, C) \
	X(169, Z4C_ddN0_0000_addr, 16, C) \
	X(170, Z4C_ddN0_0001_addr_imm8, 15, C) \
	X(171, Z4C_ddN0_0010_addr, 16, C) \
	X(172, Z4C_ddN0_0100_addr, 12, C) \
	X(173, Z4C_ddN0_0101_addr_imm8, 15, C) \
	X(174, Z4C_ddN0_0110_addr, 15, C) \
	X(175, Z4C_ddN0_1000_addr, 12, C) \
	X(176, Z4D_0000_0000_addr, 15, C) \
	X(177, Z4D_0000_0001_addr_imm16, 14, H) \
	X(178, Z4D_0000_0010_addr, 15, C) \
	X(179, Z4D_0000_0100_addr, 11, H) \
	X(180, Z4D_0000_0101_addr_imm16, 14, H) \
	X(181, Z4D_0000_0110_addr, 14, C) \
	X(182, Z4D_0000_1000_addr, 11, H) \
	X(183, Z4D_ddN0_0000_addr, 16, C) \
	X(184, Z4D_ddN0_0001_addr_imm16, 15, H) \
	X(185, Z4D_ddN0_0010_addr, 16, C) \
	X(186, Z4D_ddN0_0100_addr, 12, H) \
	X(187, Z4D_ddN0_0101_addr_imm16, 15, H) \
	X(188, Z4D_ddN0_0110_addr, 15, C) \
	X(189, Z4D_ddN0_1000_addr, 12, C) \
	X(190, Z4E_ddN0_ssN0_addr, 12, C) \
	X(191, Z50_0000_dddd_addr, 15, H) \
	X(192, Z50_ssN0_dddd_addr, 16, C) \
	X(193, Z51_ddN0_0000_addr, 21, C) \
	X(194, Z51_ddN0_ssN0_addr, 21, C) \
	X(195, Z52_0000_dddd_addr, 15, C) \
	X(196, Z52_ssN0_dddd_addr, 16, C) \
	X(197, Z53_ddN0_0000_addr, 14, C) \
	X(198, Z53_ddN0_ssN0_addr, 14, C) \
	X(199, Z54_0000_dddd_addr, 12, H) \
	X(200, Z54_ssN0_dddd_addr, 13, H) \
	X(201, Z55_ssN0_0000_addr, 23, C) \
	X(202, Z55_ssN0_ddN0_addr, 23, C) \
	X(203, Z56_0000_dddd_addr, 15, H) \
	X(204, Z56_ssN0_dddd_addr, 16, C) \
	X(205, Z57_ssN0_0000_addr, 16, C) \
	X(206, Z57_ssN0_ddN0_addr, 16, C) \
	X(207, Z58_0000_dddd_addr, 283, C) \
	X(208, Z58_ssN0_dddd_addr, 284, C) \
	X(209, Z59_0000_dddd_addr, 71, H) \
	X(210, Z59_ssN0_dddd_addr, 72, H) \
	X(211, Z5A_0000_dddd_addr, 745, C) \
	X(212, Z5A_ssN0_dddd_addr, 746, C) \
	X(213, Z5B_0000_dddd_addr, 108, C) \
	X(214, Z5B_ssN0_dddd_addr, 109, C) \
	X(215, Z5C_0000_0001_0000_dddd_0000_nmin1_addr, 14, H) \
	X(216, Z5C_0000_1000_addr, 16, C) \
	X(217, Z5C_0000_1001_0000_ssss_0000_nmin1_addr, 14, H) \
	X(218, Z5C_ssN0_0001_0000_dddd_0000_nmin1_addr, 15, C) \
	X(219, Z5C_ddN0_1000_addr, 17, C) \
	X(220, Z5C_ddN0_1001_0000_ssN0_0000_nmin1_addr, 15, C) \
	X(221, Z5D_0000_ssss_addr, 15, H) \
	X(222, Z5D_ddN0_ssss_addr, 14, H) \
	X(223, Z5E_0000_cccc_addr, 7, H) \
	X(224, Z5E_ddN0_cccc_addr, 8, C) \
	X(225, Z5F_0000_0000_addr, 12, H) \
	X(226, Z5F_ddN0_0000_addr, 13, C) \
	X(227, Z60_0000_dddd_addr, 9, H) \
	X(228, Z60_ssN0_dddd_addr, 10, H) \
	X(229, Z61_0000_dddd_addr, 9, H) \
	X(230, Z61_ssN0_dddd_addr, 10, H) \
	X(231, Z62_0000_imm4_addr, 13, C) \
	X(232, Z62_ddN0_imm4_addr, 14, C) \
	X(233, Z63_0000_imm4_addr, 13, H) \
	X(234, Z63_ddN0_imm4_addr, 14, C) \
	X(235, Z64_0000_imm4_addr, 13, C) \
	X(236, Z64_ddN0_imm4_addr, 14, C) \
	X(237, Z65_0000_imm4_addr, 13, H) \
	X(238, Z65_ddN0_imm4_addr, 14, C) \
	X(239, Z66_0000_imm4_addr, 10, C) \
	X(240, Z66_ddN0_imm4_addr, 11, C) \
	X(241, Z67_0000_imm4_addr, 10, H) \
	X(242, Z67_ddN0_imm4_addr, 11, H) \
	X(243, Z68_0000_imm4m1_addr, 13, C) \
	X(244, Z68_ddN0_imm4m1_addr, 14, C) \
	X(245, Z69_0000_imm4m1_addr, 13, H) \
	X(246, Z69_ddN0_imm4m1_addr, 14, C) \
	X(247, Z6A_0000_imm4m1_addr, 13, C) \
	X(248, Z6A_ddN0_imm4m1_addr, 14, C) \
	X(249, Z6B_0000_imm4m1_addr, 13, H) \
	X(250, Z6B_ddN0_imm4m1_addr, 14, H) \
	X(251, Z6C_0000_dddd_addr, 15, C) \
	X(252, Z6C_ssN0_dddd_addr, 16, C) \
	X(253, Z6D_0000_dddd_addr, 15, C) \
	X(254, Z6D_ssN0_dddd_addr, 16, C) \
	X(255, Z6E_0000_ssss_addr, 11, H) \
	X(256, Z6E_ddN0_ssss_addr, 11, H) \
	X(257, Z6F_0000_ssss_addr, 11, H) \
	X(258, Z6F_ddN0_ssss_addr, 12, H) \
	X(259, Z70_ssN0_dddd_0000_xxxx_0000_0000, 14, C) \
	X(260, Z71_ssN0_dddd_0000_xxxx_0000_0000, 14, H) \
	X(261, Z72_ddN0_ssss_0000_xxxx_0000_0000, 14, C) \
	X(262, Z73_ddN0_ssss_0000_xxxx_0000_0000, 14, C) \
	X(263, Z74_ssN0_dddd_0000_xxxx_0000_0000, 15, C) \
	X(264, Z75_ssN0_dddd_0000_xxxx_0000_0000, 17, C) \
	X(265, Z76_0000_dddd_addr, 12, H) \
	X(266, Z76_ssN0_dddd_addr, 13, C) \
	X(267, Z77_ddN0_ssss_0000_xxxx_0000_0000, 17, C) \
	X(268, Z78_imm8, 10, C) \
	X(269, Z79_0000_0000_addr, 16, C) \
	X(270, Z79_ssN0_0000_addr, 17, C) \
	X(271, Z7A_0000_0000, 8, C) \
	X(272, Z7B_0000_0000, 13, H) \
	X(273, Z7B_0000_1000, 5, C) \
	X(274, Z7B_0000_1001, 5, C) \
	X(275, Z7B_0000_1010, 7, C) \
	X(276, Z7B_dddd_1101, 12, C) \
	X(277, Z7C_0000_00ii, 7, C) \
	X(278, Z7C_0000_01ii, 7, H) \
	X(279, Z7D_dddd_0ccc, 7, C) \
	X(280, Z7D_ssss_1ccc, 7, H) \
	X(281, Z7E_imm8, 10, C) \
	X(282, Z7F_imm8, 33, C) \
	X(283, Z80_ssss_dddd, 4, H) \
	X(284, Z81_ssss_dddd, 4, H) \
	X(285, Z82_ssss_dddd, 4, H) \
	X(286, Z83_ssss_dddd, 4, H) \
	X(287, Z84_ssss_dddd, 4, H) \
	X(288, Z85_ssss_dddd, 4, H) \
	X(289, Z86_ssss_dddd, 4, C) \
	X(290, Z87_ssss_dddd, 4, H) \
	X(291, Z88_ssss_dddd, 4, C) \
	X(292, Z89_ssss_dddd, 4, C) \
	X(293, Z8A_ssss_dddd, 4, C) \
	X(294, Z8B_ssss_dddd, 4, H) \
	X(295, Z8C_dddd_0000, 7, C) \
	X(296, Z8C_dddd_0010, 7, C) \
	X(297, Z8C_dddd_0100, 7, H) \
	X(298, Z8C_dddd_0110, 7, C) \
	X(299, Z8C_dddd_0001, 7, C) \
	X(300, Z8C_dddd_1000, 7, H) \
	X(301, Z8C_dddd_1001, 7, C) \
	X(302, Z8D_dddd_0000, 7, H) \
	X(303, Z8D_imm4_0001, 7, C) \
	X(304, Z8D_dddd_0010, 7, H) \
	X(305, Z8D_imm4_0011, 7, C) \
	X(306, Z8D_dddd_0100, 7, H) \
	X(307, Z8D_imm4_0101, 7, C) \
	X(308, Z8D_dddd_0110, 7, C) \
	X(309, Z8D_0000_0111, 7, C) \
	X(310, Z8D_dddd_1000, 7, H) \
	X(311, Z8E_imm8, 10, C) \
	X(312, Z8F_imm8, 10, C) \
	X(313, Z90_ssss_dddd, 8, H) \
	X(314, Z91_ddN0_ssss, 12, H) \
	X(315, Z92_ssss_dddd, 8, H) \
	X(316, Z93_ddN0_ssss, 9, H) \
	X(317, Z94_ssss_dddd, 5, H) \
	X(318, Z95_ssN0_dddd, 12, H) \
	X(319, Z96_ssss_dddd, 8, H) \
	X(320, Z97_ssN0_dddd, 8, H) \
	X(321, Z98_ssss_dddd, 282, C) \
	X(322, Z99_ssss_dddd, 70, H) \
	X(323, Z9A_ssss_dddd, 744, C) \
	X(324, Z9B_ssss_dddd, 107, H) \
	X(325, Z9C_dddd_1000, 13, C) \
	X(326, Z9D_imm8, 10, C) \
	X(327, Z9E_0000_cccc, 10, H) \
	X(328, Z9F_imm8, 10, C) \
	X(329, ZA0_ssss_dddd, 3, H) \
	X(330, ZA1_ssss_dddd, 3, H) \
	X(331, ZA2_dddd_imm4, 4, C) \
	X(332, ZA3_dddd_imm4, 4, C) \
	X(333, ZA4_dddd_imm4, 4, C) \
	X(334, ZA5_dddd_imm4, 4, H) \
	X(335, ZA6_dddd_imm4, 4, H) \
	X(336, ZA7_dddd_imm4, 4, H) \
	X(337, ZA8_dddd_imm4m1, 4, H) \
	X(338, ZA9_dddd_imm4m1, 4, H) \
	X(339, ZAA_dddd_imm4m1, 4, H) \
	X(340, ZAB_dddd_imm4m1, 4, H) \
	X(341, ZAC_ssss_dddd, 6, C) \
	X(342, ZAD_ssss_dddd, 6, C) \
	X(343, ZAE_dddd_cccc, 5, C) \
	X(344, ZAF_dddd_cccc, 5, C) \
	X(345, ZB0_dddd_0000, 5, H) \
	X(346, ZB1_dddd_0000, 11, H) \
	X(347, ZB1_dddd_0111, 11, C) \
	X(348, ZB1_dddd_1010, 11, H) \
	X(349, ZB2_dddd_00I0, 6, C) \
	X(350, ZB2_dddd_0001_imm8, 13, H) \
	X(351, ZB2_dddd_0011_0000_ssss_0000_0000, 15, C) \
	X(352, ZB2_dddd_01I0, 6, C) \
	X(353, ZB2_dddd_10I0, 9, C) \
	X(354, ZB2_dddd_1001_imm8, 13, C) \
	X(355, ZB2_dddd_1011_0000_ssss_0000_0000, 15, C) \
	X(356, ZB2_dddd_11I0, 9, C) \
	X(357, ZB3_dddd_00I0, 6, C) \
	X(358, ZB3_dddd_0001_imm8, 13, H) \
	X(359, ZB3_dddd_0011_0000_ssss_0000_0000, 15, C) \
	X(360, ZB3_dddd_01I0, 6, C) \
	X(361, ZB3_dddd_0101_imm8, 13, H) \
	X(362, ZB3_dddd_0111_0000_ssss_0000_0000, 15, C) \
	X(363, ZB3_dddd_10I0, 6, C) \
	X(364, ZB3_dddd_1001_imm8, 13, H) \
	X(365, ZB3_dddd_1011_0000_ssss_0000_0000, 15, C) \
	X(366, ZB3_dddd_11I0, 6, C) \
	X(367, ZB3_dddd_1101_imm8, 13, H) \
	X(368, ZB3_dddd_1111_0000_ssss_0000_0000, 15, C) \
	X(369, ZB4_ssss_dddd, 5, H) \
	X(370, ZB5_ssss_dddd, 5, C) \
	X(371, ZB6_ssss_dddd, 5, C) \
	X(372, ZB7_ssss_dddd, 5, C) \
	X(373, ZB8_ddN0_0000_0000_rrrr_ssN0_0000, 25, C) \
	X(374, ZB8_ddN0_0010_0000_rrrr_ssN0_0000, 25, C) \
	X(375, ZB8_ddN0_0100_0000_rrrr_ssN0_0000, 25, C) \
	X(376, ZB8_ddN0_0110_0000_rrrr_ssN0_1110, 25, C) \
	X(377, ZB8_ddN0_1000_0000_rrrr_ssN0_0000, 25, C) \
	X(378, ZB8_ddN0_1010_0000_rrrr_ssN0_0000, 25, C) \
	X(379, ZB8_ddN0_1100_0000_rrrr_ssN0_0000, 25, C) \
	X(380, ZB8_ddN0_1110_0000_rrrr_ssN0_1110, 25, C) \
	X(381, ZB9_imm8, 10, C) \
	X(382, ZBA_ssN0_0000_0000_rrrr_dddd_cccc, 11, C) \
	X(383, ZBA_ssN0_0001_0000_rrrr_ddN0_x000, 11, C) \
	X(384, ZBA_ssN0_0010_0000_rrrr_ddN0_cccc, 11, C) \
	X(385, ZBA_ssN0_0100_0000_rrrr_dddd_cccc, 11, C) \
	X(386, ZBA_ssN0_0110_0000_rrrr_ddN0_cccc, 11, C) \
	X(387, ZBA_ssN0_1000_0000_rrrr_dddd_cccc, 11, C) \
	X(388, ZBA_ssN0_1001_0000_rrrr_ddN0_x000, 11, C) \
	X(389, ZBA_ssN0_1010_0000_rrrr_ddN0_cccc, 11, C) \
	X(390, ZBA_ssN0_1100_0000_rrrr_dddd_cccc, 11, C) \
	X(391, ZBA_ssN0_1110_0000_rrrr_ddN0_cccc, 11, C) \
	X(392, ZBB_ssN0_0000_0000_rrrr_dddd_cccc, 11, C) \
	X(393, ZBB_ssN0_0001_0000_rrrr_ddN0_x000, 11, C) \
	X(394, ZBB_ssN0_0010_0000_rrrr_ddN0_cccc, 11, C) \
	X(395, ZBB_ssN0_0100_0000_rrrr_dddd_cccc, 11, C) \
	X(396, ZBB_ssN0_0110_0000_rrrr_ddN0_cccc, 11, C) \
	X(397, ZBB_ssN0_1000_0000_rrrr_dddd_cccc, 11, C) \
	X(398, ZBB_ssN0_1001_0000_rrrr_ddN0_x000, 11, C) \
	X(399, ZBB_ssN0_1010_0000_rrrr_ddN0_cccc, 11, C) \
	X(400, ZBB_ssN0_1100_0000_rrrr_dddd_cccc, 11, C) \
	X(401, ZBB_ssN0_1110_0000_rrrr_ddN0_cccc, 11, C) \
	X(402, ZBC_aaaa_bbbb, 9, C) \
	X(403, ZBD_dddd_imm4, 5, C) \
	X(404, ZBE_aaaa_bbbb, 9, C) \
	X(405, ZBF_imm8, 10, C) \
	X(406, ZC_dddd_imm8, 5, H) \
	X(407, ZD_dsp12, 10, H) \
	X(408, ZE_cccc_dsp8, 6, H) \
	X(409, ZF_dddd_0dsp7, 11, C) \
	X(410, ZF_dddd_1dsp7, 11, H)

// the opcode table: rows applied in order, the later winning (init_tables);
// MAME's sizes are left out (only its disassembler uses them)
static const z8k_init_t z8k_table[] = {
	{ 0x0000, 0xffff, 1, 0 },
	{ 0x0000, 0x000f, 1, 1 },
	{ 0x0010, 0x00ff, 1, 2 },
	{ 0x0100, 0x010f, 1, 3 },
	{ 0x0110, 0x01ff, 1, 4 },
	{ 0x0200, 0x020f, 1, 5 },
	{ 0x0210, 0x02ff, 1, 6 },
	{ 0x0300, 0x030f, 1, 7 },
	{ 0x0310, 0x03ff, 1, 8 },
	{ 0x0400, 0x040f, 1, 9 },
	{ 0x0410, 0x04ff, 1, 10 },
	{ 0x0500, 0x050f, 1, 11 },
	{ 0x0510, 0x05ff, 1, 12 },
	{ 0x0600, 0x060f, 1, 13 },
	{ 0x0610, 0x06ff, 1, 14 },
	{ 0x0700, 0x070f, 1, 15 },
	{ 0x0710, 0x07ff, 1, 16 },
	{ 0x0800, 0x080f, 1, 17 },
	{ 0x0810, 0x08ff, 1, 18 },
	{ 0x0900, 0x090f, 1, 19 },
	{ 0x0910, 0x09ff, 1, 20 },
	{ 0x0a00, 0x0a0f, 1, 21 },
	{ 0x0a10, 0x0aff, 1, 22 },
	{ 0x0b00, 0x0b0f, 1, 23 },
	{ 0x0b10, 0x0bff, 1, 24 },
	{ 0x0c10, 0x0cf0, 16, 25 },
	{ 0x0c11, 0x0cf1, 16, 26 },
	{ 0x0c12, 0x0cf2, 16, 27 },
	{ 0x0c14, 0x0cf4, 16, 28 },
	{ 0x0c15, 0x0cf5, 16, 29 },
	{ 0x0c16, 0x0cf6, 16, 30 },
	{ 0x0c18, 0x0cf8, 16, 31 },
	{ 0x0d10, 0x0df0, 16, 32 },
	{ 0x0d11, 0x0df1, 16, 33 },
	{ 0x0d12, 0x0df2, 16, 34 },
	{ 0x0d14, 0x0df4, 16, 35 },
	{ 0x0d15, 0x0df5, 16, 36 },
	{ 0x0d16, 0x0df6, 16, 37 },
	{ 0x0d18, 0x0df8, 16, 38 },
	{ 0x0d19, 0x0df9, 16, 39 },
	{ 0x0e00, 0x0eff, 1, 40 },
	{ 0x0f00, 0x0fff, 1, 41 },
	{ 0x1000, 0x100f, 1, 42 },
	{ 0x1010, 0x10ff, 1, 43 },
	{ 0x1111, 0x11ff, 1, 44 },
	{ 0x1200, 0x120f, 1, 45 },
	{ 0x1210, 0x12ff, 1, 46 },
	{ 0x1311, 0x13ff, 1, 47 },
	{ 0x1400, 0x140f, 1, 48 },
	{ 0x1410, 0x14ff, 1, 49 },
	{ 0x1511, 0x15ff, 1, 50 },
	{ 0x1600, 0x160f, 1, 51 },
	{ 0x1610, 0x16ff, 1, 52 },
	{ 0x1711, 0x17ff, 1, 53 },
	{ 0x1800, 0x180f, 1, 54 },
	{ 0x1810, 0x18ff, 1, 55 },
	{ 0x1900, 0x190f, 1, 56 },
	{ 0x1910, 0x19ff, 1, 57 },
	{ 0x1a00, 0x1a0f, 1, 58 },
	{ 0x1a10, 0x1aff, 1, 59 },
	{ 0x1b00, 0x1b0f, 1, 60 },
	{ 0x1b10, 0x1bff, 1, 61 },
	{ 0x1c11, 0x1cf1, 16, 62 },
	{ 0x1c18, 0x1cf8, 16, 63 },
	{ 0x1c19, 0x1cf9, 16, 64 },
	{ 0x1d10, 0x1dff, 1, 65 },
	{ 0x1e10, 0x1eff, 1, 66 },
	{ 0x1f10, 0x1ff0, 16, 67 },
	{ 0x2000, 0x200f, 1, 68 },
	{ 0x2010, 0x20ff, 1, 69 },
	{ 0x2100, 0x210f, 1, 70 },
	{ 0x2110, 0x21ff, 1, 71 },
	{ 0x2200, 0x220f, 1, 72 },
	{ 0x2210, 0x22ff, 1, 73 },
	{ 0x2300, 0x230f, 1, 74 },
	{ 0x2310, 0x23ff, 1, 75 },
	{ 0x2400, 0x240f, 1, 76 },
	{ 0x2410, 0x24ff, 1, 77 },
	{ 0x2500, 0x250f, 1, 78 },
	{ 0x2510, 0x25ff, 1, 79 },
	{ 0x2600, 0x260f, 1, 80 },
	{ 0x2610, 0x26ff, 1, 81 },
	{ 0x2700, 0x270f, 1, 82 },
	{ 0x2710, 0x27ff, 1, 83 },
	{ 0x2810, 0x28ff, 1, 84 },
	{ 0x2910, 0x29ff, 1, 85 },
	{ 0x2a10, 0x2aff, 1, 86 },
	{ 0x2b10, 0x2bff, 1, 87 },
	{ 0x2c10, 0x2cff, 1, 88 },
	{ 0x2d10, 0x2dff, 1, 89 },
	{ 0x2e10, 0x2eff, 1, 90 },
	{ 0x2f10, 0x2fff, 1, 91 },
	{ 0x3000, 0x300f, 1, 92 },
	{ 0x3010, 0x30ff, 1, 93 },
	{ 0x3100, 0x310f, 1, 94 },
	{ 0x3110, 0x31ff, 1, 95 },
	{ 0x3200, 0x320f, 1, 96 },
	{ 0x3210, 0x32ff, 1, 97 },
	{ 0x3300, 0x330f, 1, 98 },
	{ 0x3310, 0x33ff, 1, 99 },
	{ 0x3400, 0x340f, 1, 100 },
	{ 0x3410, 0x34ff, 1, 101 },
	{ 0x3500, 0x350f, 1, 102 },
	{ 0x3510, 0x35ff, 1, 103 },
	{ 0x3600, 0x3600, 1, 104 },
	{ 0x3601, 0x36ff, 1, 105 },
	{ 0x3700, 0x370f, 1, 106 },
	{ 0x3710, 0x37ff, 1, 107 },
	{ 0x3800, 0x38ff, 1, 108 },
	{ 0x3910, 0x39f0, 16, 109 },
	{ 0x3a00, 0x3af0, 16, 110 },
	{ 0x3a01, 0x3af1, 16, 111 },
	{ 0x3a02, 0x3af2, 16, 112 },
	{ 0x3a03, 0x3af3, 16, 113 },
	{ 0x3a04, 0x3af4, 16, 114 },
	{ 0x3a05, 0x3af5, 16, 115 },
	{ 0x3a06, 0x3af6, 16, 116 },
	{ 0x3a07, 0x3af7, 16, 117 },
	{ 0x3a08, 0x3af8, 16, 118 },
	{ 0x3a09, 0x3af9, 16, 119 },
	{ 0x3a0a, 0x3afa, 16, 120 },
	{ 0x3a0b, 0x3afb, 16, 121 },
	{ 0x3b00, 0x3bf0, 16, 122 },
	{ 0x3b01, 0x3bf1, 16, 123 },
	{ 0x3b02, 0x3bf2, 16, 124 },
	{ 0x3b03, 0x3bf3, 16, 125 },
	{ 0x3b04, 0x3bf4, 16, 126 },
	{ 0x3b05, 0x3bf5, 16, 127 },
	{ 0x3b06, 0x3bf6, 16, 128 },
	{ 0x3b07, 0x3bf7, 16, 129 },
	{ 0x3b08, 0x3bf8, 16, 130 },
	{ 0x3b09, 0x3bf9, 16, 131 },
	{ 0x3b0a, 0x3bfa, 16, 132 },
	{ 0x3b0b, 0x3bfb, 16, 133 },
	{ 0x3c00, 0x3cff, 1, 134 },
	{ 0x3d00, 0x3dff, 1, 135 },
	{ 0x3e00, 0x3eff, 1, 136 },
	{ 0x3f00, 0x3fff, 1, 137 },
	{ 0x4000, 0x400f, 1, 138 },
	{ 0x4010, 0x40ff, 1, 139 },
	{ 0x4100, 0x410f, 1, 140 },
	{ 0x4110, 0x41ff, 1, 141 },
	{ 0x4200, 0x420f, 1, 142 },
	{ 0x4210, 0x42ff, 1, 143 },
	{ 0x4300, 0x430f, 1, 144 },
	{ 0x4310, 0x43ff, 1, 145 },
	{ 0x4400, 0x440f, 1, 146 },
	{ 0x4410, 0x44ff, 1, 147 },
	{ 0x4500, 0x450f, 1, 148 },
	{ 0x4510, 0x45ff, 1, 149 },
	{ 0x4600, 0x460f, 1, 150 },
	{ 0x4610, 0x46ff, 1, 151 },
	{ 0x4700, 0x470f, 1, 152 },
	{ 0x4710, 0x47ff, 1, 153 },
	{ 0x4800, 0x480f, 1, 154 },
	{ 0x4810, 0x48ff, 1, 155 },
	{ 0x4900, 0x490f, 1, 156 },
	{ 0x4910, 0x49ff, 1, 157 },
	{ 0x4a00, 0x4a0f, 1, 158 },
	{ 0x4a10, 0x4aff, 1, 159 },
	{ 0x4b00, 0x4b0f, 1, 160 },
	{ 0x4b10, 0x4bff, 1, 161 },
	{ 0x4c00, 0x4c00, 1, 162 },
	{ 0x4c01, 0x4c01, 1, 163 },
	{ 0x4c02, 0x4c02, 1, 164 },
	{ 0x4c04, 0x4c04, 1, 165 },
	{ 0x4c05, 0x4c05, 1, 166 },
	{ 0x4c06, 0x4c06, 1, 167 },
	{ 0x4c08, 0x4c08, 1, 168 },
	{ 0x4c10, 0x4cf0, 16, 169 },
	{ 0x4c11, 0x4cf1, 16, 170 },
	{ 0x4c12, 0x4cf2, 16, 171 },
	{ 0x4c14, 0x4cf4, 16, 172 },
	{ 0x4c15, 0x4cf5, 16, 173 },
	{ 0x4c16, 0x4cf6, 16, 174 },
	{ 0x4c18, 0x4cf8, 16, 175 },
	{ 0x4d00, 0x4d00, 1, 176 },
	{ 0x4d01, 0x4d01, 1, 177 },
	{ 0x4d02, 0x4d02, 1, 178 },
	{ 0x4d04, 0x4d04, 1, 179 },
	{ 0x4d05, 0x4d05, 1, 180 },
	{ 0x4d06, 0x4d06, 1, 181 },
	{ 0x4d08, 0x4d08, 1, 182 },
	{ 0x4d10, 0x4df0, 16, 183 },
	{ 0x4d11, 0x4df1, 16, 184 },
	{ 0x4d12, 0x4df2, 16, 185 },
	{ 0x4d14, 0x4df4, 16, 186 },
	{ 0x4d15, 0x4df5, 16, 187 },
	{ 0x4d16, 0x4df6, 16, 188 },
	{ 0x4d18, 0x4df8, 16, 189 },
	{ 0x4e11, 0x4ef0, 16, 190 },
	{ 0x5000, 0x500f, 1, 191 },
	{ 0x5010, 0x50ff, 1, 192 },
	{ 0x5110, 0x51f0, 16, 193 },
	{ 0x5111, 0x51f1, 16, 194 },
	{ 0x5112, 0x51f2, 16, 194 },
	{ 0x5113, 0x51f3, 16, 194 },
	{ 0x5114, 0x51f4, 16, 194 },
	{ 0x5115, 0x51f5, 16, 194 },
	{ 0x5116, 0x51f6, 16, 194 },
	{ 0x5117, 0x51f7, 16, 194 },
	{ 0x5118, 0x51f8, 16, 194 },
	{ 0x5119, 0x51f9, 16, 194 },
	{ 0x511a, 0x51fa, 16, 194 },
	{ 0x511b, 0x51fb, 16, 194 },
	{ 0x511c, 0x51fc, 16, 194 },
	{ 0x511d, 0x51fd, 16, 194 },
	{ 0x511e, 0x51fe, 16, 194 },
	{ 0x511f, 0x51ff, 16, 194 },
	{ 0x5200, 0x520f, 1, 195 },
	{ 0x5210, 0x52ff, 1, 196 },
	{ 0x5310, 0x53f0, 16, 197 },
	{ 0x5311, 0x53f1, 16, 198 },
	{ 0x5312, 0x53f2, 16, 198 },
	{ 0x5313, 0x53f3, 16, 198 },
	{ 0x5314, 0x53f4, 16, 198 },
	{ 0x5315, 0x53f5, 16, 198 },
	{ 0x5316, 0x53f6, 16, 198 },
	{ 0x5317, 0x53f7, 16, 198 },
	{ 0x5318, 0x53f8, 16, 198 },
	{ 0x5319, 0x53f9, 16, 198 },
	{ 0x531a, 0x53fa, 16, 198 },
	{ 0x531b, 0x53fb, 16, 198 },
	{ 0x531c, 0x53fc, 16, 198 },
	{ 0x531d, 0x53fd, 16, 198 },
	{ 0x531e, 0x53fe, 16, 198 },
	{ 0x531f, 0x53ff, 16, 198 },
	{ 0x5400, 0x540f, 1, 199 },
	{ 0x5410, 0x54ff, 1, 200 },
	{ 0x5510, 0x55f0, 16, 201 },
	{ 0x5511, 0x55f1, 16, 202 },
	{ 0x5512, 0x55f2, 16, 202 },
	{ 0x5513, 0x55f3, 16, 202 },
	{ 0x5514, 0x55f4, 16, 202 },
	{ 0x5515, 0x55f5, 16, 202 },
	{ 0x5516, 0x55f6, 16, 202 },
	{ 0x5517, 0x55f7, 16, 202 },
	{ 0x5518, 0x55f8, 16, 202 },
	{ 0x5519, 0x55f9, 16, 202 },
	{ 0x551a, 0x55fa, 16, 202 },
	{ 0x551b, 0x55fb, 16, 202 },
	{ 0x551c, 0x55fc, 16, 202 },
	{ 0x551d, 0x55fd, 16, 202 },
	{ 0x551e, 0x55fe, 16, 202 },
	{ 0x551f, 0x55ff, 16, 202 },
	{ 0x5600, 0x560f, 1, 203 },
	{ 0x5610, 0x56ff, 1, 204 },
	{ 0x5710, 0x57f0, 16, 205 },
	{ 0x5711, 0x57f1, 16, 206 },
	{ 0x5712, 0x57f2, 16, 206 },
	{ 0x5713, 0x57f3, 16, 206 },
	{ 0x5714, 0x57f4, 16, 206 },
	{ 0x5715, 0x57f5, 16, 206 },
	{ 0x5716, 0x57f6, 16, 206 },
	{ 0x5717, 0x57f7, 16, 206 },
	{ 0x5718, 0x57f8, 16, 206 },
	{ 0x5719, 0x57f9, 16, 206 },
	{ 0x571a, 0x57fa, 16, 206 },
	{ 0x571b, 0x57fb, 16, 206 },
	{ 0x571c, 0x57fc, 16, 206 },
	{ 0x571d, 0x57fd, 16, 206 },
	{ 0x571e, 0x57fe, 16, 206 },
	{ 0x571f, 0x57ff, 16, 206 },
	{ 0x5800, 0x580f, 1, 207 },
	{ 0x5810, 0x58ff, 1, 208 },
	{ 0x5900, 0x590f, 1, 209 },
	{ 0x5910, 0x59ff, 1, 210 },
	{ 0x5a00, 0x5a0f, 1, 211 },
	{ 0x5a10, 0x5aff, 1, 212 },
	{ 0x5b00, 0x5b0f, 1, 213 },
	{ 0x5b10, 0x5bff, 1, 214 },
	{ 0x5c01, 0x5c01, 1, 215 },
	{ 0x5c08, 0x5c08, 1, 216 },
	{ 0x5c09, 0x5c09, 1, 217 },
	{ 0x5c11, 0x5cf1, 16, 218 },
	{ 0x5c18, 0x5cf8, 16, 219 },
	{ 0x5c19, 0x5cf9, 16, 220 },
	{ 0x5d00, 0x5d0f, 1, 221 },
	{ 0x5d10, 0x5dff, 1, 222 },
	{ 0x5e00, 0x5e0f, 1, 223 },
	{ 0x5e10, 0x5eff, 1, 224 },
	{ 0x5f00, 0x5f00, 1, 225 },
	{ 0x5f10, 0x5ff0, 16, 226 },
	{ 0x6000, 0x600f, 1, 227 },
	{ 0x6010, 0x60ff, 1, 228 },
	{ 0x6100, 0x610f, 1, 229 },
	{ 0x6110, 0x61ff, 1, 230 },
	{ 0x6200, 0x620f, 1, 231 },
	{ 0x6210, 0x62ff, 1, 232 },
	{ 0x6300, 0x630f, 1, 233 },
	{ 0x6310, 0x63ff, 1, 234 },
	{ 0x6400, 0x640f, 1, 235 },
	{ 0x6410, 0x64ff, 1, 236 },
	{ 0x6500, 0x650f, 1, 237 },
	{ 0x6510, 0x65ff, 1, 238 },
	{ 0x6600, 0x660f, 1, 239 },
	{ 0x6610, 0x66ff, 1, 240 },
	{ 0x6700, 0x670f, 1, 241 },
	{ 0x6710, 0x67ff, 1, 242 },
	{ 0x6800, 0x680f, 1, 243 },
	{ 0x6810, 0x68ff, 1, 244 },
	{ 0x6900, 0x690f, 1, 245 },
	{ 0x6910, 0x69ff, 1, 246 },
	{ 0x6a00, 0x6a0f, 1, 247 },
	{ 0x6a10, 0x6aff, 1, 248 },
	{ 0x6b00, 0x6b0f, 1, 249 },
	{ 0x6b10, 0x6bff, 1, 250 },
	{ 0x6c00, 0x6c0f, 1, 251 },
	{ 0x6c10, 0x6cff, 1, 252 },
	{ 0x6d00, 0x6d0f, 1, 253 },
	{ 0x6d10, 0x6dff, 1, 254 },
	{ 0x6e00, 0x6e0f, 1, 255 },
	{ 0x6e10, 0x6eff, 1, 256 },
	{ 0x6f00, 0x6f0f, 1, 257 },
	{ 0x6f10, 0x6fff, 1, 258 },
	{ 0x7010, 0x70ff, 1, 259 },
	{ 0x7110, 0x71ff, 1, 260 },
	{ 0x7210, 0x72ff, 1, 261 },
	{ 0x7310, 0x73ff, 1, 262 },
	{ 0x7410, 0x74ff, 1, 263 },
	{ 0x7510, 0x75ff, 1, 264 },
	{ 0x7600, 0x760f, 1, 265 },
	{ 0x7610, 0x76ff, 1, 266 },
	{ 0x7710, 0x77ff, 1, 267 },
	{ 0x7800, 0x78ff, 1, 268 },
	{ 0x7900, 0x7900, 1, 269 },
	{ 0x7910, 0x79f0, 16, 270 },
	{ 0x7a00, 0x7a00, 1, 271 },
	{ 0x7b00, 0x7b00, 1, 272 },
	{ 0x7b08, 0x7b08, 1, 273 },
	{ 0x7b09, 0x7b09, 1, 274 },
	{ 0x7b0a, 0x7b0a, 1, 275 },
	{ 0x7b0d, 0x7bfd, 16, 276 },
	{ 0x7c00, 0x7c03, 1, 277 },
	{ 0x7c04, 0x7c07, 1, 278 },
	{ 0x7d00, 0x7df0, 16, 279 },
	{ 0x7d01, 0x7df1, 16, 279 },
	{ 0x7d02, 0x7df2, 16, 279 },
	{ 0x7d03, 0x7df3, 16, 279 },
	{ 0x7d04, 0x7df4, 16, 279 },
	{ 0x7d05, 0x7df5, 16, 279 },
	{ 0x7d06, 0x7df6, 16, 279 },
	{ 0x7d07, 0x7df7, 16, 279 },
	{ 0x7d08, 0x7df8, 16, 280 },
	{ 0x7d09, 0x7df9, 16, 280 },
	{ 0x7d0a, 0x7dfa, 16, 280 },
	{ 0x7d0b, 0x7dfb, 16, 280 },
	{ 0x7d0c, 0x7dfc, 16, 280 },
	{ 0x7d0d, 0x7dfd, 16, 280 },
	{ 0x7d0e, 0x7dfe, 16, 280 },
	{ 0x7d0f, 0x7dff, 16, 280 },
	{ 0x7e00, 0x7eff, 1, 281 },
	{ 0x7f00, 0x7fff, 1, 282 },
	{ 0x8000, 0x80ff, 1, 283 },
	{ 0x8100, 0x81ff, 1, 284 },
	{ 0x8200, 0x82ff, 1, 285 },
	{ 0x8300, 0x83ff, 1, 286 },
	{ 0x8400, 0x84ff, 1, 287 },
	{ 0x8500, 0x85ff, 1, 288 },
	{ 0x8600, 0x86ff, 1, 289 },
	{ 0x8700, 0x87ff, 1, 290 },
	{ 0x8800, 0x88ff, 1, 291 },
	{ 0x8900, 0x89ff, 1, 292 },
	{ 0x8a00, 0x8aff, 1, 293 },
	{ 0x8b00, 0x8bff, 1, 294 },
	{ 0x8c00, 0x8cf0, 16, 295 },
	{ 0x8c02, 0x8cf2, 16, 296 },
	{ 0x8c04, 0x8cf4, 16, 297 },
	{ 0x8c06, 0x8cf6, 16, 298 },
	{ 0x8c01, 0x8cf1, 16, 299 },
	{ 0x8c08, 0x8cf8, 16, 300 },
	{ 0x8c09, 0x8cf9, 16, 301 },
	{ 0x8d00, 0x8df0, 16, 302 },
	{ 0x8d01, 0x8df1, 16, 303 },
	{ 0x8d02, 0x8df2, 16, 304 },
	{ 0x8d03, 0x8df3, 16, 305 },
	{ 0x8d04, 0x8df4, 16, 306 },
	{ 0x8d05, 0x8df5, 16, 307 },
	{ 0x8d06, 0x8df6, 16, 308 },
	{ 0x8d07, 0x8d07, 1, 309 },
	{ 0x8d08, 0x8df8, 16, 310 },
	{ 0x8e00, 0x8eff, 1, 311 },
	{ 0x8f00, 0x8fff, 1, 312 },
	{ 0x9000, 0x90ff, 1, 313 },
	{ 0x9110, 0x91ff, 1, 314 },
	{ 0x9200, 0x92ff, 1, 315 },
	{ 0x9310, 0x93ff, 1, 316 },
	{ 0x9400, 0x94ff, 1, 317 },
	{ 0x9510, 0x95ff, 1, 318 },
	{ 0x9600, 0x96ff, 1, 319 },
	{ 0x9710, 0x97ff, 1, 320 },
	{ 0x9800, 0x98ff, 1, 321 },
	{ 0x9900, 0x99ff, 1, 322 },
	{ 0x9a00, 0x9aff, 1, 323 },
	{ 0x9b00, 0x9bff, 1, 324 },
	{ 0x9c00, 0x9cf8, 8, 325 },
	{ 0x9d00, 0x9dff, 1, 326 },
	{ 0x9e00, 0x9e0f, 1, 327 },
	{ 0x9f00, 0x9fff, 1, 328 },
	{ 0xa000, 0xa0ff, 1, 329 },
	{ 0xa100, 0xa1ff, 1, 330 },
	{ 0xa200, 0xa2ff, 1, 331 },
	{ 0xa300, 0xa3ff, 1, 332 },
	{ 0xa400, 0xa4ff, 1, 333 },
	{ 0xa500, 0xa5ff, 1, 334 },
	{ 0xa600, 0xa6ff, 1, 335 },
	{ 0xa700, 0xa7ff, 1, 336 },
	{ 0xa800, 0xa8ff, 1, 337 },
	{ 0xa900, 0xa9ff, 1, 338 },
	{ 0xaa00, 0xaaff, 1, 339 },
	{ 0xab00, 0xabff, 1, 340 },
	{ 0xac00, 0xacff, 1, 341 },
	{ 0xad00, 0xadff, 1, 342 },
	{ 0xae00, 0xaeff, 1, 343 },
	{ 0xaf00, 0xafff, 1, 344 },
	{ 0xb000, 0xb0f0, 16, 345 },
	{ 0xb100, 0xb1f0, 16, 346 },
	{ 0xb107, 0xb1f7, 16, 347 },
	{ 0xb10a, 0xb1fa, 16, 348 },
	{ 0xb200, 0xb2f0, 16, 349 },
	{ 0xb201, 0xb2f1, 16, 350 },
	{ 0xb202, 0xb2f2, 16, 349 },
	{ 0xb203, 0xb2f3, 16, 351 },
	{ 0xb204, 0xb2f4, 16, 352 },
	{ 0xb206, 0xb2f6, 16, 352 },
	{ 0xb208, 0xb2f8, 16, 353 },
	{ 0xb209, 0xb2f9, 16, 354 },
	{ 0xb20a, 0xb2fa, 16, 353 },
	{ 0xb20b, 0xb2fb, 16, 355 },
	{ 0xb20c, 0xb2fc, 16, 356 },
	{ 0xb20e, 0xb2fe, 16, 356 },
	{ 0xb300, 0xb3f0, 16, 357 },
	{ 0xb301, 0xb3f1, 16, 358 },
	{ 0xb302, 0xb3f2, 16, 357 },
	{ 0xb303, 0xb3f3, 16, 359 },
	{ 0xb304, 0xb3f4, 16, 360 },
	{ 0xb305, 0xb3f5, 16, 361 },
	{ 0xb306, 0xb3f6, 16, 360 },
	{ 0xb307, 0xb3f7, 16, 362 },
	{ 0xb308, 0xb3f8, 16, 363 },
	{ 0xb309, 0xb3f9, 16, 364 },
	{ 0xb30a, 0xb3fa, 16, 363 },
	{ 0xb30b, 0xb3fb, 16, 365 },
	{ 0xb30c, 0xb3fc, 16, 366 },
	{ 0xb30d, 0xb3fd, 16, 367 },
	{ 0xb30e, 0xb3fe, 16, 366 },
	{ 0xb30f, 0xb3ff, 16, 368 },
	{ 0xb400, 0xb4ff, 1, 369 },
	{ 0xb500, 0xb5ff, 1, 370 },
	{ 0xb600, 0xb6ff, 1, 371 },
	{ 0xb700, 0xb7ff, 1, 372 },
	{ 0xb810, 0xb8f0, 16, 373 },
	{ 0xb812, 0xb8f2, 16, 374 },
	{ 0xb814, 0xb8f4, 16, 375 },
	{ 0xb816, 0xb8f6, 16, 376 },
	{ 0xb818, 0xb8f8, 16, 377 },
	{ 0xb81a, 0xb8fa, 16, 378 },
	{ 0xb81c, 0xb8fc, 16, 379 },
	{ 0xb81e, 0xb8fe, 16, 380 },
	{ 0xb900, 0xb9ff, 16, 381 },
	{ 0xba10, 0xbaf0, 16, 382 },
	{ 0xba11, 0xbaf1, 16, 383 },
	{ 0xba12, 0xbaf2, 16, 384 },
	{ 0xba14, 0xbaf4, 16, 385 },
	{ 0xba16, 0xbaf6, 16, 386 },
	{ 0xba18, 0xbaf8, 16, 387 },
	{ 0xba19, 0xbaf9, 16, 388 },
	{ 0xba1a, 0xbafa, 16, 389 },
	{ 0xba1c, 0xbafc, 16, 390 },
	{ 0xba1e, 0xbafe, 16, 391 },
	{ 0xbb10, 0xbbf0, 16, 392 },
	{ 0xbb11, 0xbbf1, 16, 393 },
	{ 0xbb12, 0xbbf2, 16, 394 },
	{ 0xbb14, 0xbbf4, 16, 395 },
	{ 0xbb16, 0xbbf6, 16, 396 },
	{ 0xbb18, 0xbbf8, 16, 397 },
	{ 0xbb19, 0xbbf9, 16, 398 },
	{ 0xbb1a, 0xbbfa, 16, 399 },
	{ 0xbb1c, 0xbbfc, 16, 400 },
	{ 0xbb1e, 0xbbfe, 16, 401 },
	{ 0xbc00, 0xbcff, 1, 402 },
	{ 0xbd00, 0xbdff, 1, 403 },
	{ 0xbe00, 0xbeff, 1, 404 },
	{ 0xbf00, 0xbfff, 1, 405 },
	{ 0xc000, 0xcfff, 1, 406 },
	{ 0xd000, 0xdfff, 1, 407 },
	{ 0xe000, 0xefff, 1, 408 },
	{ 0xf000, 0xf07f, 1, 409 },
	{ 0xf100, 0xf17f, 1, 409 },
	{ 0xf200, 0xf27f, 1, 409 },
	{ 0xf300, 0xf37f, 1, 409 },
	{ 0xf400, 0xf47f, 1, 409 },
	{ 0xf500, 0xf57f, 1, 409 },
	{ 0xf600, 0xf67f, 1, 409 },
	{ 0xf700, 0xf77f, 1, 409 },
	{ 0xf800, 0xf87f, 1, 409 },
	{ 0xf900, 0xf97f, 1, 409 },
	{ 0xfa00, 0xfa7f, 1, 409 },
	{ 0xfb00, 0xfb7f, 1, 409 },
	{ 0xfc00, 0xfc7f, 1, 409 },
	{ 0xfd00, 0xfd7f, 1, 409 },
	{ 0xfe00, 0xfe7f, 1, 409 },
	{ 0xff00, 0xff7f, 1, 409 },
	{ 0xf080, 0xf0ff, 1, 410 },
	{ 0xf180, 0xf1ff, 1, 410 },
	{ 0xf280, 0xf2ff, 1, 410 },
	{ 0xf380, 0xf3ff, 1, 410 },
	{ 0xf480, 0xf4ff, 1, 410 },
	{ 0xf580, 0xf5ff, 1, 410 },
	{ 0xf680, 0xf6ff, 1, 410 },
	{ 0xf780, 0xf7ff, 1, 410 },
	{ 0xf880, 0xf8ff, 1, 410 },
	{ 0xf980, 0xf9ff, 1, 410 },
	{ 0xfa80, 0xfaff, 1, 410 },
	{ 0xfb80, 0xfbff, 1, 410 },
	{ 0xfc80, 0xfcff, 1, 410 },
	{ 0xfd80, 0xfdff, 1, 410 },
	{ 0xfe80, 0xfeff, 1, 410 },
	{ 0xff80, 0xffff, 1, 410 },
	{ 0, 0, 0, 0 }
};
