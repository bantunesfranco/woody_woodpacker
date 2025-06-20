/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   elf_utils.c                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/06/25 11:16:45 by bfranco       #+#    #+#                 */
/*   Updated: 2025/03/15 20:32:44 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"

void	*get_segment_by_type(t_file *file, uint32_t type)
{
	if (file->arch == ELFCLASS32)
	{
		Elf32_Ehdr *ehdr = file->ptr;
		Elf32_Phdr *phdr = file->ptr + ehdr->e_phoff;

		for (int i = 0; i < ehdr->e_phnum; i++) {
			if (phdr[i].p_type == type)
				return &phdr[i];
		}
	}
	else
	{
		Elf64_Ehdr *ehdr = file->ptr;
		Elf64_Phdr *phdr = file->ptr + ehdr->e_phoff;

		for (int i = 0; i < ehdr->e_phnum; i++) {
			if (phdr[i].p_type == type)
				return &phdr[i];
		}
	}
	return NULL;
}

void	*get_section(t_file *file, uint16_t i)
{
	if (file->arch == ELFCLASS32)
	{
		Elf32_Ehdr *header = file->ptr;
		Elf32_Shdr *section = file->ptr + header->e_shoff;
		return (void*)&section[i];
	}
	else
	{
		Elf64_Ehdr *header = file->ptr;
		Elf64_Shdr *section = file->ptr + header->e_shoff;
		return (void*)&section[i];
	}

	return (NULL);
}

void	*get_section_by_name(t_file *file, char *name)
{
	if (file->arch == ELFCLASS32)
	{
		Elf32_Ehdr *header = file->ptr;
		Elf32_Shdr *shstrtab = get_section(file, header->e_shstrndx);
		char *strtab = file->ptr + shstrtab->sh_offset;

		for (int i = 0; i < header->e_shnum; ++i)
		{
			Elf32_Shdr *section = get_section(file, i);
			if (!section)
				break;
			char *section_name = strtab + section->sh_name;
			if (!strncmp(name, section_name, strlen(name) + 1))
				return section;
		}
	}
	else
	{
		Elf64_Ehdr *header = file->ptr;
		Elf64_Shdr *shstrtab = get_section(file, header->e_shstrndx);
		char *strtab = file->ptr + shstrtab->sh_offset;

		for (int i = 0; i < header->e_shnum; ++i)
		{
			Elf64_Shdr *section = get_section(file, i);
			if (!section)
				break;
			char *section_name = strtab + section->sh_name;
			if (!strncmp(name, section_name, strlen(name) + 1))
				return section;
		}
	}

	dprintf(2, "Woody Woodpacker: Error: %s section not found\n", name);
	return (NULL);
}

static int	sanitize_load_segment(t_file *file, void *segment)
{
	if (file->arch == ELFCLASS32)
	{
		Elf32_Phdr *phdr = segment;

		return (phdr->p_type == PT_LOAD && (phdr->p_flags & PF_X) && phdr->p_filesz && phdr->p_offset
				&& phdr->p_offset < file->size && phdr->p_filesz < file->size
				&& phdr->p_offset + phdr->p_filesz < file->size
				&& phdr->p_vaddr < ((Elf32_Ehdr*)file)->e_entry
				&& phdr->p_vaddr + phdr->p_memsz < ((Elf32_Ehdr*)file)->e_entry);
	}
	else
	{
		Elf64_Phdr *phdr = segment;

		return (phdr->p_type == PT_LOAD && (phdr->p_flags & PF_X) && phdr->p_filesz && phdr->p_offset
				&& phdr->p_offset < file->size && phdr->p_filesz < file->size
				&& phdr->p_offset + phdr->p_filesz < file->size
				&& phdr->p_vaddr < ((Elf64_Ehdr*)file)->e_entry
				&& phdr->p_vaddr + phdr->p_memsz < ((Elf64_Ehdr*)file)->e_entry);
	}

	return (0);
}

void	*get_load_segment(t_file *file)
{
	if (file->arch == ELFCLASS32)
	{
		Elf32_Ehdr *ehdr = file->ptr;
		Elf32_Phdr *phdr = file->ptr + ehdr->e_phoff;

		for (int i = 0; i < ehdr->e_phnum; i++) {
			if (sanitize_load_segment(file, &phdr[i]))
				return &phdr[i];
		}
	}
	else
	{
		Elf64_Ehdr *ehdr = file->ptr;
		Elf64_Phdr *phdr = file->ptr + ehdr->e_phoff;

		for (int i = 0; i < ehdr->e_phnum; i++) {
			if (sanitize_load_segment(file, &phdr[i]))
				return &phdr[i];
		}
	}
	return NULL;
}
