.global a
.global b
.global c

.extern e

# .equ a, 0x55

# text sekcija --------
.section text
call e
.equ a, 2
.word 22
halt

# data sekcija ---------
.equ c, 4
.section data
.word 0x11

# bss sekcija ------------
.equ b, 1
.section bss
.skip 4 
.end