#!/usr/bin/env python3

import random

def random_petscii():
  b = bytearray()
  b += bytes([2,3])

  colors=[0,11,9]

  for row in range(256):
    for col in range(128):
      # tile = 0 if (row < 4 or row > 251 or col < 4 or col > 123) else colors[random.randint(0,len(colors)-1)] 
      # tile = 0 if (row < 4 or row > 251 or col < 4 or col > 123) else int(row / 16)  
      # b += bytes([tile,0])
      tile = random.randint(0x40,0x7f)
      color = ((int(row/16) + int(col/16)) % 15 + 1);
      b += bytes([tile,color])
  return b    

b = random_petscii()
filename="map0.bin"
with open(filename, 'wb') as f:
  f.write(b)
  
print(f"Wrote {len(b)} bytes to {filename}")

b1=bytearray()
b1 += bytes([2,3])
for row in range(32):
  for col in range(64):
    color = 0x01 #if col <30 else 0xC1
    if col == 30:
      if row == 0:
        char = 0x4f
      elif row == 29:
        char = 0x4c;
      else:
        char = 0x74
    elif col == 39:
      if row == 0:
        char = 0x50
      elif row == 29:
        char = 0x7a;
      else:
        char = 0x6a;
    elif row == 0 and col >= 31 and col <= 38:
        char = 0x77;
    elif row == 29 and col >= 31 and col <= 38:
        char = 0x6f;
    else:
      char=0x20;

    b1 += bytes([char,color])

filename1="map1.bin"
with open(filename1, 'wb') as f:
  f.write(b1)

print(f"Wrote {len(b1)} bytes to {filename1}")

