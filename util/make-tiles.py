#!/usr/bin/env python3

def create_full_byte_array():
  tile_length_bytes = int(16 * 16 / 2)
  b = bytearray()
  b+= bytes([0x11,0x04])
  for color in range(16):
    fourbit_color_byte = (color << 4) + color; 
    b += bytes( [fourbit_color_byte] * tile_length_bytes )
  return b

def write_to_file(filename):
  data = create_full_byte_array()
  with open(filename, 'wb') as f:
    f.write(data)
  print(f"Wrote {len(data)} bytes to {filename}")

# Example usage
write_to_file('../tiles.bin')