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

| Mnemonic | Description | Encoding(s) |
| :----: | :----: | :----: |
| `nop` | no operation | `00110000 01010101` |
| | | |
| `add x, (y/i)`   | add                       | `00000000 xxxxyyyy`, `010000ii xxxxiiii` |
| `add x, (y/i)`   | add with carry            | `00000100 xxxxyyyy`, `010000ii xxxxiiii` |
| `sub x, (y/i)`   | subtract                  | `00001000 xxxxyyyy`, `010010ii xxxxiiii` |
| `sbc x, (y/i)`   | subtract with carry       | `00001100 xxxxyyyy`, `010010ii xxxxiiii` |
| `cmp x, (y/i)`   | compare                   | `00010000 xxxxyyyy`, `010100ii xxxxiiii` |
| `cmc x, (y/i)`   | compare with carry        | `00010100 xxxxyyyy`, `010100ii xxxxiiii` |
| `and x, (y/i)`   | and                       | `00011000 xxxxyyyy`, `010110ii xxxxiiii` |
| `or x, (y/i)`    | or                        | `00011100 xxxxyyyy`, `010111ii xxxxiiii` |
| `xor x, (y/i)`   | exclusive or              | `00100000 xxxxyyyy`, `011000ii xxxxiiii` |
| `shl x, (y/i)`   | shift left                | `00100100 xxxxyyyy`, `01100100 xxxx0iii` |
| `shr x, (y/i)`   | shift right               | `00101000 xxxxyyyy`, `01101000 xxxx0iii` |
| `mov x, (y/i)`   | move                      | `00110000 xxxxyyyy`, `011100ii xxxxiiii` |
| `mvh x, (y/i)`   | move high                 | `00110100 xxxxyyyy`, `01110100 xxxx00ii` |
| `tsb x, (y/a,b)` | test bit                  | `00111000 xxxxyyyy`, `011110bb xxxxbaaa` |
| `seb x, (y/s,i)` | set bit (bit i = s)       | `00111100 xxxxyyyy`, `01111100 xxxxsiii` |
| | | |
| `jmp_ x, i`  | jump [if ...]                 | `1101xxii nccciiii` |
| `jmpr_ i`    | jump relative [if ...]        | `110010ii nccciiii` |
| `jmpbl_ i`   | jump bootloader low [if ...]  | `110000ii nccciiii` |
| `jmpbh_ i`   | jump bootloader high [if ...] | `110001ii nccciiii` |
| `call_ x, i` | call [if ...]                 | `1111xxii nccciiii` |
| `callr_ i`   | call relative [if ...]        | `111010ii nccciiii` |
| `clbl_ i`    | call bootloader low [if ...]  | `111000ii nccciiii` |
| `clbh_ i`    | call bootloader high [if ...] | `111001ii nccciiii` |
| `ret_`       | return [if ...]               | `110011ii nccciiii` |
| `clret_ i`   | call return [if ...]          | `111011ii nccciiii` |
| | | |
| `ld x, y, i` | load                          | `100xxxii yyyyiiii` |
| `ldr x, i`   | load relative                 | `100010ii xxxxiiii` |
| `lds x, i`   | load stack                    | `100000ii xxxxiiii` |
| `ldf x, i`   | load frame                    | `100001ii xxxxiiii` |
| | | |
| `st x, y, i` | store                         | `101xxxii yyyyiiii` |
| `sts x, i`   | store stack                   | `101000ii xxxxiiii` |
| `stf x, i`   | store frame                   | `101001ii xxxxiiii` |



## Memory Layout

| Address Range | Function |
| :----: | :----: |
| `0000`-`00FF` | Bootloader   |
| `0100`-`01FF` | Stack        |
| `0200`-`02FF` | Zero Page    |
| `0300`-`FFFF` | User Defined |