#make empty disk
dd=/dev/zero of=diskc.img bs=512 count=1000

nasm bootload.asm
dd if=bootload of=diskc.img bs=512 conv=notrunc seek=0

bcc -c -ansi -o kernel_c.o kernel.c

as86 -o kernel_asm.o kernel.asm

ld86 -d -o kernel kernel_c.o kernel_asm.o
#put kernel on disk at secotr 3

dd if=kernel of=diskc.img bs=512 conv=notrunc seek=3

dd if=message.txt of=diskc.img bs=512 count=1 seek=30 conv=notrunc

#java .jar simulator.jar
