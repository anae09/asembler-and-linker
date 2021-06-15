#linker input1

.global a
.global b
.global c

.extern e

.equ a, 2

.section text
jmp e
.word data

.equ c, 4
.section data
.skip 6
.word 0x11

.section bss
.equ b, 4
.skip 4
.end
