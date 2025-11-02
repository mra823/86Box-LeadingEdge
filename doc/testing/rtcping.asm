; Ultra-simple RTC test - just read port 0x300 and print it
; Assemble with: nasm -f bin rtcping.asm -o rtcping.com

        ORG     0x100

start:
        mov     dx, msg1
        mov     ah, 9
        int     0x21

        ; Read port 0x300 (register 0 - milliseconds)
        mov     dx, 0x300
        in      al, dx
        
        ; Print hex value
        call    print_hex
        call    print_crlf

        ; Read port 0x302 (register 2 - seconds)
        mov     dx, msg2
        mov     ah, 9
        int     0x21
        
        mov     dx, 0x302
        in      al, dx
        call    print_hex
        call    print_crlf

        ; Read port 0x30E (register 14 - year)
        mov     dx, msg3
        mov     ah, 9
        int     0x21
        
        mov     dx, 0x30E
        in      al, dx
        call    print_hex
        call    print_crlf

        ; Exit
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

msg1:   db 'Port 0x300: $'
msg2:   db 'Port 0x302: $'
msg3:   db 'Port 0x30E: $'
