#!/usr/bin/env python3
# license:BSD-3-Clause
# copyright-holders:Juergen Buchmueller, Ernesto Corvi
"""z8kgen.py MAME_Z8000_DIR OUT.H -- MAME's Z8002 as C.

MAME 0.288 describes the Z8000 family in src/devices/cpu/z8000 (Juergen
Buchmueller and Ernesto Corvi, BSD-3-Clause): z8000ops.hxx (the
instructions and their helpers, as z8002_device members), z8000tbl.hxx
(the opcode ranges, sizes and cycle counts), z8000cpu.h (the macros) and
z8000dab.h (DAB's table).
This turns them into C for z8k.c, which #includes the output:

  - each z8002_device member function becomes a static always-inline C
    function; the z8001_device ones are left out (the Z8002 has no segments)
  - z8k.c runs them inside z8k_run (or the ones not in HOT, below, out of
    line), with the CPU's hot state (pc, icount, fcw, the opcode words) in
    registers or a local struct S, which every function takes first: an
    instruction is NAME(S); a helper is NAME_(S, ...), with a macro
    NAME(...) that adds the S, so MAME's bodies stay as they are written
  - the members (m_pc, m_fcw, m_regs ...) stay as they are named (z8k.c
    #defines each name to its place), but for m_op[0], the opcode word,
    which is m_op0, and HALT's "if (m_icount > 0) m_icount = 0;", which is
    m_icount_end() (z8k.c's icount is biased)
  - the flag macros CHK_XXX?_ZS and CHK_ADD?_V, CHK_SUB?_V set the same
    flags without branches (shifts of the same bits; z8k.c's Z8K_ZERO)
  - the address spaces become numbers (polepos maps program, data and
    stack to one space), logging and printf go, m_mo_out goes (unbound)
  - the instructions get numbers in the order the table first names them
    (Z8K_OP0, Z8K_OPS: number, function, cycles, H or C; each function has
    one cycle count; H is in HOT below, C is not),
    and the table becomes a C array of (first, last, step, instruction)
    rows, applied in order as MAME's init_tables() does
"""
import re, sys

# The instructions Pole Position runs (the 131 that test/pp_host's play and
# attract scripts run, of 411): z8k.c keeps these in z8k_run; the others
# are functions of their own there, smaller and a little slower.
HOT = set("""
Z00_0000_dddd_imm8 Z00_ssN0_dddd Z01_0000_dddd_imm16 Z01_ssN0_dddd
Z02_0000_dddd_imm8 Z03_0000_dddd_imm16 Z03_ssN0_dddd Z04_0000_dddd_imm8
Z05_0000_dddd_imm16 Z06_0000_dddd_imm8 Z07_0000_dddd_imm16 Z09_ssN0_dddd
Z0A_0000_dddd_imm8 Z0B_0000_dddd_imm16 Z0B_ssN0_dddd Z0C_ddN0_0100
Z0D_ddN0_0001_imm16 Z0D_ddN0_0101_imm16 Z10_0000_dddd_imm32
Z12_0000_dddd_imm32 Z14_0000_dddd_imm32 Z14_ssN0_dddd Z16_0000_dddd_imm32
Z19_0000_dddd_imm16 Z1B_0000_dddd_imm16 Z1C_ddN0_1001_0000_ssss_0000_nmin1
Z1C_ssN0_0001_0000_dddd_0000_nmin1 Z1D_ddN0_ssss Z1E_ddN0_cccc Z1F_ddN0_0000
Z20_ssN0_dddd Z21_0000_dddd_imm16 Z21_ssN0_dddd Z23_ddN0_imm4 Z27_ddN0_imm4
Z2B_ddN0_imm4m1 Z2D_ssN0_dddd Z2E_ddN0_ssss Z2F_ddN0_ssss Z41_0000_dddd_addr
Z41_ssN0_dddd_addr Z43_0000_dddd_addr Z43_ssN0_dddd_addr Z4B_0000_dddd_addr
Z4C_0000_0101_addr_imm8 Z4D_0000_0001_addr_imm16 Z4D_0000_0100_addr
Z4D_0000_0101_addr_imm16 Z4D_0000_1000_addr Z4D_ddN0_0001_addr_imm16
Z4D_ddN0_0100_addr Z4D_ddN0_0101_addr_imm16 Z50_0000_dddd_addr
Z54_0000_dddd_addr Z54_ssN0_dddd_addr Z56_0000_dddd_addr Z59_0000_dddd_addr
Z59_ssN0_dddd_addr Z5C_0000_0001_0000_dddd_0000_nmin1_addr
Z5C_0000_1001_0000_ssss_0000_nmin1_addr Z5D_0000_ssss_addr
Z5D_ddN0_ssss_addr Z5E_0000_cccc_addr Z5F_0000_0000_addr Z60_0000_dddd_addr
Z60_ssN0_dddd_addr Z61_0000_dddd_addr Z61_ssN0_dddd_addr Z63_0000_imm4_addr
Z65_0000_imm4_addr Z67_0000_imm4_addr Z67_ddN0_imm4_addr
Z69_0000_imm4m1_addr Z6B_0000_imm4m1_addr Z6B_ddN0_imm4m1_addr
Z6E_0000_ssss_addr Z6E_ddN0_ssss_addr Z6F_0000_ssss_addr Z6F_ddN0_ssss_addr
Z71_ssN0_dddd_0000_xxxx_0000_0000 Z76_0000_dddd_addr Z7B_0000_0000
Z7C_0000_01ii Z7D_ssss_1ccc Z80_ssss_dddd Z81_ssss_dddd Z82_ssss_dddd
Z83_ssss_dddd Z84_ssss_dddd Z85_ssss_dddd Z87_ssss_dddd Z8B_ssss_dddd
Z8C_dddd_0100 Z8C_dddd_1000 Z8D_dddd_0000 Z8D_dddd_0010 Z8D_dddd_0100
Z8D_dddd_1000 Z90_ssss_dddd Z91_ddN0_ssss Z92_ssss_dddd Z93_ddN0_ssss
Z94_ssss_dddd Z95_ssN0_dddd Z96_ssss_dddd Z97_ssN0_dddd Z99_ssss_dddd
Z9B_ssss_dddd Z9E_0000_cccc ZA0_ssss_dddd ZA1_ssss_dddd ZA5_dddd_imm4
ZA6_dddd_imm4 ZA7_dddd_imm4 ZA8_dddd_imm4m1 ZA9_dddd_imm4m1 ZAA_dddd_imm4m1
ZAB_dddd_imm4m1 ZB0_dddd_0000 ZB1_dddd_0000 ZB1_dddd_1010 ZB2_dddd_0001_imm8
ZB3_dddd_0001_imm8 ZB3_dddd_0101_imm8 ZB3_dddd_1001_imm8 ZB3_dddd_1101_imm8
ZB4_ssss_dddd ZC_dddd_imm8 ZD_dsp12 ZE_cccc_dsp8 ZF_dddd_1dsp7
""".split())

src, out = sys.argv[1], sys.argv[2]
ops = open(src + '/z8000ops.hxx').read()
tbl = open(src + '/z8000tbl.hxx').read()
dab = open(src + '/z8000dab.h').read()
cpuh = open(src + '/z8000cpu.h').read()

def functions(text):
    """(signature, class, name, body) for each top-level member function, in order."""
    res = []
    pat = re.compile(r'^([A-Za-z_][A-Za-z0-9_ \*]*?)\b(z800[12]_device)::([A-Za-z0-9_]+)\s*\(([^)]*)\)\s*\n\{', re.M)
    pos = 0
    while True:
        m = pat.search(text, pos)
        if not m:
            break
        i = m.end()
        depth = 1
        while depth:
            c = text[i]
            depth += (c == '{') - (c == '}')
            i += 1
        res.append((m.group(1).strip(), m.group(2), m.group(3), m.group(4).strip(), text[m.end() - 1:i], m.start(), i))
        pos = i
    return res

funcs = functions(ops)
# the #define blocks between functions (flag checks, privilege traps)
defines = []
prev = 0
for f in funcs:
    chunk = ops[prev:f[5]]
    for line in chunk.split('\n'):
        if line.startswith('#define'):
            defines.append(line)
    prev = f[6]

# Z and S without branches (the same flags: when result is 0, its top bit is
# 0); z8k.c's Z8K_ZERO(x) is x == 0, as 1 or 0
ZS = {'B': ('int8_t', 7, 'Z8K_ZERO(result)'), 'W': ('int16_t', 15, 'Z8K_ZERO(result)'),
      'L': ('int32_t', 31, 'Z8K_ZERO(result)'), 'Q': ('int64_t', 63, '(uint32_t)(result == 0)')}
for k, (t, bit, zero) in ZS.items():
    old = '#define CHK_XXX%s_ZS if (!result) SET_Z; else if ((%s)%sresult < 0) SET_S' % (k, t, ' ' if k == 'B' else '')
    if old not in defines:
        sys.exit('z8kgen: CHK_XXX%s_ZS is not MAME 0.288\'s' % k)
    defines[defines.index(old)] = ('#define CHK_XXX%s_ZS m_fcw |= %s << 6 | (uint32_t)((result) >> %d & 1) << 5'
                                   '   /* %s */' % (k, zero, bit, old[8:]))

# V without a branch: the sign bit of MAME's overflow expression
for name, bit in (('ADDB', 7), ('ADDW', 15), ('ADDL', 31), ('SUBB', 7), ('SUBW', 15), ('SUBL', 31)):
    m = [i for i, l in enumerate(defines) if l.startswith('#define CHK_%s_V if (' % name)]
    want = {'ADD': '((value & dest & ~result) | (~value & ~dest & result))',
            'SUB': '((~value & dest & ~result) | (value & ~dest & result))'}[name[:3]]
    old = '#define CHK_%s_V if (%s & S%02d) SET_V' % (name, want, bit + 1)
    if len(m) != 1 or defines[m[0]] != old:
        sys.exit('z8kgen: CHK_%s_V is not MAME 0.288\'s' % name)
    defines[m[0]] = '#define CHK_%s_V m_fcw |= (uint32_t)(%s >> %d & 1) << 4   /* %s */' % (name, want, bit, old[8:])

def convert(body):
    b = body
    b = re.sub(r'memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &(\w+)', r'int \1', b)
    b = re.sub(r'\bprintf\s*\(', 'Z8K_NOLOG(', b)
    b = re.sub(r'\bLOG\s*\(', 'Z8K_NOLOG(', b)
    b = re.sub(r'\blogerror\s*\(', 'Z8K_NOLOG(', b)
    b = re.sub(r'\bm_mo_out\s*\(', 'Z8K_NOLOG(', b)
    b = re.sub(r'\bm_op\[0\]', 'm_op0', b)
    # HALT's; z8k.c's m_icount is only added to and taken from
    b = b.replace('if (m_icount > 0) m_icount = 0;', 'm_icount_end();')
    for m in re.finditer(r'\bm_icount\b(?!_end)\s*(..)', b):
        if m.group(1) not in ('+=', '-='):
            sys.exit('z8kgen: m_icount used as ' + b[m.start():m.end() + 10])     # (the opcode word: z8k.c keeps it apart)
    if 'z8002_device' in b or 'this' in re.sub(r'\bthis\w', '', b):
        if re.search(r'\bthis\b|z8002_device', b):
            sys.exit('z8kgen: C++ left in a body:\n' + b[:400])
    return b

rows = re.findall(r'\{\s*(0x[0-9a-f]+|0),\s*(0x[0-9a-f]+|0),\s*(\d+),\s*(\d+),\s*(\d+),\s*&z8002_device::(\w+)\s*\}', tbl)
if len(rows) < 500:
    sys.exit('z8kgen: the table has %d rows' % len(rows))
names = {f[2] for f in funcs}
for r in rows:
    if r[5] not in names:
        sys.exit('z8kgen: no handler ' + r[5])
# the instructions, numbered as the table first names them, and their cycles
ops, cyc = [], {}
for r in rows:
    if r[5] not in cyc:
        ops.append(r[5])
        cyc[r[5]] = r[4]
    elif cyc[r[5]] != r[4]:
        sys.exit('z8kgen: %s has two cycle counts' % r[5])
opnum = {n: i for i, n in enumerate(ops)}
if HOT - set(ops):
    sys.exit('z8kgen: HOT names no instruction ' + ' '.join(sorted(HOT - set(ops))))
kind = lambda n: 'H' if n in HOT else 'C'

protos, bodies, calls = [], [], []
for ret, cls, name, args, body, _, _ in funcs:
    if cls != 'z8002_device':
        continue
    args = re.sub(r'memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space', 'int space', args)
    if name in opnum:
        if ret != 'void' or args:
            sys.exit('z8kgen: instruction %s takes or gives something' % name)
        head = 'static Z8K_AI void %s(z8k_st_t *S)' % name
    else:
        head = 'static Z8K_AI %s %s_(z8k_st_t *S%s)' % (ret, name, ', ' + args if args else '')
        calls.append('#define %s(...) %s_(S, ##__VA_ARGS__)' % (name, name))
    protos.append(head + ';')
    bodies.append('%s\n%s\n' % (head, convert(body)))

dabm = re.search(r'static const uint16_t Z8000_dab\[0x800\] = \{(.*?)\};', dab, re.S)

o = []
o.append('// made by tools/z8kgen.py from MAME 0.288 (src/devices/cpu/z8000: z8000ops.hxx,')
o.append('// z8000tbl.hxx, z8000cpu.h, z8000dab.h; Juergen Buchmueller, Ernesto Corvi; BSD-3-Clause). Do not edit.')
o.append('// #included by z8k.c, which supplies the CPU\'s state (S, z8k_st_t) and memory.\n')
o.append('// z8000cpu.h: the register file, the vectors, the flags and the operand fields')
o.extend(l for l in cpuh.split('\n') if l.startswith('#define'))
o.append('')
o.append('static const uint16_t Z8000_dab[0x800] = {%s};\n' % dabm.group(1))
o.extend(defines)
o.append('')
o.append('// the helpers take the state first (NAME(...) is NAME_(S, ...))')
o.extend(calls)
o.append('')
o.extend(protos)
o.append('')
o.extend(bodies)
if ops[0] != 'zinvalid':
    sys.exit('z8kgen: the table does not start with zinvalid')
o.append('// the instructions: X(number, function, cycles, H or C: Pole Position runs it');
o.append('// or not); 0, the first row\'s (every opcode), is Z8K_OP0, the others are Z8K_OPS')
o.append('#define Z8K_NOPS %d' % len(ops))
o.append('#define Z8K_OP0(X) X(0, %s, %s, %s)' % (ops[0], cyc[ops[0]], kind(ops[0])))
o.append('#define Z8K_OPS(X) \\')
for i, n in enumerate(ops):
    if i:
        o.append('\tX(%d, %s, %s, %s)%s' % (i, n, cyc[n], kind(n), ' \\' if i < len(ops) - 1 else ''))
o.append('')
o.append('// the opcode table: rows applied in order, the later winning (init_tables);')
o.append('// MAME\'s sizes are left out (only its disassembler uses them)')
o.append('static const z8k_init_t z8k_table[] = {')
for beg, end, step, size, c, name in rows:
    o.append('\t{ %s, %s, %s, %d },' % (beg, end, step, opnum[name]))
o.append('\t{ 0, 0, 0, 0 }')
o.append('};')
open(out, 'w').write('\n'.join(o) + '\n')
print('z8kgen: %d functions (%d instructions), %d table rows -> %s' % (len(protos), len(ops), len(rows), out))
