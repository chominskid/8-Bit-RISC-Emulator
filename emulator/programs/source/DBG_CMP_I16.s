main:
    push ra
    mov ge, 1234
    mov gf, 1235
    sts ge.l, 0
    sts ge.h, 1
    sts gf.l, 2
    sts gf.h, 3
    add sp, 4
    rcall dbg_sr_cmp_i16_i16
    rcall print_u8_bin
    pop ra
    ret
    
    

# Compare two i16 arguments and return the resulting status register.
#
# Arguments:
#     sp-4: i16 lhs
#     sp-2: i16 rhs
#
# Return:
#     sp-1: u8 sr
dbg_sr_cmp_i16_i16:
    lds ge.l, -4
    lds ge.h, -3
    lds gf.l, -2
    lds gf.h, -1
    cmp ge.l, gf.l
    cmc ge.h, gf.h
    sts sr, -4
    sub sp, 3
    ret



# Print a u8 in binary to the top-left of the screen.
# 
# Arguments:
#     -1: u8 sr
#
# Return:
#     void
print_u8_bin:
    mov ge, 0xE000
    lds gb, -1
    mov gc, 0x80
    mov gd, 8
    mov gf.l, 0x31
    mov gf.h, 0x10
print_u8_bin_l0:
    mov gg.l, gc
    and gg.l, gb
    mov gg.l, sr
    and gg.l, 1
    mov gg.h, gf.l
    sub gg.h, gg.l
    st gg.h, ge, 0
    st gf.h, ge, 1
    add ge.l, 2
    adc ge.h, 0
    shl gb, 1
    sub gd, 1
    rjmp nz, print_u8_bin_l0

    sub sp, 1
    ret