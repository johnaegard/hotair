#!/usr/bin/env python3

import random

b = bytearray()
b += bytes([2,3])

colors=[0,11,9]

for row in range(256):
  for col in range(128):
    # tile = 0 if (row < 4 or row > 251 or col < 4 or col > 123) else colors[random.randint(0,len(colors)-1)] 
    tile = 0 if (row < 4 or row > 251 or col < 4 or col > 123) else int(row / 16)  
    b += bytes([tile,0])

filename="map0.bin"
with open(filename, 'wb') as f:
  f.write(b)
  
print(f"Wrote {len(b)} bytes to {filename}")

b1=bytearray()
b1 += bytes([2,3])
for row in range(32):
  for col in range(64):
    color = 0x00 if col <30 else 0x61
    char =  0x60 
    b1 += bytes([char,color])

filename1="map1.bin"
with open(filename1, 'wb') as f:
  f.write(b1)

print(f"Wrote {len(b1)} bytes to {filename1}")

