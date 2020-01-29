# Author: Tancredi-Paul Grozav <paul@grozav.info>
# Try talking with the kernel
# ============================================================================ #
import os
import time

# Let the partner know that we are ready for serial communication
print(chr(0))

def start_talking():
  print(chr(0))

serial_file = "project/build/serial.txt"
open(serial_file, "a").close() # make sure it exists

while True:
#  print("Waiting for serial init.")
  if os.path.getsize(serial_file) == 1:
    with open(serial_file, mode="rb") as f:
      if f.read() == chr(0):
        start_talking()
#      else:
        # protocol error
  time.sleep(1) # 1 second
# ============================================================================ #
