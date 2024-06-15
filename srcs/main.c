#include "woody.h"

int	parse_header(t_file *file)
{
	Elf64_Ehdr *header = file->ptr;
	unsigned char *ident = header->e_ident;

	file->arch = ident[EI_CLASS];
	file->endian = ident[EI_DATA];

	if (memcmp(ident, ELFMAG, 4))
		return (-1);
	if (ident[EI_CLASS] == ELFCLASSNONE || ident[EI_DATA] == ELFDATANONE || ident[EI_VERSION] == EV_NONE)
		return (-1);
	if (header->e_version == EV_NONE || header->e_phnum != 0)
		return (-1);
	if (header->e_type != ET_EXEC || header->e_type != ET_DYN)
		return (-1);

	return (0);
}

int check_file(char *name, t_file *file)  
{
	if ((file->fd = open(name, O_RDONLY)) == -1)
		return (-1);
	if ((file->size = lseek(file->fd, 0, SEEK_END)) == (off_t)-1)
		return (-1);
	if ((file->ptr = mmap(0, file->size, PROT_READ | PROT_WRITE, MAP_PRIVATE, file->fd, 0)) == MAP_FAILED)
		return (-1);
	file->end = file->ptr + file->size;
	return (parse_header(file));
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		dprintf(2, "Woody Woodpacker: Usage: %s <file>\n", argv[0]);
		return (1);
	}

	t_file file;
	memset(&file, 0, sizeof(t_file));

	if (!check_file(argv[1], &file))
	{
		dprintf(2, "Woody Woodpacker: Error: %s is not a valid file\n", argv[1]);
		return (1);
	}

	printf("Woody Woodpacker: File %s is valid\n", argv[1]);

	printf("endian: %d\n", file.endian);
	printf("arch: %d\n", file.arch);
	printf("size: %ld\n", file.size);
	
}