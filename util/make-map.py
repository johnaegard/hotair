#!/usr/bin/env python3

import random

def random_petscii():

  h = 256
  w = 128

  b = bytearray()
  b += bytes([2,3])

  colors=[6,9,9,9,9,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,15]

  color_grid = []
  color_grid = [[0 for _ in range(w)] for _ in range(h)]
  
  for row in range(h):
    for col in range(w):
      color_grid[row][col] = colors[random.randint(0,len(colors)-1)]

  for passes in range(5):
    for row in range(1,h-1):
      for col in range(1,w-1):
        roll = random.randint(0,99) 
        if roll < 25: 
          color_grid[row][col] = color_grid[row][col-1]
        elif roll < 50: 
          color_grid[row][col] = color_grid[row-1][col]
        elif roll < 75: 
          color_grid[row][col] = color_grid[row][col+1]
        elif roll < 99: 
          color_grid[row][col] = color_grid[row+1][col]

  for row in range(256):
    for col in range(128):
      tile = random.randint(0x40,0x7f)
      color = color_grid[row][col]
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
    color = 0x01 if col <34 else 0xC1
    if col == 34:
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
    elif row == 0 and col >= 34 and col <= 38:
        char = 0x77;
    elif row == 29 and col >= 34 and col <= 38:
        char = 0x6f;
    else:
      char=0x20;

    b1 += bytes([char,color])

wind_label_addr = 2 + ( (23*64) + 35) *2;
color = 0x21;
b1[wind_label_addr:wind_label_addr+8] = bytes([0x17, color, 0x09, color, 0x0e, color, 0x04, color])

filename1="map1.bin"
with open(filename1, 'wb') as f:
  f.write(b1)

print(f"Wrote {len(b1)} bytes to {filename1}")

