# ============================================================================ #
# Author: Tancredi-Paul Grozav <paul@grozav.info>
# ============================================================================ #
current_directory="$(cd $(dirname $0); pwd)" &&
docker exec -it os bash /mnt/run.sh &&
(
#  -device rtl8139,netdev=net0,mac=52:54:00:12:13:56 \
qemu-system-x86_64 \
  -m 400M \
  -cdrom ${current_directory}/build/bootable.iso \
  -boot d \
  -device rtl8139,netdev=net0 \
  -netdev user,id=net0 \
  -object filter-dump,id=f1,netdev=net0,file=${current_directory}/dump.dat \
  &
) &&
# sleep 3 && ( sleep 1 | telnet 127.0.0.1 5556 )
exit 0
# ============================================================================ #
