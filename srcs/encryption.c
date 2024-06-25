/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   encryption.c                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/06/22 16:47:02 by bfranco       #+#    #+#                 */
/*   Updated: 2024/06/25 20:25:15 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"
#include <openssl/evp.h>
#include <openssl/rand.h>

// extern void		xor_rot_encrypt(unsigned char *text, uint64_t text_size, unsigned char *key, size_t key_size);

void axor_rot_encrypt(uint8_t *data, size_t data_size, const uint8_t *key, size_t key_size) {
	if (data_size == 0 || key_size == 0) return;

	size_t key_index = 0;
	for (size_t i = 0; i < data_size; ++i) {
		data[i] ^= key[key_index]; // XOR with the key
		data[i] = (data[i] << 2) | (data[i] >> (8 - 2)); // Rotate left by 2 bits

		key_index = (key_index + 1) % key_size; // Cycle through the key
	}
}

static int parameterized_encryption(void *text, uint64_t text_size) {
	unsigned char salt[16];
	unsigned char key[32];
	int iterations = 100000;

	char password[] = "very_secure_password_1234";
	// char password[32];
	// memset(password, 0, 32);
	// printf("Enter a password: ");
	// fgets(password, 32, stdin);

	// Generate a random salt
	if (RAND_bytes(salt, sizeof(salt)) != 1) {
		return -1;
	}
	// Derive the encryption key using PBKDF2
	if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), iterations, EVP_sha256(), sizeof(key), key) == 0) {
		return -1;
	}

	axor_rot_encrypt((unsigned char*)text, text_size, key, sizeof(key));
	return 0;
}

static int	encrypt_32(t_file *file)
{
	Elf32_Shdr *text = get_section_by_name(file, ".text");
	if (!text)
		return (-1);
		
	file->text = file->ptr + get_uint32(text->sh_offset, file->endian);
	file->text_size = get_uint32(text->sh_size, file->endian);

	return (parameterized_encryption(file->text, file->text_size));
}

static int	encrypt_64(t_file *file)
{
	Elf64_Shdr *text = get_section_by_name(file, ".text");
	if (!text)
		return (-1);
		
	file->text = file->ptr + get_uint64(text->sh_offset, file->endian);
	file->text_size = get_uint64(text->sh_size, file->endian);
	
	return (parameterized_encryption(file->text, file->text_size));
}

int	encrypt_file(t_file *file)
{
	int ret = -1;

	switch (file->arch)
	{
		case ELFCLASS32:
			ret = encrypt_32(file);
			break;
		case ELFCLASS64:
			ret = encrypt_64(file);
			break;
		default:
			dprintf(2, "Woody Woodpacker: Error: Unsupported architecture\n");
			break;
	}
	return (ret);
}