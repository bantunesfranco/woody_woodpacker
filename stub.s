bits 64

section .text
global _start

_start:
	lea r8, [rel _start];
	mov r9, r8;
	sub r9, [r8 + main_offset]
	jmp init_data

modify_data_flags:
	mov eax, 0xa
	mov rdi, r8
	sub rdi, [r8 + text_segment_offset]
	mov rsi, [r8 + text_segment_offset]
	mov edx, 0x7
	syscall

	call init_data

ft_strlen:
	xor eax, eax
	lea edi, [rel woody]

.loop:
	cmp byte [edi + eax], 0
	je print_woody
	inc eax
	jmp .loop

print_woody:
	mov edx, eax
	mov esi, edi
	mov edi, 0x1
	mov eax, 0x1
	syscall

decrypt:
	; r8 = _start address
	; r9 = main address
	; r10 = key
	; r11 = text section
	; r12 = text length
	; r13 = key index

	lea r10, [rel key]
	mov r11, [r8 + text_section_offset]
	mov r12, [r8 + text_length]
	xor r13, r13

xor_rot_loop:
	mov al, byte [r11]
	mov bl, byte [r10 + r13]
	ror al, 2
	xor al, bl
	mov byte [r11], al
	inc r11
	inc r13

	cmp [r10 + r13], byte 0
	jne no_reset
	xor r13, r13

no_reset:
	dec r12
	jnz xor_rot_loop

reset:
	push r9
	xor r8, r8
	xor r9, r9
	xor r10, r10
	xor r11, r11
	xor r12, r12
	xor r13, r13
	ret

init_data:

	call modify_data_flags

	woody				db "....WOODY....", 0xa, 0x0
	main_offset			dq 0x0000000000000000
	text_segment_offset	dq 0x0000000000000000
	text_section_offset	dq 0x0000000000000000
	text_length			dq 0x0000000000000000
	key					db "1234567890abcdef1234567890abcdef", 0x0