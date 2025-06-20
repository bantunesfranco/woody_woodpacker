bits 64
section .text
global xor_rot_encrypt

; Function: xor_rot_encrypt
; Description: Encrypts data using XOR and then ROT, cycling through the key if data is larger.
; Parameters:
;   - rdi: pointer to the data
;   - rsi: size of the data
;   - rdx: pointer to the key
;   - rcx: key size

xor_rot_encrypt:
	test rsi, rsi
	jz done
	xor r8, r8

xor_rot_loop:
	mov al, [rdi]
	mov bl, [rdx + r8]
	xor al, bl
	rol al, 2
	mov [rdi], al
	inc rdi
	inc r8
	cmp r8, rcx
	jb no_reset
	xor r8, r8

no_reset:
	dec rsi
	jnz xor_rot_loop

done:
	ret

;	void xor_rot_encrypt(void *data, uint64_t data_size, unsigned char *key, size_t key_size)
;	{
;		if (data_size == 0 || key_size == 0)
;			return;
;
;		size_t key_index = 0;
;		for (size_t i = 0; i < data_size; ++i)
;		{
;			data[i] ^= key[key_index];
;			data[i] = (data[i] << 2) | (data[i] >> (8 - 2));
;			key_index = (key_index + 1) % key_size;
;		}
;	}
