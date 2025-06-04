#include "cube3d.h"

int	ft_isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

int parse_color(char *color_str, int *color)
{
	char *str;
	int	rgb[3];
	int i;
	int current_color;
	str = color_str;
	while (*str == ' ')
		str++;
	clean_line_ending(str);
	i = 0;
	current_color = 0;
	rgb[0] = rgb [1] = rgb[2] = 0;
	while (str[i] && current_color < 3)
	{
		if (ft_isdigit(str[i]))
			rgb[current_color] = rgb[current_color] * 10 + (str[i] - '0');
		else if (str[i] == ',')
			current_color++;
		else if (str[i] != ' ')
			return (printf("Error: Invalid RGB format\n"), 0);
		i++;
	}
	if (current_color != 2 || rgb[0] > 255 || rgb[1] > 255 || rgb[2] > 255)
		return (printf("Error: Invalid RGB values (0-255)\n"), 0);
	*color = (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
	return (1);
}

int parse_texture(char *path_str, char **texture_path)
{
	char *path;
	int fd;
	int len;
	path = path_str;
	while (*path_str == ' ')
		path_str++;
	clean_line_ending(path);
	len = ft_strlen(path);
	if (len < 4)
		return (printf("Error: Texture path too short: %s\n", path), 0);
	if (path[len - 4] != '.' || path[len - 3] != 'x' || path[len - 2] != 'p' || path[len - 1] != 'm')
		return (printf("Error: Texture must be .xpm format: %s\n", path), 0);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (printf("Error: Cannot open texture: %s\n", path), 0);
	close(fd);
	*texture_path = ft_strdup(path);
	return (*texture_path != NULL);
}

int parse_cub_file(char *filename, t_game *game)
{
	int fd;
	char *line;
	t_scene_data scene;

	ft_memset(&scene, 0 , sizeof(t_scene_data));
	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return (printf("Error: Cannot open %s\n", filename), 0);
	line = get_next_line(fd);
	while (line)
	{
		if (!process_line(line, &scene, game))
		{
			printf("Error in line: %s", line);
			free(line);
			close(fd);
			return (0);
		}
		free(line);
		line = get_next_line(fd);
	}
	close (fd);
	return (finalize_parsing(&scene, game));
}

int process_line(char *line , t_scene_data *scene, t_game *game)
{
	char *line_content;

	line_content = line;
	while (*line_content == ' ' || *line_content == '\t')
		line_content++;
	if (*line_content == '\0' || *line_content == '\n')
		return (1);
	if (scene->map_started)
		return (add_map_line(line_content, game, content));
	if (ft_strncmp(line_content, "NO ", 3) == 0)
		return (parse_texture(line_content + 3, &scene->north_texture));
	if (ft_strncmp(line_content, "SO ", 3) == 0)
		return (parse_texture(line_content + 3, &scene->south_texture));
	if (ft_strncmp(line_content, "WE ", 3) == 0)
		return (parse_texture(line_content + 3, &scene->west_texture));
	if (ft_strncmp(line_content, "EA ", 3) == 0)
		return (parse_texture(line_content + 3, &scene->east_texture));
	if (ft_strncmp(line_content, "F ", 2) == 0)
		return (parse_color(line_content + 2, &scene->floor_color));
	if (ft_strncmp(line_content, "C ", 2) == 0)
		return (parse_color(line_content + 2, &scene->ceiling_color));
	scene->map_started = 1;
	return (add_map_line(line_content, game, scene));
}

int add_map_line(char *line, t_game *game, t_scene_data *scene)
{
	clean_line_ending(line);
	if (!game->map.matrix)
	{
		game->map.matrix = malloc(sizeof(char *) * 1000);
		if (!game->map.matrix)
			return (printf("Error: Memory allocation failed\n"), 0);
	}
	game->map.matrix[scene->map_count] = ft_strdup(line);
	if (!game->map.matrix[scene->map_count])
		return (printf("Error: Memory allocation failed\n"), 0);
	scene->map_count++;
	return (1);
}
void clean_line_ending(char *line)
{
	int len = ft_strlen(line);
	if (len > 0 && line[len - 1] == '\n')
		line[len - 1] = '\0';
}

int finalize_parsing(t_scene_data *scene, t_game *game)
{
	t_texture_paths	paths;

	if (!scene->north_texture || !scene->south_texture || 
		!scene->west_texture || !scene->east_texture)
		return (printf("Error: Missing texture(s)\n"), 0);
	if (scene->map_count == 0)
		return (printf("Error: No map found\n"), 0);
	
	// Termine la matrice et calcule dimensions
	game->map.matrix[scene->map_count] = NULL;
	game->map.height = scene->map_count;
	game->map.width = calculate_map_width(game);
	game->map.floor_color = scene->floor_color;
	game->map.sky_color = scene->ceiling_color;
	
	// Initialise tout le jeu
	if (!init_mlx_window(game))
		return (printf("Error: MLX init failed\n"), 0);
	init_player(&game->player);
	game->player.game = game;
	if (!set_player_pos(game))
		return (printf("Error: No player position found\n"), 0);
	
	// Charge les textures
	paths.north = scene->north_texture;
	paths.south = scene->south_texture;
	paths.east = scene->east_texture;
	paths.west = scene->west_texture;
	if (!load_directional_textures(game, &paths))
		return (printf("Error: Failed to load textures\n"), 0);
	
	// Charge le reste du jeu
	game->current_weapon = HANDS;
	if (!load_all_weapons(game) || !load_weapon_pickup_sprites(game) || 
		!set_weapon_positions(game) || !load_open_door_sprites(game) || 
		!set_open_door_positions(game) || !init_all_enemies(game))
		return (printf("Error: Failed to load game assets\n"), 0);
	
	init_portals(game);
	init_ui_components(game);
	printf("✅ Game initialized!\n");
	return (1);
}
