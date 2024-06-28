/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   elf.c                                              :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/06/25 11:16:45 by bfranco       #+#    #+#                 */
/*   Updated: 2024/06/28 11:37:17 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"

static void	*get_section(t_file *file, uint16_t i)
{
	if (file->arch == ELFCLASS32)
	{
		Elf32_Ehdr *header = file->ptr;
		Elf32_Shdr *sections = (void*)header + get_uint32(header->e_shoff, file->endian);
		return ((void*)&sections[i]);
	}
	else
	{
		Elf64_Ehdr *header = file->ptr;
		Elf64_Shdr *sections = (void*)header + get_uint64(header->e_shoff, file->endian);
		return ((void*)&sections[i]);
	}
}

void	*get_section_by_name(t_file *file, char *name)
{
	if (file->arch == ELFCLASS32)
	{
		Elf32_Ehdr *header = file->ptr;
		Elf32_Shdr *shstrtab = get_section(file, get_uint32(header->e_shstrndx, file->endian));
		char *strtab = (void*)header + get_uint32(shstrtab->sh_offset, file->endian);

		for (int i = 0; i < header->e_shnum; ++i)
		{
			Elf32_Shdr *section = get_section(file, i);
			char *section_name = strtab + get_uint32(section->sh_name, file->endian);
			if (!strncmp(name, section_name, strlen(name) + 1))
				return section;
		}
	}
	else
	{
		Elf64_Ehdr *header = file->ptr;
		Elf64_Shdr *shstrtab = get_section(file, get_uint64(header->e_shstrndx, file->endian));
		char *strtab = (void*)header + get_uint64(shstrtab->sh_offset, file->endian);

		for (int i = 0; i < header->e_shnum; ++i)
		{
			Elf64_Shdr *section = get_section(file, i);
			char* section_name = strtab + get_uint32(section->sh_name, file->endian);
			if (!strncmp(name, section_name, strlen(name) + 1))
				return section;
		}
	}

	dprintf(2, "Woody Woodpacker: Error: .text section not found\n");
	return (NULL);
}

void	*get_text_segment(t_file *file)
{
	if (file->arch == ELFCLASS32)
	{
		Elf32_Ehdr *header = file->ptr;
		Elf32_Phdr *phdr = (void*)header + get_uint32(header->e_phoff, file->endian);

		for (int i = 0; i < header->e_phnum; ++i)
		{
			if (phdr[i].p_type == PT_LOAD && phdr[i].p_flags & PF_X)
				return &phdr[i];
		}
	}
	else
	{
		Elf64_Ehdr *header = file->ptr;
		Elf64_Phdr *phdr = (void*)header + get_uint64(header->e_phoff, file->endian);

		for (int i = 0; i < header->e_phnum; ++i)
		{
			if (phdr[i].p_type == PT_LOAD && phdr[i].p_flags & PF_X)
				return &phdr[i];
		}
	}

	dprintf(2, "Woody Woodpacker: Error: .text segment not found\n");
	return (NULL);
}