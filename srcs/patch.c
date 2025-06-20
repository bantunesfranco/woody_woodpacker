/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   patch.c                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/19 19:23:30 by bfranco       #+#    #+#                 */
/*   Updated: 2025/03/21 19:57:27 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"
#include <assert.h>

static int64_t get_padding(t_file *file, void *seg1)
{
	char* seg1_end = 0;
	
	if (!seg1)
		return (-1);
	
	if (file->arch == ELFCLASS32)
	seg1_end = (char*)(file->ptr + ((Elf32_Phdr*)seg1)->p_offset + ((Elf32_Phdr*)seg1)->p_filesz);
	else if (file->arch == ELFCLASS64)
	seg1_end = (char*)(file->ptr + ((Elf64_Phdr*)seg1)->p_offset + ((Elf64_Phdr*)seg1)->p_filesz);
	
	int64_t pad = 0;
	while (*(seg1_end+pad) == 0)
		++pad;
	return (pad);
}

static void *find_free_space(t_file *file, size_t size, t_location *patch_location)
{
	void *seg = get_load_segment(file);
	int64_t pad = get_padding(file, seg);
	
	if (pad == -1)
		return (NULL);

	*patch_location = LOAD;
	if ((size_t)pad >= size)
		return (file->arch == ELFCLASS32
			? (char*)file->ptr + ((Elf32_Phdr*)seg)->p_offset + ((Elf32_Phdr*)seg)->p_filesz
			: (char*)file->ptr + ((Elf64_Phdr*)seg)->p_offset + ((Elf64_Phdr*)seg)->p_filesz);

	*patch_location = NOTE;
	seg = use_note_segment(file, size);
	if (!seg)
	{
		*patch_location = BSS;
		seg = use_bss_segment(file, size);
	} 
	if (!seg)
	{
		*patch_location = NEW;
		seg = create_new_segment(file, size);
	}
	if (!seg)
		return (NULL);
	
	return (seg);
}

void *patch(t_file *file, t_payload *payload, int *patch_location)
{
	if (generate_encription_key(payload) == -1)
	{
		dprintf(2, "Woody Woodpacker: Error: Failed to generate encryption key\n");
		return (NULL);
	}

	payload->text_size = file->arch == ELFCLASS32 ? ((Elf32_Shdr*)file->text_sec)->sh_size : ((Elf64_Shdr*)file->text_sec)->sh_size;
	payload->segment_offset = file->arch == ELFCLASS32 ? ((Elf32_Phdr*)file->load_seg)->p_memsz : ((Elf64_Phdr*)file->load_seg)->p_memsz;

	if (file->arch == ELFCLASS32)
	{
		payload->entry_offset = ((Elf32_Phdr*)file->load_seg)->p_vaddr + ((Elf32_Phdr*)file->load_seg)->p_memsz - ((Elf32_Ehdr*)file->ptr)->e_entry;
		payload->text_offset = ((Elf32_Phdr*)file->load_seg)->p_vaddr + ((Elf32_Phdr*)file->load_seg)->p_memsz - ((Elf32_Shdr*)file->text_sec)->sh_addr;
	}
	else if (file->arch == ELFCLASS64)
	{
		payload->entry_offset = ((Elf64_Phdr*)file->load_seg)->p_vaddr + ((Elf64_Phdr*)file->load_seg)->p_memsz - ((Elf64_Ehdr*)file->ptr)->e_entry;
		payload->text_offset = ((Elf64_Phdr*)file->load_seg)->p_vaddr + ((Elf64_Phdr*)file->load_seg)->p_memsz - ((Elf64_Shdr*)file->text_sec)->sh_addr;
	}

	void *payload_addr = find_free_space(file, SHELLCODE_SIZE, (t_location*)patch_location);
	if (!payload_addr)
	{
		dprintf(2, "Woody Woodpacker: Error: Failed to find free space\n");
		return (NULL);
	}

	return (payload_addr);
}