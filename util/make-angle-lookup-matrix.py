#!/usr/bin/env python3

import math

def clockwise_angle(v1, v2):
  """
  Compute the clockwise angle from vector v1 to vector v2.
  Vectors must be 2D and originate at the origin.
  """

  if vector == (0,0):
    return 0

  x1, y1 = v1
  x2, y2 = v2

  # Compute the angle of each vector from the positive x-axis
  angle1 = math.atan2(y1, x1)
  angle2 = math.atan2(y2, x2)

  # Compute the difference in angles (clockwise)
  angle_rad = angle1 - angle2

  # Convert to degrees and normalize to [0, 360)
  angle_deg = math.degrees(angle_rad) % 360

  return angle_deg

# Example usage
v1 = (0, 1)

print("unsigned int angle_lookup[31][31] = {")

for x in range(-15,16):
  print("  {", end="") 
  for y in range(-15,16):
    vector=(x*8,y*8)
    angle_deg = int(clockwise_angle(v1, vector))
    sprite24_bearing = int(angle_deg / 15)
    print(f"{sprite24_bearing},",end="")
    # print(f"vector {str(vector)} {angle_deg} {sprite16_bearing} ")
  print("},") 
print("}")