# sum array values

.section text

ldr r0, $array # r0 <= array pointer
ldr r1, $array_end
sub r1, r0 # r1 <= array_len

ldr r3, $0 # r3 <= index

loop:
    cmp r3, r1 
    jeq exit # index == array_len : exit
    jeq %exit
    ldr r4, %sum
    ldr r2, [r3 + array] # proveriti jos
    add r4, r0
    str r4, sum
    # update index
    ldr r5, $1 
    add r3, r5
    # move array pointer


exit:  
    halt

.section data
array: .word 1, 2, 3, 4
array_end:
.end 

# ne parsira posle .end

.section bss
sum: 
    .word 0x0


