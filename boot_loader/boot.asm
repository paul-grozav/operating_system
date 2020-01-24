# Author: Tancredi-Paul Grozav <paul@grozav.info>
# ============================================================================ #
.code64 # tell the assembler that we're using 16 bit mode
.global init # makes our label "init" available to the outside

init: # this is the beginning of our binary later.
# mov $0x0e, %ah
# int $0x10 # call the function in ah from interrupt 0x10

  mov $0x0e, %ah
  mov $0x57, %al # 57=W
  int $0x10

  mov $0x00, %ah			# subfunction 0
  mov $0x04, %al			# select mode 18 (or 12h if prefer)
  int $0x10				# call graphics interrupt

  mov $0x0e, %ah
  mov $0x58, %al # 58=X
  int $0x10

  # Set color 1=blue
  mov $0x01, %al
  mov $0x0c, %ah
  # Set pixel X(cx), set Y(dx) and draw (int 10)
  mov $0x01, %cx
  mov $0x01, %dx
  int $0x10
  # Set pixel X(cx), set Y(dx) and draw (int 10)
  mov $0x02, %cx
  mov $0x01, %dx
  int $0x10
  # Set pixel X(cx), set Y(dx) and draw (int 10)
  mov $0x03, %cx
  mov $0x01, %dx
  int $0x10

  mov $0x00, %ah
  # wait for keypress
  int $0x16

  mov $0x00, %ah			# again subfunc 0
  mov $0x03, %al			# text mode 3
  int $0x10				# call int
  mov $0x04c, %ah
  mov $0x00, %al			# end program normally
  int $0x21

  mov $0x0e, %ah
  mov $0x59, %al # 59=Y
  int $0x10

  hlt # stops executing. Note! avoid: jmp init
# ===========================================================
# ===========================================================

# The last parameter is the value that fills the binary file, up to 510 bytes.
# It can be any value, not just 0. And you can see it using:
# hexdump -C boot.bin
.fill 510-(.-init), 1, 191 # add zeroes to make it 510 bytes long

# Adds 2 extra "magic" bytes that tell BIOS that this is bootable, reaching a
# size of 512 bytes
.word 0xaa55
# ============================================================================ #
