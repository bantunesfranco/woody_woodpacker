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
} t_file;

uint16_t		get_uint16(uint16_t byte, uint16_t endian);
uint32_t		get_uint32(uint32_t byte, uint16_t endian);
int32_t			get_int32(int32_t byte, uint16_t endian);
uint64_t		get_uint64(uint64_t byte, uint16_t endian);

int	encrypt_file(t_file *file);

#endif