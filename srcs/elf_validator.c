/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   elf_validator.c                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/15 13:33:14 by bfranco       #+#    #+#                 */
/*   Updated: 2025/03/15 20:52:04 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"

static int validate_program_headers64(void *file_data, size_t file_size)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr*)file_data;
	if (ehdr->e_phoff >= file_size || ehdr->e_phentsize != 56)
		return 0;

	size_t ph_table_size = ehdr->e_phnum * ehdr->e_phentsize;
	if (ehdr->e_phoff + ph_table_size > file_size)
		return 0;

	Elf64_Phdr *phdr = (Elf64_Phdr *)(file_data + ehdr->e_phoff);
	for (size_t i = 0; i < ehdr->e_phnum; i++)
	{
		if (phdr[i].p_offset >= file_size
			|| phdr[i].p_offset + phdr[i].p_filesz > file_size)
			return 0;

		switch (phdr[i].p_type)
		{
			case PT_NULL:
			case PT_LOAD:
			case PT_DYNAMIC:
			case PT_INTERP:
			case PT_NOTE:
			case PT_SHLIB:
			case PT_PHDR:
			case PT_TLS:
				break;
			default:
				// Allow for processor-specific and OS-specific segments
				if (phdr[i].p_type < PT_LOOS || phdr[i].p_type > PT_HIPROC)
				{
					return 0;
				}
		}

		if ((phdr[i].p_flags & (PF_R | PF_W | PF_X)) == 0 && phdr[i].p_type != PT_NULL)
			return 0;

		if (phdr[i].p_align & (phdr[i].p_align - 1))
			return 0;
	}

	return 1;
}

static int validate_program_headers32(void *file_data, size_t file_size)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr*)file_data;
	if (ehdr->e_phoff >= file_size || ehdr->e_phentsize != 56)
		return 0;

	size_t ph_table_size = ehdr->e_phnum * ehdr->e_phentsize;
	if (ehdr->e_phoff + ph_table_size > file_size)
		return 0;

	Elf32_Phdr *phdr = (Elf32_Phdr *)(file_data + ehdr->e_phoff);
	for (size_t i = 0; i < ehdr->e_phnum; i++)
	{
		if (phdr[i].p_offset >= file_size
			|| phdr[i].p_offset + phdr[i].p_filesz > file_size)
			return 0;

		switch (phdr[i].p_type)
		{
			case PT_NULL:
			case PT_LOAD:
			case PT_DYNAMIC:
			case PT_INTERP:
			case PT_NOTE:
			case PT_SHLIB:
			case PT_PHDR:
			case PT_TLS:
				break;
			default:
				// Allow for processor-specific and OS-specific segments
				if (phdr[i].p_type < PT_LOOS || phdr[i].p_type > PT_HIPROC)
					return 0;
		}

		if ((phdr[i].p_flags & (PF_R | PF_W | PF_X)) == 0 && phdr[i].p_type != PT_NULL)
			return 0;

		if (phdr[i].p_align & (phdr[i].p_align - 1))
			return 0;
	}

	return 1;
}

static int validate_section_headers64(void *file_data, size_t file_size)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr*)file_data;
	if (ehdr->e_shoff >= file_size || ehdr->e_shentsize != 64)
		return 0;

	size_t sh_table_size = ehdr->e_shnum * ehdr->e_shentsize;
	if (ehdr->e_shoff + sh_table_size > file_size)
		return 0;

	if (ehdr->e_shstrndx >= ehdr->e_shnum && ehdr->e_shstrndx != SHN_UNDEF)
		return 0;

	Elf64_Shdr *shdr = (Elf64_Shdr *)(file_data + ehdr->e_shoff);
	for (size_t i = 1; i < ehdr->e_shnum; i++)
	{
		if (shdr[i].sh_offset >= file_size
			|| (shdr[i].sh_type != SHT_NOBITS
			&& shdr[i].sh_offset + shdr[i].sh_size > file_size))
			return 0;

		switch (shdr[i].sh_type)
		{
			case SHT_NULL:
			case SHT_PROGBITS:
			case SHT_SYMTAB:
			case SHT_STRTAB:
			case SHT_RELA:
			case SHT_HASH:
			case SHT_DYNAMIC:
			case SHT_NOTE:
			case SHT_NOBITS:
			case SHT_REL:
			case SHT_SHLIB:
			case SHT_DYNSYM:
			case SHT_INIT_ARRAY:
			case SHT_FINI_ARRAY:
			case SHT_PREINIT_ARRAY:
			case SHT_GROUP:
			case SHT_SYMTAB_SHNDX:
				break;
			default:
				// Allow for processor-specific and OS-specific sections
				if ((shdr[i].sh_type < SHT_LOOS || shdr[i].sh_type > SHT_HIOS)
					&& (shdr[i].sh_type < SHT_LOPROC || shdr[i].sh_type > SHT_HIPROC))
					return 0;
		}

		if (shdr[i].sh_addralign & (shdr[i].sh_addralign - 1))
			return 0;

		if (shdr[i].sh_name >= shdr[ehdr->e_shstrndx].sh_size)
			return 0;
	}

	return 1;
}

static int validate_section_headers32(void *file_data, size_t file_size)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr*)file_data;
	if (ehdr->e_shoff >= file_size || ehdr->e_shentsize != 64)
		return 0;

	size_t sh_table_size = ehdr->e_shnum * ehdr->e_shentsize;
	if (ehdr->e_shoff + sh_table_size > file_size)
		return 0;

	if (ehdr->e_shstrndx >= ehdr->e_shnum && ehdr->e_shstrndx != SHN_UNDEF)
		return 0;

	Elf32_Shdr *shdr = (Elf32_Shdr *)(file_data + ehdr->e_shoff);
	for (size_t i = 1; i < ehdr->e_shnum; i++)
	{
		if (shdr[i].sh_offset >= file_size ||
			(shdr[i].sh_type != SHT_NOBITS && shdr[i].sh_offset + shdr[i].sh_size > file_size))
			return 0;

		switch (shdr[i].sh_type)
		{
			case SHT_NULL:
			case SHT_PROGBITS:
			case SHT_SYMTAB:
			case SHT_STRTAB:
			case SHT_RELA:
			case SHT_HASH:
			case SHT_DYNAMIC:
			case SHT_NOTE:
			case SHT_NOBITS:
			case SHT_REL:
			case SHT_SHLIB:
			case SHT_DYNSYM:
			case SHT_INIT_ARRAY:
			case SHT_FINI_ARRAY:
			case SHT_PREINIT_ARRAY:
			case SHT_GROUP:
			case SHT_SYMTAB_SHNDX:
				break;
			default:
				// Allow for processor-specific and OS-specific sections
				if ((shdr[i].sh_type < SHT_LOOS || shdr[i].sh_type > SHT_HIOS)
					&& (shdr[i].sh_type < SHT_LOPROC || shdr[i].sh_type > SHT_HIPROC))
					return 0;
		}

		if (shdr[i].sh_addralign & (shdr[i].sh_addralign - 1))
			return 0;

		if (ehdr->e_shstrndx != SHN_UNDEF && 
			shdr[i].sh_name >= shdr[ehdr->e_shstrndx].sh_size)
			return 0;
	}

	return 1;
}

static int validate_program_headers(void *file_data, size_t file_size, int arch)
{
	switch (arch)
	{
		case ELFCLASS32:
			return validate_program_headers32(file_data, file_size);
		case ELFCLASS64:
			return validate_program_headers64(file_data, file_size);
		default:
			dprintf(2, "Woody Woodpacker: Error: Invalid program headers class\n");
			return 0;
	}
}

static int validate_section_headers(void *file_data, size_t file_size, int arch)
{
	switch (arch)
	{
		case ELFCLASS32:
			return validate_section_headers32(file_data, file_size);
		case ELFCLASS64:
			return validate_section_headers64(file_data, file_size);
		default:
			dprintf(2, "Woody Woodpacker: Error: Invalid section headers class\n");
			return 0;
	}
}

static int validate_headers(t_file *file)
{
	if (!validate_program_headers(file->ptr, file->size, file->arch))
		return 0;

	if (!validate_section_headers(file->ptr, file->size, file->arch))
		return 0;

	return 1;
}

void	print_sections(t_file *file)
{
	if (file->arch == ELFCLASS32)
	{
		Elf32_Ehdr *header = file->ptr;
		Elf32_Shdr *shstrtab = get_section(file, header->e_shstrndx);
		char *strtab = (void*)header + shstrtab->sh_offset;

		for (int i = 0; i < header->e_shnum; ++i)
		{
			Elf32_Shdr *section = get_section(file, i);
			printf("Section %d\n", i);
			printf("Name: %s\n", strtab + section->sh_name);
			printf("Size: %d\n", section->sh_size);
		}
	}
	else
	{
		Elf64_Ehdr *header = file->ptr;
		Elf64_Shdr *shstrtab = get_section(file, header->e_shstrndx);
		char *strtab = (void*)header + shstrtab->sh_offset;

		for (int i = 0; i < header->e_shnum; ++i)
		{
			Elf64_Shdr *section = get_section(file, i);
			printf("Section %d\n", i);
			printf("Name: %s\n", strtab + section->sh_name);
			printf("Size: %ld\n", section->sh_size);
		}
	}
}

int	validate_elf(t_file *file)
{
	Elf64_Ehdr *header = file->ptr;
	unsigned char *ident = header->e_ident;

	file->arch = ident[EI_CLASS];

	if (memcmp(ident, ELFMAG, 4))
		return (-1);
	if (ident[EI_CLASS] == ELFCLASSNONE || ident[EI_DATA] != ELFDATA2LSB || ident[EI_VERSION] == EV_NONE)
		return (-1);
	if (header->e_type != ET_EXEC && header->e_type != ET_DYN)
		return (-1);
	if (header->e_machine != EM_X86_64 && header->e_machine != EM_386)
		return (-1);
	if (header->e_version == EV_NONE)
		return (-1);
	if (!validate_headers(file))
		return (-1);

	printf("Magic: %c%c%c%c\n", ident[0], ident[1], ident[2], ident[3]);
	printf("Class: %s\n", ident[EI_CLASS] == ELFCLASS32 ? "32-bit" : "64-bit");
	printf("Type: %s\n", header->e_type == ET_EXEC ? "Executable" : "Shared object");
	printf("Machine: %s\n", header->e_machine == EM_X86_64 ? "x86-64" : "x86");
	printf("Version: %d\n", header->e_version);
	printf("Program header table entries: %d\n", header->e_phnum);
	printf("Program header table offset: %p\n", (void*)header->e_phoff);
	printf("Entry point address: %p\n\n", (void*)header->e_entry);

	// print_sections(file);
	return (0);
}