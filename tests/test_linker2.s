#linker input2

.global f
.global g
.extern c

.section data
.equ f, 3
.word c

.section bss
.equ g, 1
.skip 3
.end
