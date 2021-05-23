.section test
# test comment

.word start
.word a
.word b
.word c

a:
b:
c:
iret

.section start
        ldr r0, $5
        ldr r1, $6
        ldr r2, $7
        ldr r3, $8

        add r0, r2
        sub r2, r3
        mul r1, r3
        div r3, r3

        ldr r0, $tmp
        ldr r1, [r0]
        ldr r2, [r0 + 5]
        ldr r3, [r0 + 0x1234]

        ldr r5, LF
        ldr r0, $smth

        and r4, r4
        or r5, r5

        shl r6, r6

        halt


.equ tmp, 0xff00
.equ LF, 0xA

.end