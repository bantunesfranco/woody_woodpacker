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

typedef struct s_file
{
	int			fd;
	uint16_t	endian;
	uint16_t	arch;
	off_t		size;
	void		*ptr;
	void		*end;
	void		*text;
	uint64_t	text_size;
} t_file;

typedef struct s_payload
{
	void		*text;
	uint64_t	text_size;
	uint64_t	size;
} t_payload;

uint16_t		get_uint16(uint16_t byte, uint16_t endian);
uint32_t		get_uint32(uint32_t byte, uint16_t endian);
int32_t			get_int32(int32_t byte, uint16_t endian);
uint64_t		get_uint64(uint64_t byte, uint16_t endian);

int				encrypt_file(t_file *file);
void			*get_section_by_name(t_file *file, char *name);
void			*get_text_segment(t_file *file);

#endif