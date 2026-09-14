#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * Level08 (64-bit) is a "backup" tool running SUID level09. Given a filename,
 * it opens that file for reading and copies its contents into ./backups/<name>.
 *
 * The privilege leak: it opens argv[1] with level09's rights, so if argv[1]
 * points (through a symlink) at level09's .pass, the tool reads the secret and
 * drops a copy into ./backups/ that we can read.
 *
 * (There is also a format-string bug: log_wrapper hands the filename straight
 * to snprintf as a format string. The symlink route is simpler, so we use it.)
 */

void	log_wrapper(FILE *log_file, char *prefix, char *name)
{
	char	line[264];

	strcpy(line, prefix);
	snprintf(&line[strlen(line)], 254 - strlen(line), name);	/* fmt-string bug */
	line[strcspn(line, "\n")] = 0;
	fprintf(log_file, "LOG: %s\n", line);
	return ;
}

int	main(int argc, char **argv)
{
	if (argc != 2)
		printf("Usage: %s filename\n", argv[0]);

	FILE	*log_fp = fopen("./backups/.log", "w");
	if (log_fp == 0)
	{
		printf("ERROR: Failed to open %s\n", "./backups/.log");
		exit(1);
	}
	log_wrapper(log_fp, "Starting back up: ", argv[1]);

	/* Opened with level09's rights -- a symlink here leaks the .pass. */
	FILE	*src_fp = fopen(argv[1], "r");
	if (src_fp == 0)
	{
		printf("ERROR: Failed to open %s\n", argv[1]);
		exit(1);
	}

	/* Destination is forced under ./backups/ (prefix blocks absolute paths). */
	char	dest_path[100];
	strcpy(dest_path, "./backups/");
	strncat(dest_path, argv[1], 99 - (strlen(dest_path)) - 1);

	int		out_fd;
	out_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, 0660);	/* 0xc1, 0660 */
	if (out_fd < 0)
	{
		printf("ERROR: Failed to open %s%s\n", "./backups/", argv[1]);
		exit(1);
	}

	int		byte;
	while ((byte = fgetc(src_fp)) != EOF)
		write(out_fd, &byte, 1);
	log_wrapper(log_fp, "Finished back up ", argv[1]);

	fclose(src_fp);
	close(out_fd);
	return (0);
}
