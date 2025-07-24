#!/usr/bin/env python3

def read_binary_file_two_bytes_at_a_time(filename):
  try:
    with open(filename, 'rb') as file:
      while True:
        chunk = file.read(2)
        if not chunk:
          break  # End of file

        byte1 = chunk[0]
        byte2 = chunk[1]

        green = (byte1 & 0xF0) >> 4  # Most significant nibble of byte1
        blue  = byte1 & 0x0F         # Least significant nibble of byte1
        red   = byte2 & 0x0F         # Least significant nibble of byte2

        print(f"{red * 16:<4} {green * 16:<4} {blue*16:<4} {red:X}{green:X}{blue:X}")        

  except IOError as e:
    print(f"Error reading file: {e}")

if __name__ == "__main__":
  filename = "assets/palette/x16-default-palette.bin"  
  read_binary_file_two_bytes_at_a_time(filename)
