# Author: Tancredi-Paul Grozav <paul@grozav.info>
# ============================================================================ #
.code16 # tell the assembler that we're using 16 bit mode
.global init # makes our label "init" available to the outside

init: # this is the beginning of our binary later.
  jmp init # jump to "init" - will loop in this recursive call, hanging

.fill 510-(.-init), 1, 0 # add zeroes to make it 510 bytes long
.word 0xaa55 # adds 2 extra "magic" bytes that tell BIOS that this is bootable,
# reaching a size of 512 bytes
# ============================================================================ #
