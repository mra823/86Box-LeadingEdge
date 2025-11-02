; Simple RTC Register Dump
; Reads all 32 registers from port 0x300-0x31F
; 
; Assemble with: nasm -f bin rtcdump.asm -o rtcdump.com

        ORG     0x100

start:
        mov     dx, banner
        mov     ah, 9
        int     0x21

        ; Dump all 32 registers
        mov     cx, 32          ; 32 registers
        mov     dx, 0x300       ; Start at port 0x300
        xor     bx, bx          ; Register counter

dump_loop:
        ; Print register number
        mov     al, bl
        call    print_hex
        mov     dl, ':'
        call    print_char
        mov     dl, ' '
        call    print_char

        ; Read and print register value
        mov     dx, 0x300
        add     dl, bl          ; Port = 0x300 + register
        in      al, dx
        call    print_hex

        ; Print space or newline (4 per line)
        inc     bx
        test    bl, 3
        jnz     .space
        call    print_crlf
        jmp     .continue
.space:
        mov     dl, ' '
        call    print_char
        mov     dl, ' '
        call    print_char
.continue:
        loop    dump_loop

        ; Exit
        call    print_crlf
        mov     ah, 0x4C
        int     0x21

print_hex:
        push    ax
        push    dx
        mov     ah, al
        shr     al, 4
        call    print_hex_digit
        mov     al, ah
        and     al, 0x0F
        call    print_hex_digit
        pop     dx
        pop     ax
        ret

print_hex_digit:
        cmp     al, 10
        jl      .decimal
        add     al, 'A' - 10
        jmp     .print
.decimal:
        add     al, '0'
.print:
        mov     dl, al
        call    print_char
        ret

print_char:
        push    ax
        mov     ah, 2
        int     0x21
        pop     ax
        ret

print_crlf:
        push    dx
        mov     dl, 13
        call    print_char
        mov     dl, 10
        call    print_char
        pop     dx
        ret

banner:
        db 'Leading Edge Model D - RTC Register Dump', 13, 10
        db 'Port 0x300-0x31F (MM58167 chip)', 13, 10, 13, 10, '$'
