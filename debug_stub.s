bits 64
default rel

_start:
    ; Save original registers
    push rax
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13

    lea r8, [rel _start]
    mov r9, r8
    sub r9, [r8 + main_offset]

    ; Debug print current position
    mov rax, 1          ; write syscall
    mov rdi, 1          ; stdout
    lea rsi, [rel dbg1] ; message
    mov rdx, dbg1_len   ; length
    syscall

; mprotect(address, size, prot)
modify_data_flags:
    mov eax, 0xa
    mov rdi, r8
    sub rdi, [r8 + text_segment_offset]
    mov rcx, 0xfffffffffffff000
    and rdi, rcx
    mov rsi, [r8 + text_length]
    add rsi, 0xfff
    and rsi, rcx

    ; Check mprotect result
    test rax, rax
    js error_mprotect

decrypt:
    lea r10, [rel key]
    mov r11, r8
    sub r11, [r8 + text_section_offset]
    mov r12, [r8 + text_length]
    xor r13, r13
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel dbg2]
    mov rdx, dbg2_len
    syscall

xor_rot_loop:
    cmp r12, 0x0
    je print_woody

    cmp r13, 0x20
    jae error_bounds

    mov al, byte [r11]
    mov bl, byte [r10 + r13]
    ror al, 0x2
    xor al, bl
    mov byte [r11], al
    inc r11
    inc r13

    cmp r13, 0x20
    jne no_reset
    xor r13, r13

no_reset:
    dec r12
    jmp xor_rot_loop

print_woody:
    mov eax, 0x1
    lea rdi, [rel woody]
    mov rsi, 0xe
    syscall

reset:
    ; Restore original registers
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rax

    mov [rsp], r9
    ret

error_mprotect:
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel err1]
    mov rdx, err1_len
    syscall
    jmp error_exit

error_bounds:
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel err2]
    mov rdx, err2_len
    syscall
    jmp error_exit

error_exit:
    mov rax, 60         ; exit syscall
    mov rdi, 1          ; exit code 1
    syscall

section .data
    dbg1:       db "[DEBUG] Starting decryption at: ", 0xa
    dbg1_len:   equ $ - dbg1
    dbg2:       db "[DEBUG] Beginning decryption loop", 0xa
    dbg2_len:   equ $ - dbg2
    err1:       db "[ERROR] mprotect failed", 0xa
    err1_len:   equ $ - err1
    err2:       db "[ERROR] Key index out of bounds", 0xa
    err2_len:   equ $ - err2
    woody:      db "....WOODY....", 0xa
    align 8
    main_offset:         dq 0x0000000000000000
    text_segment_offset: dq 0x0000000000000000
    text_section_offset: dq 0x0000000000000000
    text_length:         dq 0x0000000000000000
    key:                db "1234567890abcdef1234567890abcdef"