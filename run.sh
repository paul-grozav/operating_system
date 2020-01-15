#!/bin/bash
# ============================================================================ #
# Author: Tancredi-Paul Grozav <paul@grozav.info>
# ============================================================================ #
# OS setup
#apt update &&
#apt install -y curl gcc g++ make libgmp-dev libmpfr-dev libmpc-dev &&


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
  ([ ! -f binutils-${version}.tar.gz ] && curl https://ftp.gnu.org/gnu/binutils/binutils-${version}.tar.gz --output binutils-${version}.tar.gz || true) &&
  ([ ! -d binutils-${version} ] && tar xf binutils-${version}.tar.gz || true) &&
  mkdir -p build-binutils &&
  cd build-binutils &&
  ../binutils-${version}/configure --target=${target} --prefix="${prefix}" --with-sysroot --disable-nls --disable-werror &&
  make -j $(nproc) &&
  make install
fi &&

# Build GCC
if [ ! -f ${prefix}/bin/${target}-gcc ]; then
  version="9.2.0" &&
  mkdir -p ${work_path}/gcc &&
  cd ${work_path}/gcc &&
  ([ ! -f gcc-${version}.tar.gz ] && curl https://ftp.gnu.org/gnu/gcc/gcc-${version}/gcc-${version}.tar.gz --output gcc-${version}.tar.gz || true) &&
  ([ ! -d gcc-${version} ] && tar xf gcc-${version}.tar.gz || true) &&
  mkdir -p build-gcc &&
  cd build-gcc &&
  ../gcc-${version}/configure --target=${target} --prefix="${prefix}" --disable-nls --enable-languages=c,c++ --without-headers &&
  make all-gcc -j $(nproc) &&
  make all-target-libgcc -j $(nproc) &&
  make install-gcc &&
  make install-target-libgcc
fi &&

# Compile kernel
cd ${work_path}/project &&
${target}-gcc -std=gnu99 -ffreestanding -g -c start.s -o start.o &&
${target}-gcc -std=gnu99 -ffreestanding -g -I. -c kernel.c -o kernel.o &&
${target}-gcc -std=gnu99 -ffreestanding -g -I. -c module.c -o module.o &&
${target}-gcc -ffreestanding -nostdlib -g -T linker.ld start.o kernel.o module.o -o mykernel.elf -lgcc &&

# Run kernel using:
# qemu-system-i386 -kernel project/mykernel.elf

exit 0
# ============================================================================ #
