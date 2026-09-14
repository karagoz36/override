#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Level02 (64-bit) reads level03's password off disk into a stack buffer,
 * then asks us to log in. The password check itself is sound, but on the
 * failure path it does printf(entered_name) -- a format-string bug that lets
 * us read the stack, where the real password is sitting.
 */
int	main(void)
{
	char	entered_name[100];	/* our username, later printed unsafely */
	char	secret_pass[48];	/* level03's real password, from the file  */
	char	entered_pass[112];	/* the password we type                    */
	int		bytes_read;
	FILE	*pass_fp;

	pass_fp = fopen("/home/users/level03/.pass", "r");
	if (!pass_fp)
	{
		fwrite("ERROR: failed to open password file\n", 1, 36, stderr);
		exit(1);
	}
	bytes_read = fread(&secret_pass, 1, 41, pass_fp);
	secret_pass[strcspn(secret_pass, "\n")] = 0;
	if (bytes_read != 41)
	{
		fwrite("ERROR: failed to read password file\n", 1, 36, stderr);
		exit(1);
	}
	fclose(pass_fp);

	puts("===== [ Secure Access System v1.0 ] =====");
	puts("/***************************************\\");
	puts("| You must login to access this system. |");
	puts("\\**************************************/");
	printf("--[ Username: ");
	fgets(entered_name, 100, stdin);
	entered_name[strcspn(entered_name, "\n")] = 0;
	printf("--[ Password: ");
	fgets(entered_pass, 100, stdin);
	entered_pass[strcspn(entered_pass, "\n")] = 0;
	puts("*****************************************");

	/* We don't need to guess -- we leak secret_pass through the bug below. */
	if (strncmp(secret_pass, entered_pass, 41))
	{
		/*
		 * BUG: entered_name is the format string. "%22$p"-style specifiers
		 * dump stack words, including the secret_pass buffer.
		 */
		printf(entered_name);
		puts(" does not have access!");
		exit(1);
	}

	printf("Greetings, %s!\n", entered_name);
	system("/bin/sh");
	return (0);
}
