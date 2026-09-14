#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * Level05 (32-bit, NX enabled) is a tiny "to lowercase" filter. It reads a
 * line, lowercases the uppercase letters, prints it with printf(line) -- a
 * format-string bug -- and calls exit(0).
 *
 * Because the stack is non-executable, we can't run stack shellcode. Instead:
 *   1) put execve("/bin/sh") shellcode (behind a NOP sled) in an env var;
 *   2) use the format string to overwrite exit@GOT with that address, so the
 *      final exit(0) jumps into our shellcode.
 *
 * Only uppercase letters are transformed, so our address bytes and format
 * specifiers (never uppercase ASCII) pass through untouched.
 */
int	main(void)
{
	char	line[100];
	int		pos;

	fgets(line, 100, stdin);
	pos = 0;
	while (pos < strlen(line))
	{
		if (line[pos] > 0x40 && line[pos] <= 0x5a)
			line[pos] = line[pos] ^ 0x20;	/* uppercase -> lowercase */
		pos++;
	}
	printf(line);	/* BUG: user-controlled format string */
	exit(0);		/* target: exit@GOT redirected to our shellcode */
}
