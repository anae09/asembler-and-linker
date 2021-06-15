.global c
.extern a, d

.section text
jmp %d
.skip 5

.section data
ldr r5, bss
.skip 10

.section bss
c:
.word 0x6666

.end