.global a, d
.extern c

.section text
ldr r0, a
a: jmp %c

.section bss
.skip 5
d: .word 0x5555

.section data
.skip 2
b: .word 0x7777

.end