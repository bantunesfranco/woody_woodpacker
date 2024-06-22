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
	if (get_uint16(header->e_type, file->endian) != ET_EXEC && get_uint16(header->e_type, file->endian) != ET_DYN)
		return (-1);
	if (get_uint16(header->e_machine, file->endian) != EM_X86_64 && get_uint16(header->e_machine, file->endian) != EM_386)
		return (-1);
	if (get_uint32(header->e_version, file->endian) == EV_NONE)
		return (-1);
	if (get_uint16(header->e_phnum, file->endian) == 0)
		return (-1);

	printf("Magic: %c%c%c%c\n", ident[0], ident[1], ident[2], ident[3]);
	printf("Class: %s\n", ident[EI_CLASS] == ELFCLASS32 ? "32-bit" : "64-bit");
	printf("Type: %s\n", get_uint16(header->e_type, file->endian) == ET_EXEC ? "Executable" : "Shared object");
	printf("Machine: %s\n", get_uint16(header->e_machine, file->endian) == EM_X86_64 ? "x86-64" : "x86");
	printf("Version: %d\n", get_uint32(header->e_version, file->endian));
	printf("Program header table entries: %d\n", get_uint16(header->e_phnum, file->endian));
	printf("Program header table offset: %p\n", (void*)get_uint64(header->e_phoff, file->endian));
	printf("Entry point address: %p\n\n", (void*)get_uint64(header->e_entry, file->endian));

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
	file->end = (void*)((uint32_t*)file->ptr + file->size);
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

	if (check_file(argv[1], &file) == -1)
	{
		dprintf(2, "Woody Woodpacker: Error: %s is not a valid file\n", argv[1]);
		return (1);
	}

	if (encrypt_file(&file) == -1)
		return (1);

	return (0);
}