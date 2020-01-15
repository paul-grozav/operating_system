# Author: Tancredi-Paul Grozav <paul@grozav.info>
# ============================================================================ #
.code16 # tell the assembler that we're using 16 bit mode
.global init # makes our label "init" available to the outside

init: # this is the beginning of our binary later.
  # Note The dollar sign is used in front of constant numeric values.

  # Set AH to value 0xe which is the teletype function. This function prints a
  # character and automatically advances the cursor. The character printer by
  # this function will be read from(must be set to) AL.
  mov $0x0e, %ah

  # Set the character we want to be printed: X (or 0x58 in hex)
  mov $0x58, %al

  # Call the function in AH (teletype) from interrupt 0x10(video)
  int $0x10 # call the function in ah from interrupt 0x10

  # Print another character by setting AL and calling the interrupt
  mov $0x59, %al
  int $0x10

  hlt # stops executing. Note! avoid: jmp init
  # If we would call init again, that would print same character forever

# The last parameter is the value that fills the binary file, up to 510 bytes.
# It can be any value, not just 0. And you can see it using:
# hexdump -C boot.bin
.fill 510-(.-init), 1, 191 # add zeroes to make it 510 bytes long

# Adds 2 extra "magic" bytes that tell BIOS that this is bootable, reaching a
# size of 512 bytes
.word 0xaa55
# ============================================================================ #
