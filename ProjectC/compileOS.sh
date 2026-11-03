#!/bin/bash
# Andrew Dergham, Mason Andrade, Chris Felumero
# Project C build script: builds the disk image, kernel, shell and test programs

# 1. Create a raw blank disk image of 512KB (1000 sectors of 512 bytes)
dd if=/dev/zero of=diskc.img bs=512 count=1000

# 2. Assemble the bootloader using NASM
nasm bootload.asm

# 3. Write the assembled bootloader to the very beginning (sector 0) of the disk image
dd if=bootload of=diskc.img bs=512 conv=notrunc seek=0

# 4. Compile the host-side utility used to copy files into the disk image
gcc -o loadFile loadFile.c

# 5. Compile the Kernel:
# Assemble the assembly part
as86 -o kernel_asm.o kernel.asm
# Compile the C part
bcc -ansi -c -o kernel_c.o kernel.c
# Link them together into a single kernel binary
ld86 -d -o kernel kernel_c.o kernel_asm.o

# 6. Compile User Programs (shell and two test programs):
# Assemble the shared user library
as86 -o userlib.o userlib.asm
# Loop through each program to compile and link with the user library
for p in shell tstpr1 tstpr2; do
    bcc -ansi -c -o $p.o $p.c
    ld86 -d -o $p $p.o userlib.o
done

# 7. Finalize the disk image:
# Use the loadFile utility to place the compiled binaries into the disk image.
# Files are placed sequentially starting after the bootloader.
./loadFile kernel
./loadFile shell
./loadFile tstpr1
./loadFile tstpr2
./loadFile message.txt
