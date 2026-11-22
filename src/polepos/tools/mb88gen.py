#!/usr/bin/env python3
# license:BSD-3-Clause
# copyright-holders:Ernesto Corvi
"""mb88gen.py MB88XX.CPP OUT.H -- MAME's MB88xx instructions as C.

MAME 0.288's Fujitsu MB88xx core (src/devices/cpu/mb88xx/mb88xx.cpp,
Ernesto Corvi, BSD-3-Clause) runs each instruction in one switch in
execute_run(). This takes that switch as it is and turns its C++ into
mb88.c's names: the members stay m_ names (mb88.c #defines them to the
running MCU's fields), the device callbacks (K, R, O, P) become the
board's functions, the serial timer's adjust becomes mb88.c's, and
assignments to PC and PA become SET_PC and SET_PA.
mb88.c #includes the output inside its execute loop.
"""
import re, sys

cpp = open(sys.argv[1]).read()
m = re.search(r'void mb88_cpu_device::execute_run\(\)\n\{.*?\n\t\tswitch \(opcode\)\n\t\t\{\n', cpp, re.S)
if not m:
    sys.exit('mb88gen: no execute_run switch')
i = m.end()
depth = 1
start = i
while depth:
    c = cpp[i]
    depth += (c == '{') - (c == '}')
    i += 1
body = cpp[start:i - 1]
n_cases = len(re.findall(r'^\t\t\tcase 0x', body, re.M))

subs = [
    (r'm_read_k\(\)', 'mb88_read_k(M->id)'),
    (r'm_read_r\[([^\]]+)\]\(\)', r'mb88_read_r(M->id, (\1) & 3)'),
    (r'm_write_r\[([^\]]+)\]\(', r'mb88_write_r(M->id, (\1) & 3, '),
    (r'm_write_p\(', 'mb88_write_p(M->id, '),
    (r'm_serial->adjust\(attotime::from_hz\(clock\(\) / SERIAL_PRESCALE\), 0, attotime::from_hz\(clock\(\) / SERIAL_PRESCALE\)\)', 'mb88_serial_on()'),
    # PC and PA are one register in mb88.c (PA << 6 | PC)
    (r'\bm_PC = (.+);', r'SET_PC(\1);'),
    (r'\bm_PA = (.+);', r'SET_PA(\1);'),
]
for a, b in subs:
    body = re.sub(a, b, body)
left = re.findall(r'm_(?:read|write)_\w+|->adjust|attotime|::', body)
if left:
    sys.exit('mb88gen: C++ left: %s' % sorted(set(left)))
out = ['// made by tools/mb88gen.py from MAME 0.288 (src/devices/cpu/mb88xx/mb88xx.cpp,',
       '// Ernesto Corvi, BSD-3-Clause): execute_run()\'s switch. Do not edit.',
       '// #included by mb88.c inside its execute loop.',
       body.rstrip() + '\n']
open(sys.argv[2], 'w').write('\n'.join(out))
print('mb88gen: %d cases -> %s' % (n_cases, sys.argv[2]))
