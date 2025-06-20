#include "woody.h"

int validate_file(char *name, t_file *file)  
{
	off_t size;
	if ((file->fd = open(name, O_RDONLY)) == -1)
		return (-1);
	if ((size = lseek(file->fd, 0, SEEK_END)) == -1)
		return (-1);
	if ((file->ptr = mmap(0, size + 2*SHELLCODE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, file->fd, 0)) == MAP_FAILED)
		return (-1);
	file->size = size;
	return (validate_elf(file));
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		dprintf(2, "Woody Woodpacker: Usage: %s <file>\n", argv[0]);
		return (1);
	}

	t_file file = {0};

	if (validate_file(argv[1], &file) == -1)
	{
		dprintf(2, "Woody Woodpacker: Error: %s is not a valid file\n", argv[1]);
		return (1);
	}

	t_payload payload;

	if (inject_payload(&file, &payload) == -1)
	{
		dprintf(2, "Woody Woodpacker: Error: Failed to inject payload\n");
		return (1);
	}

	encrypt_file(&file, &payload);
	create_new_executable(&file);
	return (0);
}

