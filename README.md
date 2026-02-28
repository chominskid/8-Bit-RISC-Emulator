# Emulator

**Usage**: `./emulator <program binary>`

When launching the emulator, the following occurs in order:

<ol>
<li>The debug bootloader binary (programs/DEBUG_BOOTLOADER.bin) is copied to address `0x0000`.</li>
<li>The chosen program binary is copied to address `0x0300`.</li>
<li>The emulator displays an 80x50 character screen and a debug overview of the CPU state in a window.</li>
<li>The CPU begins execution at the debug bootloader (program counter starts at `0x0000`).</li>
<li>The debug bootloader calls the program at address `0x0300`.</li>
<li>If and when the program returns, the debug bootloader loops indefinitely.</li>
</ol>

## Commands
The terminal that launched the emulator is used as a command input to the emulator.

| Syntax | Description |
| :----: | :----: |
| `run` | Run the CPU as quickly as possible (with no timing overhead). |
| `run <f>` | Try to run the CPU at a fixed frequency `f` (Hz). |
| `step` | Execute one CPU cycle. |
| `step <n>` | Execute `n` CPU cycles as quickly as possible. |
| `stop` | Stop the CPU if it's running. |
| `exit` | Close the emulator. |

## Screen

The screen to the left of the emulator window is an 80x50 character screen. Each character is an 8x8 bitmap character with foreground and background colors selectable from 16 predefined colors. The screen can be controlled through its character memory, located at address `0xE000`. This memory consists of 4000 16-bit words. Each word corresponds to a single character position, in row-major order starting from the top-left corner. All writes to this memory region will immediately update the screen (provided they are not outside the bounds of the screen, as the memory region is expanded to 8192 bytes).
  
### Character Memory Word Format

**Order**: Big-Endian  
**Alignment**: 2  
  
`ffffbbbb cccccccc`  
**f**: foreground color  
**b**: background color  
**c**: character  

### Colors

| Code | Color |
| :----: | :----: |
| 0000 | Black |
| 0001 | White |
| 0010 | Red |
| 0011 | Orange |
| 0100 | Yellow |
| 0101 | Green |
| 0110 | Blue |
| 0111 | Purple |
| 1000 | Dark Red |
| 1001 | Dark Orange |
| 1010 | Dark Yellow |
| 1011 | Dark Green |
| 1100 | Dark Blue |
| 1101 | Dark Purple |
| 1110 | Gray |
| 1111 | Dark Gray |

### Character Map

See emulator/src/frontend/font.cpp

## Memory Layout

| Address Range | Read/Write | Function |
| :----: | :----: | :----: |
| `0000`-`00FF` | R | Bootloader   |
| `0100`-`01FF` | R/W | Stack        |
| `0200`-`02FF` | R/W | Zero Page    |
| `0300`-`....` | R/W | Program |
| `....`-`DFFF` | R/W | Main Memory |
| `E000`-`FFFF` | R/W | Character Memory |

# ISA Description

## Registers

| Label | Width | Description |
| ----- | ----- | :----: |
| ra    |   16  | return address |
| sr    |   8   | status register |
| sp    |   8   | stack pointer |
| fp/ga |   8   | frame pointer / general purpose a |
| gb    |   8   | general purpose b |
| gc    |   8   | general purpose c |
| gd    |   8   | general purpose d |
| ge    |   16  | general purpose e |
| gf    |   16  | general purpose f |
| gg    |   16  | general purpose g |
| gh    |   16  | general purpose h |



## Instruction Encoding

**Order**: Big-Endian  
**Alignment**: 2  

### Arithmetic \(A\) Instruction Encoding

**`00oooo.. xxxxyyyy`**  
**o**: ALU operation  
**x**: operand 1 (D addressing mode)  
**y**: operand 2 (D addressing mode)  

Perform ALU operation on registers x and y, storing the result in x. The operation may modify the status register.

### Immediate Arithmetic \(IA\) Instruction Encoding

**`01ooooii xxxxiiii`**  
**o**: ALU operation  
**x**: operand 1 (D addressing mode)  
**i**: signed immediate, operand 2  
  
Perform ALU operation on register x and immediate, storing the result in x. The operation may modify the status register.

### Memory \(M\) Instruction Encoding

**`10smmmii xxxxiiii`**  
**s**: 1 = store, 0 = load  
**m**: base address (M addressing mode)  
**x**: target register (D addressing mode)  
**i**: signed immediate, address offset  
  
Store/load 1 byte of data at address (base address + immediate) from/to target register.

### Control \(C\) Instruction Encoding

**`11smmmii nccciiii`**  
**s**: save return address (1)  
**m**: base address (C addressing mode)  
**n**: negate condition (1)  
**c**: condition code  
**i**: signed immediate, address offset  
  
Conditionally set pc to address (base address + immediate * 2), optionally saving its old value to ra (before this instruction executes, pc holds the address of the next instruction).



## Addressing Modes

### Data \(D\) addressing mode

| Code | Mnemonic | Description |
| :----: | :----: | :----: |
| `0000` | ra.l  | return address low byte           |
| `0001` | ra.h  | return address high byte          |
| `0010` | sr    | status register                   |
| `0011` | sp    | stack pointer                     |
| `0100` | fp/ga | frame pointer / general purpose a |
| `0101` | gb    | general purpose b                 |
| `0110` | gc    | general purpose c                 |
| `0111` | gd    | general purpose d                 |
| `1000` | ge.l  | general purpose e low byte        |
| `1001` | ge.h  | general purpose e high byte       |
| `1010` | gf.l  | general purpose f low byte        |
| `1011` | gf.h  | general purpose f high byte       |
| `1100` | gg.l  | general purpose g low byte        |
| `1101` | gg.h  | general purpose g high byte       |
| `1110` | gh.l  | general purpose h low byte        |
| `1111` | gh.h  | general purpose h high byte       |
    
### Memory \(M\) addressing mode

| Code | Mnemonic/Name | Description |
| :----: | :----: | :----: |
| `000` | `sp` / stack address | base address = 0x0100 + sp |
| `001` | `fp` / frame address | base address = 0x0100 + fp |
| `010` | `pc` / relative      | base address = pc          |
| `011` | `gb` / zero page | base address = 0x0200 + gb |
| `100` | `ge`            | base address = ge          |
| `101` | `gf`            | base address = gf          |
| `110` | `gg`            | base address = gg          |
| `111` | `gh`            | base address = gh          |

### Control \(C\) addressing mode

| Code | Mnemonic/Name | Description |
| :----: | :----: | :----: |
| `000` | `bl` / bootloader low  | base address = 0x0040      |
| `001` | `bh` / bootloader high | base address = 0x00C0      |
| `010` | `pc` / relative        | base address = pc          |
| `011` | `ra` / return          | base address = ra & 0xFFFE |
| `100` | `ge`              | base address = ge & 0xFFFE |
| `101` | `gf`              | base address = gf & 0xFFFE |
| `110` | `gg`              | base address = gg & 0xFFFE |
| `111` | `gh`              | base address = gh & 0xFFFE |



## Status Register

**`sr = ....cvnz`**  

**c**: carry output  
If the last executed arithmetic operation defines carry, this bit is modified according to the carry output of the operation.  
  
**v**: overflow  
If the last executed arithmetic operation defines overflow, this bit is modified to indicate whether arithmetic overflow has occured.  
  
**n**: negative  
This bit is equal to the 7th bit of the result of the last executed arithmetic operation.  
  
**z**: zero  
This bit is set if and only if the result of the last executed arithmetic operation is equal to zero.  



## Condition Codes
|  Code  |  Mnemonic |               Description               |
| :----: |   :----:  |                  :----:                 |
|  `000`   |   `c`/`geu` | carry / greater or equal (unsigned)     |
|  `001`   |   `v`       | overflow                                |
|  `010`   |   `n`       | negative                                |
|  `011`   |   `z`/`eq`  | zero / equal                            |
|  `100`   |   `g`       | greater (signed) [(v ? c : !n) && !z]   |
|  `101`   |   `ge`      | greater or equal (signed) [v ? c : !n]  |
|  `110`   |   `gu`      | greater (unsigned) [c && !z]            |
|  `111`   |             | true                                    |

## ALU Operations

| Code | Name | Flags Affected | Effect |
| :----: | :----: | :----: | :----: |
| `0000` | `add`  | `cvnz` | `op1 += op2`                                                                  |
| `0001` | `adc`  | `cvnz` | `op1 += op2 + c`                                                              |
| `0010` | `sub`  | `cvnz` | `op1 += ~op2 + 1`                                                             |
| `0011` | `sbc`  | `cvnz` | `op1 += ~op2 + c`                                                             |
| `0100` | `cmp`  | `cvnz` | `op1 + ~op2 + 1`                                                              |
| `0101` | `cmc`  | `cvnz` | `op1 + ~op2 + c`                                                              |
| `0110` | `and`  | `__nz` | `op1 &= op2`                                                                  |
| `0111` | `or `  | `__nz` | `op1 \|= op2`                                                                 |
| `1000` | `xor`  | `__nz` | `op1 ^= op2`                                                                  |
| `1001` | `shl`  | `__nz` | `op1 <<= op2 & 0x07`                                                          |
| `1010` | `shr`  | `__nz` | `op1 >>= op2 & 0x07`                                                          |
| `1011` |        | `____` |                                                                               |
| `1100` | `mov`  | `____` | `op1 = op2`                                                                   |
| `1101` | `movh` | `____` | `op1 = (op1 & 0x3F) \| ((op2 << 6) & 0xC0)`                                   |
| `1110` | `tsb`  | `__nz` | `z = (op1 & (1 << (op2 & 0x07))) != 0, n = (op1 & (1 << (op2 & 0x38))) != 0`  |
| `1111` | `seb`  | `__nz` | `if (op2 & 0x08) op1 \|= 1 << (op2 & 0x07), else op1 &= ~(1 << (op2 & 0x07))` |



## Instruction

| Syntax | Mnemonic | Description | Encoding(s) |
| :----: | :----: | :----: | :----: |
| `nop` | no operation | add register gb to register gb | `00110000 01010101` |
| | | | |
| `add x, (y/i)` | add | add (register y / immediate i) to register x                                                    | `00000000 xxxxyyyy` / `010000ii xxxxiiii` |
| `add x, (y/i)` | add with carry | add (register y / immediate i) to register x with carry                              | `00000100 xxxxyyyy` / `010000ii xxxxiiii` |
| `sub x, (y/i)` | subtract | subtract (register y / immediate i) from register x                                        | `00001000 xxxxyyyy` / `010010ii xxxxiiii` |
| `sbc x, (y/i)` | subtract with carry | subtract (register y / immediate i) from register x with carry                  | `00001100 xxxxyyyy` / `010010ii xxxxiiii` |
| `cmp x, (y/i)` | compare | compare (register y / immediate i) to register x                                            | `00010000 xxxxyyyy` / `010100ii xxxxiiii` |
| `cmc x, (y/i)` | compare with carry | compare (register y / immediate i) to register x with carry                      | `00010100 xxxxyyyy` / `010100ii xxxxiiii` |
| `and x, (y/i)` | and | and (register y / immediate i) with register x                                                  | `00011000 xxxxyyyy` / `010110ii xxxxiiii` |
| `or x, (y/i)`  | or | or (register y / immediate i) with register x                                                    | `00011100 xxxxyyyy` / `010111ii xxxxiiii` |
| `xor x, (y/i)` | exclusive or | exclusive or (register y / immediate i) with register x                                | `00100000 xxxxyyyy` / `011000ii xxxxiiii` |
| `shl x, (y/i)` | shift left | shift register x left by (register y / immediate i) bits                                 | `00100100 xxxxyyyy` / `01100100 xxxx0iii` |
| `shr x, (y/i)` | shift right | shift register x right by (register y / immediate i) bits                               | `00101000 xxxxyyyy` / `01101000 xxxx0iii` |
| `mov x, (y/i)` | move | move (register y / immediate i) to register x                                                  | `00110000 xxxxyyyy` / `011100ii xxxxiiii` |
| `mvh x, (y/i)` | move high | move highest 2 bits of (register y / immediate i) to highest 2 bits of register x         | `00110100 xxxxyyyy` / `01110100 xxxx00ii` |
| `tsb x, a, b` | test bits | set z and n to bits a and b of register x respectively                                     | `011110bb xxxxbaaa` |
| `tsb x, y`    | test bits | set z and n to bits (register y & 0x07) and (register y & 0x38) of register x respectively | `00111000 xxxxyyyy` |
| `seb x, i, s` | set bit | set bit i of register x to s                                                                 | `01111100 xxxxsiii` |
| `seb x, y` | set bit | set bit (register y & 0x07) of register x to (register y & 0x08)                                | `00111100 xxxxyyyy` |
| | | | |
| `jmp [c,] x, i`     | jump                 | jump to address in wide register x with immediate offset i [if condition c is true]        | `1101xxii nccciiii` |
| `jmpr [c,] i`       | jump relative        | jump by immediate offset i [if condition c is true]                                        | `110010ii nccciiii` |
| `jmpbl [c,] i`      | jump bootloader low  | jump to address offset by immediate i from bootloader low address [if condition c is true] | `110000ii nccciiii` |
| `jmpbh [c,] i`      | jump bootloader high | jump bootloader high [if condition c is true]                                              | `110001ii nccciiii` |
| `call [c,] x, i`    | call                 | call function at address in wide register x [if condition c is true]                       | `1111xxii nccciiii` |
| `rcall [c,] i`      | call relative        | call relative [if condition c is true]                                                     | `111010ii nccciiii` |
| `clbl [c,] i`       | call bootloader low  | call bootloader low [if condition c is true]                                               | `111000ii nccciiii` |
| `clbh [c,] i`       | call bootloader high | call bootloader high [if condition c is true]                                              | `111001ii nccciiii` |
| `ret [c]`           | return               | return [if condition c is true]                                                            | `110011ii nccciiii` |
| `callret [c,] i`    | call return          | call return [if condition c is true]                                                       | `111011ii nccciiii` |
| | | | |
| `ld x, y, i` | load                          | `100xxxii yyyyiiii` |
| `ldr x, i`   | load relative                 | `100010ii xxxxiiii` |
| `lds x, i`   | load stack                    | `100000ii xxxxiiii` |
| `ldf x, i`   | load frame                    | `100001ii xxxxiiii` |
| | | | |
| `st x, y, i` | store                         | `101xxxii yyyyiiii` |
| `sts x, i`   | store stack                   | `101000ii xxxxiiii` |
| `stf x, i`   | store frame                   | `101001ii xxxxiiii` |

### Condition Encoding

**`nccc`**  
**n**: negate condition  
**c**: condition code  

| Syntax | Description | Condition | Encoding |
| :----: | :----: | :----: |
| `c` / `geu` | carry / greater than or equal to unsigned | `c` | `0000` |
| `v` | overflow | `v` | `0001` |
| `n` | negative | `n` | `0010` |
| `z` / `eq` | zero / equal to | `z` | `0011` |
| `g` | greater | `(v ? c : !n) && !z` | `0100` |
| `ge` | greater than or equal to | `v ? c : !n` | `0101` |
| `gu` | greater than unsigned | `c && !z` | `0110` |
| | | |
| `nc` / `lu` | not carry | `!c` | `1000` |
| `nv` | not overflow | `!v` | `1001` |
| `nn` | not negative | `!n` | `1010` |
| `nz` / `ne` | not zero / not equal | `!z` | `1011` |
| `le` | less than or equal to | `(v ? !c : n) \|\| z` | `1100` |
| `l` | less than | `v ? !c : n` | `1101` |
| `leu` | less than or equal to unsigned | `!c \|\| z` | `1110` |
| | | |
| | always (specified by omitting condition) | `true` | `0111` |

## Memory Layout

| Address Range | Function |
| :----: | :----: |
| `0000`-`00FF` | Bootloader   |
| `0100`-`01FF` | Stack        |
| `0200`-`02FF` | Zero Page    |
| `0300`-`FFFF` | User Defined |