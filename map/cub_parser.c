#include "../cube3d.h"

int	parse_cub_file(char *filename, t_game *game)
{
	int				fd;
	char			*line;
	t_scene_data	scene;

	printf("DEBUG: Starting parse_cub_file\n");
	ft_memset(&scene, 0, sizeof(t_scene_data));
	
	printf("DEBUG: Opening file: %s\n", filename);
	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return (printf("Error: Cannot open %s\n", filename), 0);
	
	printf("DEBUG: Allocating matrix\n");
	game->map.matrix = malloc(sizeof(char *) * 1000);
	if (!game->map.matrix)
		return (close(fd), 0);
	
	printf("DEBUG: Starting to read lines\n");
	line = get_next_line(fd);
	while (line)
	{
		printf("DEBUG: Processing line: %s", line);
		if (!process_line(line, &scene, game))
		{
			printf("DEBUG: process_line failed\n");
			return (free(line), close(fd), 0);
		}
		free(line);
		line = get_next_line(fd);
	}
	
	printf("DEBUG: Finished reading, validating\n");
	close(fd);
	return (validate_and_load_scene(&scene, game));
}

int	process_line(char *line, t_scene_data *scene, t_game *game)
{
	char	*trimmed;

	trimmed = line;
	while (*trimmed == ' ' || *trimmed == '\t')
		trimmed++;
	if (*trimmed == '\0' || *trimmed == '\n')
	{
		printf("DEBUG: Empty line, skipping\n");
		return (1);
	}
	
	printf("DEBUG: map_started = %d\n", scene->map_started);
	
	if (scene->map_started)
	{
		printf("DEBUG: Adding map line: %s\n", trimmed);
		return (add_map_line(trimmed, game, scene));
	}
	if (ft_strncmp(trimmed, "NO ", 3) == 0)
	{
		printf("DEBUG: Found NO texture\n");
		return (parse_texture(trimmed + 3, &scene->north_texture));
	}
	if (ft_strncmp(trimmed, "SO ", 3) == 0)
	{
		printf("DEBUG: Found SO texture\n");
		return (parse_texture(trimmed + 3, &scene->south_texture));
	}
	if (ft_strncmp(trimmed, "WE ", 3) == 0)
	{
		printf("DEBUG: Found WE texture\n");
		return (parse_texture(trimmed + 3, &scene->west_texture));
	}
	if (ft_strncmp(trimmed, "EA ", 3) == 0)
	{
		printf("DEBUG: Found EA texture\n");
		return (parse_texture(trimmed + 3, &scene->east_texture));
	}
	if (ft_strncmp(trimmed, "F ", 2) == 0)
	{
		printf("DEBUG: Found F color\n");
		return (parse_color(trimmed + 2, &scene->floor_color));
	}
	if (ft_strncmp(trimmed, "C ", 2) == 0)
	{
		printf("DEBUG: Found C color\n");
		return (parse_color(trimmed + 2, &scene->ceiling_color));
	}
	
	printf("DEBUG: Unknown line, starting map: %s\n", trimmed);
	scene->map_started = 1;
	return (add_map_line(trimmed, game, scene));
}

int	parse_texture(char *path_str, char **texture_path)
{
	char	*path;
	int		fd;

	path = path_str;
	while (*path == ' ')
		path++;
	clean_line_ending(path);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (printf("Error: Texture not found: %s\n", path), 0);
	close(fd);
	*texture_path = ft_strdup(path);
	return (*texture_path != NULL);
}

int	parse_color(char *color_str, int *color)
{
	char	*str;
	int		rgb[3];
	int		i;
	int		val_idx;

	str = color_str;
	while (*str == ' ')
		str++;
	clean_line_ending(str);
	i = 0;
	val_idx = 0;
	rgb[0] = rgb[1] = rgb[2] = 0;
	while (str[i] && val_idx < 3)
	{
		if (ft_isdigit(str[i]))
			rgb[val_idx] = rgb[val_idx] * 10 + (str[i] - '0');
		else if (str[i] == ',')
			val_idx++;
		else if (str[i] != ' ')
			return (printf("Error: Invalid RGB format\n"), 0);
		i++;
	}
	if (val_idx != 2 || rgb[0] > 255 || rgb[1] > 255 || rgb[2] > 255)
		return (printf("Error: Invalid RGB values\n"), 0);
	*color = (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
	return (1);
}

int add_map_line(char *line, t_game *game, t_scene_data *scene)
{
	printf("DEBUG: add_map_line called with: '%s', count = %d\n", line, scene->map_count);
	clean_line_ending(line);
	game->map.matrix[scene->map_count] = ft_strdup(line);
	if (!game->map.matrix[scene->map_count])
	{
		printf("DEBUG: ft_strdup failed\n");
		return (0);
	}
	scene->map_count++;
	printf("DEBUG: map_count now = %d\n", scene->map_count);
	return (1);
}

// map/cub_parser.c
int	validate_and_load_scene(t_scene_data *scene, t_game *game)
{
	printf("DEBUG: validate_and_load_scene - checking textures\n");
	printf("DEBUG: north_texture = %p\n", scene->north_texture);
	printf("DEBUG: south_texture = %p\n", scene->south_texture);
	printf("DEBUG: west_texture = %p\n", scene->west_texture);
	printf("DEBUG: east_texture = %p\n", scene->east_texture);

	if (!scene->north_texture || !scene->south_texture || 
		!scene->west_texture || !scene->east_texture)
		return (printf("Error: Missing textures\n"), 0);

	printf("DEBUG: Setting map matrix end\n");
	game->map.matrix[scene->map_count] = NULL;
	
	printf("DEBUG: Setting map height to %d\n", scene->map_count);
	game->map.height = scene->map_count;
	
	printf("DEBUG: Calling calculate_map_width\n");
	game->map.width = calculate_map_width(game);
	printf("DEBUG: Map width = %d\n", game->map.width);
	
	// Stocker les chemins des textures dans la structure game pour les charger plus tard
	game->texture_paths.north = scene->north_texture;
	game->texture_paths.south = scene->south_texture;
	game->texture_paths.east = scene->east_texture;
	game->texture_paths.west = scene->west_texture;
	game->map.floor_color = scene->floor_color;
	game->map.sky_color = scene->ceiling_color;
	
	printf("DEBUG: validate_and_load_scene success\n");
	return (1);
}