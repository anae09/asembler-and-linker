.global a, c
.extern b

.section text
    jmp     %a
    jmp     %e
    jmp     %b
    jmp     %d
d: .word d
    ldr r0, b
    str r0, c
    ldr r0, e

.section data
    .skip   8
e: .word    a, c
    .word   bss
a: .word    b

.section bss
c: .skip    8

.end