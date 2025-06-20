/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   encryption.c                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/06/22 16:47:02 by bfranco       #+#    #+#                 */
/*   Updated: 2025/03/15 18:13:37 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"
#include <openssl/evp.h>
#include <openssl/rand.h>

extern void		xor_rot_encrypt(unsigned char *text, uint64_t text_size, unsigned char *key, size_t key_size);

void axor_rot_encrypt(uint8_t *data, size_t data_size, const uint8_t *key, size_t key_size) {
	if (data_size == 0 || key_size == 0)
		return;

	size_t key_index = 0;
	for (size_t i = 0; i < data_size; ++i) {
		data[i] ^= key[key_index]; // XOR with the key
		data[i] = (data[i] << 2) | (data[i] >> (8 - 2)); // Rotate left by 2 bits
		key_index = (key_index + 1) % key_size; // Cycle through the key
	}
}

int generate_encription_key(t_payload *payload)
{
	unsigned char key[KEY_SIZE];
	unsigned char salt[16];
	int iterations = 100000;

	char password[] = "very_secure_password_1234";
	// char password[32];
	// memset(password, 0, 32);
	// printf("Enter a password (size < 32): ");
	// fgets(password, 32, stdin);

	// Generate a random salt
	if (RAND_bytes(salt, sizeof(salt)) != 1) {
		return -1;
	}
	// Derive the encryption key using PBKDF2
	if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), iterations, EVP_sha256(), KEY_SIZE, key) == 0) {
		return -1;
	}

	memcpy(payload->key, key, KEY_SIZE);
	return 0;
}

void	encrypt_file(t_file *file, t_payload *payload)
{
	unsigned char *key = (unsigned char*)payload->key;
	uint8_t *data = file->text_sec;
	size_t data_size = file->text_size;
	xor_rot_encrypt(data, data_size, key, KEY_SIZE);
}