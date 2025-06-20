/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   code_cave.c                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/15 16:03:54 by bfranco       #+#    #+#                 */
/*   Updated: 2025/03/15 20:36:21 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"

static int validate_note_section(void *note_data, size_t size)
{
    if (size < sizeof(Elf64_Nhdr))
        return 0;

    Elf64_Nhdr *nhdr = (Elf64_Nhdr *)note_data;
    char *name = (char *)(note_data + sizeof(Elf64_Nhdr));

    return (nhdr->n_type == NT_GNU_ABI_TAG &&
            nhdr->n_namesz == 4 &&
            strncmp(name, "GNU", 4) == 0);
}

static void *get_note_segment(t_file *file)
{
    void *section = get_section_by_name(file, ".note.ABI-tag");
    if (!section)
        return NULL;

    if (file->arch == ELFCLASS64)
    {
        Elf64_Shdr *note_section = section;
        Elf64_Phdr *note_segment = get_segment_by_type(file, PT_NOTE);
        
        if (!note_segment)
            return NULL;

        void *note_data = file->ptr + note_section->sh_offset;
        if (!validate_note_section(note_data, note_section->sh_size))
            return NULL;

        return note_segment;
    }
    else
    {
        Elf32_Shdr *note_section = section;
        Elf32_Phdr *note_segment = get_segment_by_type(file, PT_NOTE);
        
        if (!note_segment)
            return NULL;

        void *note_data = file->ptr + note_section->sh_offset;
        if (!validate_note_section(note_data, note_section->sh_size))
            return NULL;

        return note_segment;
    }
}

static void	*get_segment_containing(t_file *file, Elf64_Addr addr)
{
	if (file->arch == ELFCLASS32)
	{
		Elf32_Ehdr *ehdr = file->ptr;
		Elf32_Phdr *phdr = file->ptr + ehdr->e_phoff;
		for (int i = 0; i < ehdr->e_phnum; i++) {
			if ((Elf32_Addr)addr >= phdr[i].p_vaddr && 
				(Elf32_Addr)addr < phdr[i].p_vaddr + phdr[i].p_memsz)
				return &phdr[i];
		}
	}
	else
	{
		Elf64_Ehdr *ehdr = file->ptr;
		Elf64_Phdr *phdr = file->ptr + ehdr->e_phoff;

		for (int i = 0; i < ehdr->e_phnum; i++) {
			if (addr >= phdr[i].p_vaddr && 
				addr < phdr[i].p_vaddr + phdr[i].p_memsz)
				return &phdr[i];
		}
	}
	return NULL;
}


static void	*make_segment_executable(t_file *file, void *section, void *segment)
{
	if (file->arch == ELFCLASS32)
	{
		((Elf32_Shdr*)section)->sh_flags |= SHF_EXECINSTR | SHF_ALLOC;
		((Elf32_Phdr*)segment)->p_type = PT_LOAD;
		((Elf32_Phdr*)segment)->p_flags |= PF_X | PF_R;
		return file->ptr + ((Elf32_Shdr*)section)->sh_offset;
	}
	else
	{
		((Elf64_Shdr*)section)->sh_flags |= SHF_EXECINSTR | SHF_ALLOC;
		((Elf64_Phdr*)segment)->p_type = PT_LOAD;
		((Elf64_Phdr*)segment)->p_flags |= PF_X | PF_R;
		return file->ptr + ((Elf64_Shdr*)section)->sh_offset;
	}
	return NULL;
}

void	*use_note_segment(t_file *file, size_t size)
{
	void *section = get_section_by_name(file, ".note.ABI-tag");
	if (!section)
		return NULL;

	file->payload_section = section;
	if (file->arch == ELFCLASS32)
	{
		Elf32_Shdr *note_section = section;
		Elf32_Phdr *note_segment = get_note_segment(file);
	
		if (!note_segment || note_section->sh_size < size)
			return NULL;

		return make_segment_executable(file, note_section, note_segment);
	}
	else
	{
		Elf64_Shdr *note_section = section;
		Elf64_Phdr *note_segment = get_note_segment(file);

		if (!note_segment || note_section->sh_size < size)
			return NULL;

		return make_segment_executable(file, note_section, note_segment);
	}
	return NULL;
}

void	*use_bss_segment(t_file *file, size_t size)
{
	void *section = get_section_by_name(file, ".bss");
	if (!section)
		return NULL;

	file->payload_section = section;
	if (file->arch == ELFCLASS32)
	{
		Elf32_Shdr *bss_section = section;
		Elf32_Phdr *bss_segment = get_segment_containing(file, bss_section->sh_addr);

		if (!bss_segment || bss_section->sh_size < size)
			return NULL;

		bss_section->sh_type = SHT_PROGBITS;
		memset(file->ptr + bss_section->sh_offset, 0, bss_section->sh_size);
		bss_segment->p_filesz = bss_segment->p_memsz;


		return make_segment_executable(file, bss_section, bss_segment);
	}
	else
	{
		Elf64_Shdr *bss_section = section;
		Elf64_Phdr *bss_segment = get_segment_containing(file, bss_section->sh_addr);

		if (!bss_segment || bss_section->sh_size < size)
			return NULL;

		bss_section->sh_type = SHT_PROGBITS;
		memset(file->ptr + bss_section->sh_offset, 0, bss_section->sh_size);
		bss_segment->p_filesz = bss_segment->p_memsz;

		return make_segment_executable(file, bss_section, bss_segment);
	}
	return NULL;
}