#ifndef WOODY_H
#define WOODY_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <endian.h>
#include <elf.h>

#define SHELLCODE_SIZE 233
// #define SHELLCODE_SIZE 520
#define PATCH_OFFSET 170
#define KEY_SIZE 32

typedef enum e_location
{
	LOAD,
	NOTE,
	BSS,
	NEW
} t_location;

typedef struct s_file
{
	int			fd;
	uint16_t	arch;
	uint64_t	size;
	uint64_t	text_size;
	void		*ptr;
	void		*text_sec;
	void		*load_seg;
	void		*payload_section;
} t_file;

typedef struct s_payload
{
	uint64_t	entry_offset;
	uint64_t	segment_offset;
	uint64_t	text_offset;
	uint64_t	text_size;
	char		key[KEY_SIZE];
} t_payload;

int		validate_elf(t_file *file);
int		inject_payload(t_file *file, t_payload *payload);
int		generate_encription_key(t_payload *payload);
void	encrypt_file(t_file *file, t_payload *payload);
void	create_new_executable(t_file *file);
void	*patch(t_file *file, t_payload *payload, int *patch_location);
void	*use_bss_segment(t_file *file, uint64_t size);
void	*use_note_segment(t_file *file, uint64_t size);
void	*create_new_segment(t_file *file, uint64_t size);

// Utils
void	*get_section(t_file *file, uint16_t i);
void	*get_section_by_name(t_file *file, char *name);
void	*get_segment_by_type(t_file *file, uint32_t type);
void	*get_load_segment(t_file *file);

#endif