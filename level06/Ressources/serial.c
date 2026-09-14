/*
 * Reproduces level06's hashing algorithm to compute the serial for any login.
 *
 *   gcc serial.c -o serial
 *   ./serial username      ->  serial for 'username' = 6234463
 *
 * The login must be at least 6 printable characters (matches the binary's
 * strnlen(login,32) > 5 and the "<= 31" printable check).
 */
#include <stdio.h>
#include <string.h>

int	main(int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "usage: %s <login>\n", argv[0]);
		return (1);
	}

	const char		*login = argv[1];
	size_t			len = strlen(login);
	unsigned int	hash = (login[3] ^ 0x1337) + 0x5eeded;	/* seed */

	for (size_t i = 0; i < len; i++)
	{
		if ((unsigned char)login[i] <= 31)
			return (1);
		hash += (hash ^ (unsigned char)login[i]) % 1337;
	}

	printf("serial for '%s' = %u\n", login, hash);
	return (0);
}
