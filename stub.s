bits 64
default rel

_start:
	lea r8, [rel _start];
	mov r9, r8;
	sub r9, [r8 + main_offset]

; mprotect(address, size, prot)
modify_data_flags:
	mov eax, 0xa
	mov rdi, r8
	sub rdi, [r8 + text_segment_offset]
	mov rsi, [r8 + text_segment_offset]
	mov edx, 0x7
	syscall

; r8 = _start address
; r9 = main address
; r10 = key
; r11 = text section
; r12 = text length
; r13 = key index
decrypt:
	lea r10, [rel key]
	mov r11, r8
	sub r11, [r8 + text_section_offset]
	mov r12, [r8 + text_length]
	xor r13, r13

xor_rot_loop:
	cmp r12, 0x0
	je print_woody
	
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

; write(1, "....WOODY....\n", 14)
print_woody:
	mov eax, 0x1
	lea edi, [rel woody]
	mov esi, 0xe
	syscall

reset:
	mov [rsp], r9
	xor r8, r8
	xor r9, r9
	xor r10, r10
	xor r11, r11
	xor r12, r12
	xor r13, r13
	ret

init_data:
	woody				db "....WOODY....", 0xa
	main_offset			dq 0x0000000000000000
	text_segment_offset	dq 0x0000000000000000
	text_section_offset	dq 0x0000000000000000
	text_length			dq 0x0000000000000000
	key					db "1234567890abcdef1234567890abcdef"