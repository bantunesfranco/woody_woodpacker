/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   encryption.c                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/06/22 16:47:02 by bfranco       #+#    #+#                 */
/*   Updated: 2024/06/22 17:19:00 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"

int	encrypt_32(t_file *file)
{
	(void)file;
	return 0;
}

int	encrypt_64(t_file *file)
{
	(void)file;
	return 0;
}

int	encrypt_file(t_file *file)
{
	int ret = -1;

	switch (file->arch)
	{
		case ELFCLASS32:
			ret = encrypt_32(file);
			break;
		case ELFCLASS64:
			ret = encrypt_64(file);
			break;
		default:
			dprintf(2, "Woody Woodpacker: Error: Unsupported architecture\n");
			break;
	}
	return (ret);
}