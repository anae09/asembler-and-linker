.global a
.equ a, 2
.equ b, 2

.section text
    .skip 2
    jmp a
    jmp %b
.end