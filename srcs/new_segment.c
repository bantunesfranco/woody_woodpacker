/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   new_segment.c                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/15 20:37:55 by bfranco       #+#    #+#                 */
/*   Updated: 2025/03/15 20:51:22 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"

static Elf64_Off find_segment_space(t_file *file)
{
    Elf64_Ehdr *ehdr = file->ptr;
    Elf64_Phdr *phdr = file->ptr + ehdr->e_phoff;
    Elf64_Off last_offset = 0;

    // Find the last segment's end
    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Off seg_end = phdr[i].p_offset + phdr[i].p_filesz;
        if (seg_end > last_offset)
            last_offset = seg_end;
    }

    // Align to page size
    long page_size = 4096;
    return (last_offset + page_size - 1) & ~(page_size - 1);
}

void *create_new_segment(t_file *file, size_t size)
{
    if (file->arch == ELFCLASS64)
    {
        Elf64_Ehdr *ehdr = file->ptr;
        Elf64_Phdr *phdr = file->ptr + ehdr->e_phoff;
        
        // Find space for new segment
        Elf64_Off new_offset = find_segment_space(file);
        
        // Create new program header
        Elf64_Phdr new_phdr = {
            .p_type = PT_LOAD,
            .p_flags = PF_R | PF_X,
            .p_offset = new_offset,
            .p_vaddr = new_offset + 0x400000,
            .p_paddr = new_offset + 0x400000,
            .p_filesz = size,
            .p_memsz = size,
            .p_align = 4096
        };

        // Add new program header
        Elf64_Phdr *last_phdr = &phdr[ehdr->e_phnum];
        *last_phdr = new_phdr;
        ehdr->e_phnum++;

        printf("Debug: Created new segment at offset 0x%lx\n", new_offset);
        return file->ptr + new_offset;
    }
	else
	    return NULL;
}