#!/bin/bash
# Andrew Dergham, Mason Andrade, Chris Felumero
# Project C build script: builds the disk image, kernel, shell and test programs

# make an empty disk and put the bootloader in sector 0
dd if=/dev/zero of=diskc.img bs=512 count=1000
nasm bootload.asm
dd if=bootload of=diskc.img bs=512 conv=notrunc seek=0

# the file-loading utility (runs on the host, not inside the OS)
gcc -o loadFile loadFile.c

# kernel
bcc -ansi -c -o kernel_c.o kernel.c
as86 -o kernel_asm.o kernel.asm
ld86 -d -o kernel kernel_c.o kernel_asm.o

# user programs
as86 -o userlib.o userlib.asm
for p in shell tstpr1 tstpr2; do
    bcc -ansi -c -o $p.o $p.c
    ld86 -d -o $p $p.o userlib.o
done

# put everything on the disk (kernel must be first so it lands at sector 3)
./loadFile kernel
./loadFile shell
./loadFile tstpr1
./loadFile tstpr2
./loadFile message.txt
