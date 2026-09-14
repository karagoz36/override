#include <stdio.h>
#include <string.h>

/*
 * Level01 is a fake "admin login". The real bug is the size mismatch on the
 * password read: the stack buffer is 64 bytes but fgets is told 100, which
 * lets us smash the saved return address and run a ret2libc attack.
 */

char	a_user_name[256];

int	verify_user_name(void)
{
	puts("verifying username....\n");
	/* Username must be exactly "dat_wil". */
	return (strncmp(a_user_name, "dat_wil", 7));
}

int	verify_user_pass(char *candidate)
{
	/* Would require "admin"... but the caller never really enforces it. */
	return (strncmp(candidate, "admin", 5));
}

int	main(void)
{
	/* Saved EIP sits 80 bytes past the start of this 64-byte buffer. */
	char	pass_field[64];

	bzero(pass_field, 64);

	puts("********* ADMIN LOGIN PROMPT *********");
	puts("Enter Username: ");
	fgets(a_user_name, 256, stdin);

	if (verify_user_name() == 0)
	{
		puts("Enter Password: ");

		/* BUG: up to 100 bytes read into a 64-byte buffer. */
		fgets(pass_field, 100, stdin);

		/*
		 * (X == 0 || X != 0) is always true, so the password value is
		 * irrelevant - what matters is that fgets already overflowed.
		 */
		if (verify_user_pass(pass_field) == 0 || verify_user_pass(pass_field) != 0)
			puts("nope, incorrect password...\n");
	}
	else
	{
		puts("nope, incorrect username...\n");
	}

	return (0);
}
