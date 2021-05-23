.section wtest
lbl0:
        .word 0x5
        # .word 04
        .word 04
lbl1:
        .word 3, lbl1, 1
        .word 0

        .word 0x5678
        .word 0xF8F8
.end