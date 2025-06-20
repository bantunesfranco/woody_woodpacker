/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   inject.c                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/06/25 17:51:22 by bfranco       #+#    #+#                 */
/*   Updated: 2025/03/21 22:55:17 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"

void	create_new_executable(t_file *file)
{
	char *new_name = "woody";
	int fd = open(new_name, O_CREAT | O_RDWR, 0755);
	lseek(file->fd, 0, SEEK_SET);
	write(fd, file->ptr, file->size);
	close(fd);
	munmap(file->ptr, file->size + 2*SHELLCODE_SIZE);
	close(file->fd);
}

void set_entrypoint(t_file *file, int patch_location)
{
	if (patch_location == LOAD)
	{
		if (file->arch == ELFCLASS32)
		{
			((Elf32_Ehdr*)file->ptr)->e_entry = ((Elf32_Phdr*)file->load_seg)->p_vaddr + ((Elf32_Phdr*)file->load_seg)->p_filesz;
			((Elf32_Phdr*)file->load_seg)->p_memsz += SHELLCODE_SIZE;
			((Elf32_Phdr*)file->load_seg)->p_filesz += SHELLCODE_SIZE;
			
		}
		else if (file->arch == ELFCLASS64)
		{
			((Elf64_Ehdr*)file->ptr)->e_entry = ((Elf64_Phdr*)file->load_seg)->p_vaddr +  ((Elf64_Phdr*)file->load_seg)->p_filesz;
			printf("Woody Woodpacker: new entry point: %#lx\n", ((Elf64_Ehdr*)file->ptr)->e_entry);
			((Elf64_Phdr*)file->load_seg)->p_memsz += SHELLCODE_SIZE;
			((Elf64_Phdr*)file->load_seg)->p_filesz += SHELLCODE_SIZE;
		}
	}
	else
	{
		if (file->arch == ELFCLASS32)
			((Elf32_Ehdr*)file->ptr)->e_entry = ((Elf32_Phdr*)file->payload_section)->p_vaddr;
		else if (file->arch == ELFCLASS64)
			((Elf64_Ehdr*)file->ptr)->e_entry = ((Elf64_Phdr*)file->payload_section)->p_vaddr;
	}
}

static int	get_shellcode(char *shellcode)
{
	int fd = open("./stub", O_RDONLY);
	if (fd == -1)
	{
		dprintf(2, "Woody Woodpacker: Error: Failed to open stub\n");
		return -1;
	}
	
	ssize_t bytes_read = read(fd, shellcode, SHELLCODE_SIZE);
	close(fd);
	if (bytes_read == -1)
	{
		dprintf(2, "Woody Woodpacker: Error: Failed to read stub\n");
		return -1;
	}

	return 0;
}


static int verify_injection(t_file *file, void *payload_addr)
{
    if (file->arch == ELFCLASS64)
    {
        Elf64_Phdr *load = (Elf64_Phdr*)file->load_seg;
        
        // Verify segment permissions
        if (!(load->p_flags & PF_X))
        {
            dprintf(2, "Error: Segment not executable\n");
            return -1;
        }
        
        // Verify payload is within segment bounds
		printf("load addr %p\n",(char*)file->ptr + load->p_offset);
		printf("payload addr %p\n", payload_addr);
		printf("load_end addr %p\n", (char*)file->ptr + load->p_offset + load->p_filesz);
        if ((char*)payload_addr < (char*)file->ptr + load->p_offset ||
            (char*)payload_addr + SHELLCODE_SIZE > (char*)file->ptr + load->p_offset + load->p_filesz)
        {
            dprintf(2, "Error: Payload outside segment bounds\n");
            return -1;
        }
    }
    return 0;
}


int	inject_payload(t_file *file, t_payload *payload)
{
	file->text_sec = get_section_by_name(file, ".text");
	file->load_seg = get_load_segment(file);

	if (!file->text_sec || !file->load_seg)
	{
		dprintf(2, "Woody Woodpacker: Error: Failed to get text section or load segment\n");
		return (-1);
	}

	int patch_location;
	void *payload_addr = patch(file, payload, &patch_location);
	if (!payload_addr)
	{
		dprintf(2, "Woody Woodpacker: Error: Failed to patch payload\n");
		return (-1);
	}

	char shellcode[SHELLCODE_SIZE];
	if (get_shellcode((char*)shellcode) != 0)
	{
		dprintf(2, "Woody Woodpacker: Error: Failed to get shellcode\n");
		return (-1);
	}

	memcpy(payload_addr, shellcode, SHELLCODE_SIZE);
	memcpy(payload_addr + PATCH_OFFSET, payload, sizeof(t_payload));
	
	printf("Woody Woodpacker: Successfully injected payload into %s\n", patch_location == LOAD ? "LOAD" : patch_location == NOTE ? "NOTE" : "BSS");

	set_entrypoint(file, patch_location);

	if (verify_injection(file, payload_addr) != 0)
		return -1;
	return (0);
}