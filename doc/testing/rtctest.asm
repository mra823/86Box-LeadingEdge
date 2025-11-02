; Leading Edge Model D RTC Test Program
; Assembles to a DOS .COM file
; Usage: RTCTEST.COM
;
; This program reads and displays the time/date from the Leading Edge Model D
; Real-Time Clock at I/O port 0x300-0x31F (MM58167 chip).
;
; Assemble with NASM:
;   nasm -f bin rtctest.asm -o rtctest.com
;
; Or with MASM/TASM:
;   tasm rtctest.asm
;   tlink /t rtctest.obj

        ORG     0x100           ; DOS .COM format

start:
        ; Print banner
        mov     dx, banner
        mov     ah, 9
        int     0x21

        ; Read RTC registers
        mov     dx, 0x300       ; Base I/O address

        ; Read seconds (register 2)
        mov     dx, 0x302
        in      al, dx
        mov     [seconds], al

        ; Read minutes (register 3)
        mov     dx, 0x303
        in      al, dx
        mov     [minutes], al

        ; Read hours (register 4)
        mov     dx, 0x304
        in      al, dx
        mov     [hours], al

        ; Read day of month (register 6)
        mov     dx, 0x306
        in      al, dx
        mov     [day], al

        ; Read month (register 7)
        mov     dx, 0x307
        in      al, dx
        mov     [month], al

        ; Read year from alarm register 14 (non-standard location)
        mov     dx, 0x30E
        in      al, dx
        mov     [year], al

        ; Display time
        mov     dx, time_msg
        mov     ah, 9
        int     0x21

        ; Print hours
        mov     al, [hours]
        call    print_bcd
        mov     dl, ':'
        call    print_char

        ; Print minutes
        mov     al, [minutes]
        call    print_bcd
        mov     dl, ':'
        call    print_char

        ; Print seconds
        mov     al, [seconds]
        call    print_bcd
        call    print_crlf

        ; Display date
        mov     dx, date_msg
        mov     ah, 9
        int     0x21

        ; Print month
        mov     al, [month]
        call    print_bcd
        mov     dl, '/'
        call    print_char

        ; Print day
        mov     al, [day]
        call    print_bcd
        mov     dl, '/'
        call    print_char

        ; Print year (19XX format - year is base-80)
        mov     dx, year_prefix
        mov     ah, 9
        int     0x21
        mov     al, [year]
        call    print_bcd
        call    print_crlf

        ; Display raw hex values for debugging
        mov     dx, debug_msg
        mov     ah, 9
        int     0x21

        mov     dx, hex_time
        mov     ah, 9
        int     0x21
        mov     al, [hours]
        call    print_hex
        mov     dl, ':'
        call    print_char
        mov     al, [minutes]
        call    print_hex
        mov     dl, ':'
        call    print_char
        mov     al, [seconds]
        call    print_hex
        call    print_crlf

        mov     dx, hex_date
        mov     ah, 9
        int     0x21
        mov     al, [month]
        call    print_hex
        mov     dl, '/'
        call    print_char
        mov     al, [day]
        call    print_hex
        mov     dl, '/'
        call    print_char
        mov     al, [year]
        call    print_hex
        call    print_crlf

        ; Exit
        mov     ah, 0x4C
        int     0x21

; Print BCD value in AL as two decimal digits
print_bcd:
        push    ax
        push    dx
        mov     ah, al
        shr     al, 4           ; High nibble
        add     al, '0'
        mov     dl, al
        call    print_char
        mov     al, ah
        and     al, 0x0F        ; Low nibble
        add     al, '0'
        mov     dl, al
        call    print_char
        pop     dx
        pop     ax
        ret

; Print hex value in AL
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

; Print character in DL
print_char:
        push    ax
        mov     ah, 2
        int     0x21
        pop     ax
        ret

; Print CR/LF
print_crlf:
        push    dx
        mov     dl, 13
        call    print_char
        mov     dl, 10
        call    print_char
        pop     dx
        ret

; Data
banner:     db 'Leading Edge Model D RTC Test', 13, 10
            db 'MM58167 chip at I/O port 0x300', 13, 10, 13, 10, '$'
time_msg:   db 'Time: $'
date_msg:   db 'Date: $'
year_prefix: db '19$'
debug_msg:  db 13, 10, 'Raw hex values:', 13, 10, '$'
hex_time:   db '  Time: $'
hex_date:   db '  Date: $'

; Storage for RTC values
seconds:    db 0
minutes:    db 0
hours:      db 0
day:        db 0
month:      db 0
year:       db 0
