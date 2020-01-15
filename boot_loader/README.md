Compile source code into an object: `as boot.asm -o boot.o`

And link object into final binary: `ld boot.o -o boot.bin --oformat binary -e init`

`--oformat binary` instructs the linker to produce a plain binary file and `-e init` specifies the entry point into our program, to have it start at the assembly label init.

`as` and `ld` are part of linux package `binutils`.

Run with: `qemu-system-x86_64 boot.bin`
