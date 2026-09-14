/*
 * Helper to print the runtime address of an environment variable, as seen
 * from a target binary of a given name. Compile and run it with the SAME
 * argv[0] length as the target so the stack layout (and thus the address)
 * matches.
 *
 *   gcc -m32 getenv.c -o getenv
 *   ./getenv SHELLCODE ./level05
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int	main(int argc, char **argv)
{
	char	*ptr;

	if (argc < 2)
	{
		fprintf(stderr, "usage: %s <ENV_VAR> [target_name]\n", argv[0]);
		return (1);
	}
	ptr = getenv(argv[1]);
	if (!ptr)
	{
		fprintf(stderr, "%s not found in environment\n", argv[1]);
		return (1);
	}
	/* Address in this process; adjust for the target's argv[0] length. */
	printf("%s is at %p\n", argv[1], ptr);
	return (0);
}
