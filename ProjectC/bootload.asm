bits 16
KSEG    equ 0x1000
KSIZE   equ 10
KSTART  equ 3

org 0h

mov ax,KSEG
mov ds,ax
mov ss,ax
mov es,ax

mov ax,0xfff0
mov sp,ax
mov bp,ax

mov cl,KSTART+1
mov dh,0
mov ch,0
mov ah,2
mov al,KSIZE
mov dl,80h
mov bx,0
int 13h

jmp KSEG:0

times 510-($-$$) db 0

dw 0xAA55
