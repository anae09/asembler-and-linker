# sum 2 numbers

.section text
main: ldr r0, a
ldr r1, b

add r0, r1
halt

.section data
.word a, 5
.word b, 6

.end