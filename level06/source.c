#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ptrace.h>

/*
 * Level06 (32-bit) is a login/serial check with a self-debugging guard.
 * auth() hashes the login and requires the serial to equal that hash. The
 * hash is fully deterministic, so instead of breaking anything at runtime we
 * just reimplement it and compute a valid serial for a login of our choice.
 *
 * The ptrace(0,0,1,0) call is anti-debug: if a debugger is already attached
 * it returns -1 and the program bails. Computing the serial offline avoids it.
 */

int	auth(char *login_str, int given_serial)
{
	login_str[strcspn(login_str, "\n")] = 0;
	int	login_len = strnlen(login_str, 32);

	if (login_len > 5)
	{
		/* Anti-debug: ptrace(PTRACE_TRACEME,...) -- -1 means already traced. */
		if (ptrace(0, 0, (void *)1, 0) == -1)
		{
			puts("\033[32m.----------------------------.");
			puts("\033[31m| !! TAMPERING DETECTED !!  |");
			puts("\033[32m'----------------------------'");
			return (1);
		}

		/* Seed: 0x1337 = 4919, 0x5eeded = 6221293. */
		int	acc = (login_str[3] ^ 4919) + 6221293;
		int	pos = 0;
		while (pos < login_len)
		{
			if (login_str[pos] <= 31)	/* reject non-printable input */
				return (1);
			/*
			 * The binary inlines a magic-number division that is exactly a
			 * modulo 1337 (0x539); each step folds one byte into the hash.
			 */
			acc += (acc ^ login_str[pos]) % 1337;
			pos++;
		}

		if (given_serial != acc)
			return (1);
		return (0);
	}
	return (1);
}

int	main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	puts("***********************************");
	puts("*\t\tlevel06\t\t  *");
	puts("***********************************");
	printf("-> Enter Login: ");
	char	login_str[32];
	fgets(login_str, 32, stdin);

	puts("***********************************");
	puts("***** NEW ACCOUNT DETECTED ********");
	puts("***********************************");
	printf("-> Enter Serial: ");
	int		given_serial;
	scanf("%u", &given_serial);

	if (auth(login_str, given_serial) == 0)
	{
		puts("Authenticated!");
		system("/bin/sh");
		return (0);
	}
	return (1);
}
