.section start
a:
jeq b # neposredno adr, relok. zapis APS
jne %a # lokalni + PCREL -> nema relok zapisa
jne %d
jmp 0x55
b:
jgt *start
jgt *0x66

jmp *r0

jmp *[r1]
jmp *[r1 + 0x1234]
jmp *[r2 + 10]

jmp *[r3 + a]

.section data
.skip 3
d:
.word 0xFFFF

.end