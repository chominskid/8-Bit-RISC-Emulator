hello_world:
    mov gd, 0x16                            # set color (white on black)
    sts ra.l 0                              # save return address
    sts ra.h 1
    callr _1                                # get address of string
    mov ge.l ra.l
    mov ge.h ra.h
    mov gf.l 0x00                           # load address of screen (0xE000)
    mov gf.h 0x20
    lds ra.l 0                              # restore return address
    lds ra.h 1
_0:
    ld gc ge 0                              # get next char
    cmp gc 0                                # return to bootloader if char is 0
    retz _1
    st gc gf 0                              # write char to screen
    st gd gf 1                              # write color to screen
    add ge.l 1                              # increment string and screen pointers
    adc ge.h 0
    add gf.l 2
    adc gf.h 0
    jmpr _0                                 # loop
_1:
    callret
.string "Hello, world!\0"


hello_world:
01110001    mov gd, 0x10
01110110

10100000    sts ra.l 0
00000000

10100000    sts ra.h 1
00010001

11101001    callr _1
01110000

00110000    mov ge.l ra.l
10000000

00110000    mov ge.h ra.h
10010001

01110000    mov gf.l 0x00
10100000

01110010    mov gf.h 0x20
10110000

10000000    lds ra.l 0
00000000

10000000    lds ra.h 1
00010001
_0:
10010000    ld gc ge 0
01100000

01010000    cmp gc 0
01100000

11001100    retz _1
00110000

10110100    st gc gf 0
01100000

10110100    st gd gf 0
01110001

01000000    add ge.l 1
10000001

01000100    adc ge.h 0
10010000

01000000    add gf.l 2
10100010

01000100    adc gf.h 0
10110000

11001011    jmpr _0
01110110
_1:
11101100    callret
01110000

01001000    "Hello, world!\0"
01100101
01101100
01101100
01101111
00101100
00100000
01110111
01101111
01110010
01101100
01100100
00100001
00000000


01110001
01110110
10100000
00000000
10100000
00010001
11101001
01110000
00110000
10000000
00110000
10010001
01110000
10100000
01110010
10110000
10000000
00000000
10000000
00010001
10010000
01100000
01010000
01100000
11001100
00110000
10110100
01100000
10110100
01110001
01000000
10000001
01000100
10010000
01000000
10100010
01000100
10110000
11001011
01110110
11101100
01110000
01001000
01100101
01101100
01101100
01101111
00101100
00100000
01110111
01101111
01110010
01101100
01100100
00100001
00000000