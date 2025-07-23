#!/usr/bin/env python3

import random

def junkyard_petscii(w,h):

  b = bytearray()
  b += bytes([2,3])

  colors=[0xDB,0x6B,9,9,9,9,9,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,9,9,9,9,9,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,15]

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

def overlay(w,h):
  b=bytearray()
  b += bytes([2,3])
  for row in range(h):
    for col in range(w):
      color = 0x01 if col <70 else 0xC1
      if col == 70:
        if row == 0:
          char = 0x4f
        elif row == 59:
          char = 0x4c;
        else:
          char = 0x74
      elif col == 79:
        if row == 0:
          char = 0x50
        elif row == 59:
          char = 0x7a;
        else:
          char = 0x6a;
      elif row == 0 and col >= 70 and col <= 78:
          char = 0x77;
      elif row == 59 and col >= 70 and col <= 78:
          char = 0x6f;
      else:
        char=0x20;

      b += bytes([char,color])
  return b    

def draw_block(w,row,col,b):
  print(w,row,col)

  row0_byte = int(2 + (row * w + col) * 2)
  row1_byte = row0_byte + (2*w)
  row2_byte = row1_byte + (2*w)
  row3_byte = row2_byte + (2*w)
  color = 0x02

  b[row0_byte:row0_byte+8] = bytes([0x70,color,0x40,color,0x40,color,0x6e,color])
  b[row1_byte:row1_byte+8] = bytes([0x42,color,0x20,color,0x20,color,0x42,color])
  b[row2_byte:row2_byte+8] = bytes([0x42,color,0x20,color,0x20,color,0x42,color])
  b[row3_byte:row3_byte+8] = bytes([0x6d,color,0x40,color,0x40,color,0x7d,color])


w=128
h=256
b = junkyard_petscii(w,h)

ccol = int((w/2) -2)
crow = int((h/2) -2)

draw_block(w,crow-6,ccol-6,b)
draw_block(w,crow-6,ccol+6,b)
draw_block(w,crow+6,ccol-6,b)
draw_block(w,crow+6,ccol+6,b)

filename="map0.bin"
with open(filename, 'wb') as f:
  f.write(b) 

print(f"Wrote {len(b)} bytes to {filename}")

ow = 128
oh = 64

b1=overlay(ow,oh)
wind_label_addr = 2 + ( (53*ow) + 75) *2;
color = 0x21;
b1[wind_label_addr:wind_label_addr+8] = bytes([0x17, color, 0x09, color, 0x0e, color, 0x04, color])

filename1="map1.bin"
with open(filename1, 'wb') as f:
  f.write(b1)

print(f"Wrote {len(b1)} bytes to {filename1}")

