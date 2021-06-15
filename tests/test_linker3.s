.global e
.extern g

.equ e, 3

.section text
jne g
jmp %LABELA

.section bss
.skip 4
LABELA:
.end
