#!/bin/bash
# ============================================================================ #
# Author: Tancredi-Paul Grozav <paul@grozav.info>
# ============================================================================ #
# podman run -it --volume $(pwd):/mnt:rw --name=my_os_builder \
#   docker.io/debian:11.5 # pwd=fs
# OS setup
# apt-get update &&
# apt-get upgrade &&
# apt-get install -y curl gcc g++ make libgmp-dev libmpfr-dev libmpc-dev \
#  grub2 xorriso &&
# Continue without installing GRUB? [yes/no] yes
# exit # return to hypervisor
# podman start os && podman exec -it my_os_builder /bin/bash

set -x
work_path="/mnt" &&
cd ${work_path} &&
# Preparation
export prefix="${work_path}/cross" &&
export target="i686-elf" && # see https://wiki.osdev.org/Target_Triplet
export PATH="${prefix}/bin:${PATH}" &&

mkdir -p ${prefix} &&

# Build BinUtils
if [ ! -f ${prefix}/bin/${target}-ld ]; then
  version="2.33.1" &&
  mkdir -p ${work_path}/binutils &&
  cd ${work_path}/binutils &&
  ([ ! -f binutils-${version}.tar.gz ] && curl \
    https://ftp.gnu.org/gnu/binutils/binutils-${version}.tar.gz --output \
    binutils-${version}.tar.gz || true) &&
  ([ ! -d binutils-${version} ] && tar xf binutils-${version}.tar.gz || true) &&
  mkdir -p build-binutils &&
  cd build-binutils &&
  ../binutils-${version}/configure --target=${target} --prefix="${prefix}" \
    --with-sysroot --disable-nls --disable-werror &&
  make -j $(nproc) &&
  make install
fi &&

# Build GCC
if [ ! -f ${prefix}/bin/${target}-gcc ]; then
  version="9.2.0" &&
  mkdir -p ${work_path}/gcc &&
  cd ${work_path}/gcc &&
  ([ ! -f gcc-${version}.tar.gz ] && curl \
    https://ftp.gnu.org/gnu/gcc/gcc-${version}/gcc-${version}.tar.gz --output \
    gcc-${version}.tar.gz || true) &&
  ([ ! -d gcc-${version} ] && tar xf gcc-${version}.tar.gz || true) &&
  mkdir -p build-gcc &&
  cd build-gcc &&
  ../gcc-${version}/configure --target=${target} --prefix="${prefix}" \
    --disable-nls --enable-languages=c,c++ --without-headers &&
  make all-gcc -j $(nproc) &&
  make all-target-libgcc -j $(nproc) &&
  make install-gcc &&
  make install-target-libgcc
fi &&

# Compile kernel
echo "Compiling kernel ..." &&
cd ${work_path}/project &&
rm -rf build && mkdir -p build && cd build &&
src_folder="${work_path}/project/src" &&
source_files="kernel" &&
modules="kernel terminal serial base interrupt heap video video_mode video_font\
  keyboard _pci _driver__rtl8139 _network _network__data\
  _network__ethernet_interface _network__service__dhcp__client\
  _network__service__http__client _network__ip__tcp" &&
for m in ${modules}; do
  source_files="${source_files} module_${m}"
done &&
# Compile
c_compiler_params="${target}-gcc -std=gnu99 -ffreestanding -g3 -O1 -Wall \
  -Wextra -I${src_folder}" &&
${target}-as ${src_folder}/start.s -o start.o &&
${target}-as ${src_folder}/module_interrupt_asm.s -o module_interrupt_asm.o &&
for f in ${source_files}; do
  ${c_compiler_params} -c ${src_folder}/${f}.c -o ${f}.o
done
# Link
object_list="start.o module_interrupt_asm.o" &&
for f in ${source_files}; do
  object_list="${object_list} ${f}.o"
done
${target}-gcc -ffreestanding -nostdlib -g -T ../linker.ld ${object_list} -lgcc \
  -o my_kernel.elf &&
echo "Compiled kernel!" &&

# Make ISO
echo "Creating .iso file ..." &&
mkdir -p iso/boot/grub &&
(cat - <<EOF 1>iso/boot/grub/grub.cfg
set timeout=0 # Wait 0 seconds for the user to choose the item, or use default
set default=0 # Set the default menu entry - index 0 (first)
CONFIG_PANIC_TIMEOUT=10

menuentry "My Operating System"
{
  multiboot /boot/kernel-file #The multiboot command replaces the kernel command
  boot
}
EOF
) &&
cp my_kernel.elf iso/boot/kernel-file &&
grub-mkrescue iso -o bootable.iso 2>/dev/null &&
echo "Created .iso file!" &&


# Run iso: qemu-system-i386 -cdrom build/bootable.iso -boot d
# Run kernel using:
# qemu-system-i386 -curses -kernel project/build/my_kernel.elf
# Exit with: ESC, 2 you can switch to QEMU's console, then write quit and type ENTER to close the emulator.
# Serial console: ESC, 3 to switch to QEMU's serial console
# Serial redirect: -serial file:project/build/serial.txt
# Current running cmd: qemu-system-i386 -curses -cdrom project/build/bootable.iso -boot d -serial file:project/build/serial.txt

#chown root:root -R ${work_path} &&

exit 0
# ============================================================================ #

