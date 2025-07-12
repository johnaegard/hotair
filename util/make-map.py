#!/usr/bin/env python3

import random

filename = "map0.bin"
b = bytearray()
b += bytes([2,3])

colors=[0,11,9]

for row in range(256):
  for col in range(128):
    # tile = 0 if (row < 4 or row > 251 or col < 4 or col > 123) else colors[random.randint(0,len(colors)-1)] 
    tile = 0 if (row < 4 or row > 251 or col < 4 or col > 123) else int(row / 16)  
    b += bytes([tile,0])

with open(filename, 'wb') as f:
  f.write(b)
  
print(f"Wrote {len(b)} bytes to {filename}")