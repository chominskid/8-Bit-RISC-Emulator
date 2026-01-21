FILE = "./programs/SEQ_CHAR.bin"

DATA = """
01110000
01010000
01110001
01100110
01110000
10000000
01110010
10010000
01000000
01010001
11001100
00110000
10110000
01010000
10110000
01100001
01000000
10000010
01000100
10010000
11001011
01111001
"""

with open(FILE, "wb") as file:
    for line in DATA.split('\n'):
        line = line.strip()
        if len(line) == 0:
            continue
        num = int(line, base=2)
        file.write(num.to_bytes(1, byteorder="little"))