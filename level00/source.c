#include <stdio.h>
#include <stdlib.h>

/*
 * Level00 is a warm-up: no memory corruption at all.
 * The binary reads one integer and compares it to a hard-coded value.
 * Since it runs SUID level01, entering the right number drops us into a
 * shell owned by level01.
 */
int	main(void)
{
	int	entered_pin;

	puts("***********************************");
	puts("* \t     -Level00 -\t\t  *");
	puts("***********************************");
	printf("Password:");

	scanf("%d", &entered_pin);

	/* cmp eax,0x149c  ->  the magic value is 0x149c == 5276. */
	if (entered_pin != 5276)
	{
		puts("\nInvalid Password!");
		return (1);
	}

	puts("\nAuthenticated!");
	system("/bin/sh");
	return (0);
}
