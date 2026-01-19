# void fib() {
#     uint16_t x;
#     uint16_t y;
#     uint16_t z;
#     for (;;) {
#         x = 0;
#         y = 1;
#         while (y >= x) {
#             x = y;
#             y = z;
#         }
#     }
# }

fib:
    mov ge.l, 0
    mov ge.h, 0
    mov gf.l, 1
    mov gf.h, 0
fib_l0:
    mov gg.l, ge.l
    mov gg.h, ge.h
    add gg.l, gf.l
    adc gg.h, gf.h
    mov ge.l, gf.l
    mov ge.h, gf.h
    mov gf.l, gg.l
    mov gf.h, gg.h
    jrnc fib_l0
    jr fib

01110000 10000000
01110000 10010000
01110000 10100001
01110000 10110000
00110000 11001000
00110000 11011001
00000000 11001010
00000100 11011011
00110000 10001010
00110000 10011011
00110000 10101100
00110000 10111101
11001011 10000111
11001011 01110010

70 80
70 90
70 A1
70 B0
30 C8
30 D9
00 CA
04 DB
30 8A
30 9B
30 AC
30 BD
CB 87
CB 72