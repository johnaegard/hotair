#!/usr/bin/env python3

import random

filename = "map0.bin"
b = bytearray()
b += bytes([2,3])

colors=[0,11,9,6]

for row in range(128):
  for col in range(256):
    tile = colors[random.randint(0,len(colors)-1)]
    b += bytes([tile,0])

with open(filename, 'wb') as f:
  f.write(b)
  
print(f"Wrote {len(b)} bytes to {filename}")