/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   texture_loader_2.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abkhefif <abkhefif@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 14:33:28 by abkhefif          #+#    #+#             */
/*   Updated: 2025/06/03 14:33:29 by abkhefif         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cube3d.h"

int	load_all_textures(t_game *game)
{
	if (!load_basic_textures(game))
		return (0);
	if (!load_door_textures(game))
		return (0);
	if (!load_special_textures(game))
		return (0);
	return (1);
}
