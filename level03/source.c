#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

/*
 * Level03 hides a XOR check behind some arithmetic.
 * main() reads an integer and calls test(guess, 322424845).
 * test() derives derived_key = 322424845 - guess. Only a small set of keys
 * is accepted (a switch); any other key decrypts with a random byte, a dead
 * end. decrypt() XORs a fixed ciphertext with the key and, when it becomes
 * "Congratulations!", spawns a shell.
 */

void	decrypt(int key)
{
	char	cipher_text[] = "Q}|u`sfg~sf{}|a3";
	size_t	length;
	size_t	i;

	length = strlen(cipher_text);
	i = 0;
	while (i < length)
	{
		cipher_text[i] = cipher_text[i] ^ key;
		i++;
	}

	if (strncmp(cipher_text, "Congratulations", 17) == 0)
		system("/bin/sh");
	else
		puts("\nInvalid Password");
	return ;
}

void	test(int guess, int magic)
{
	int	derived_key;

	derived_key = magic - guess;
	/* The binary accepts only these keys; everything else -> random byte. */
	switch (derived_key)
	{
		case 1: case 2: case 3: case 4: case 5:
		case 6: case 7: case 8: case 9:
		case 16: case 17: case 18: case 19: case 20: case 21:
			decrypt(derived_key);
			break ;
		default:
			decrypt(rand());
			break ;
	}
	return ;
}

int	main(void)
{
	int	guess;

	srand(time(0));

	puts("***********************************");
	puts("*\t\tlevel03\t\t**");
	puts("***********************************");
	printf("Password:");

	scanf("%d", &guess);
	test(guess, 322424845);
	return (0);
}
