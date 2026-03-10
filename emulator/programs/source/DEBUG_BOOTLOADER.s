# Simple debug bootloader.
#
# Procedure:
#     1. set up the stack
#     2. call 0x0300 (no arguments)
#     3. halt
reset:
    mov sp, 0
    mov fp, 0
    mov ge, 0x0300
    call ge
reset_l0:
    rjmp reset_l0