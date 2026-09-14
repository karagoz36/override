#include <stdio.h>
#include <string.h>

/*
 * Level07 (32-bit) is "wil's number storage service": a menu that stores and
 * reads unsigned ints in an on-stack table with NO bounds checking. It only
 * rejects "reserved" indices (index % 3 == 0) and numbers whose top byte is
 * 183. argv and env are wiped, so shellcode has nowhere to live.
 *
 * The bug: slots[index] with an attacker-controlled unsigned index is an
 * arbitrary stack write -- including main's saved return address. We build a
 * ret2libc frame there, using an unsigned overflow on the index to dodge the
 * "% 3" guard.
 */

void	clear_stdin(void)
{
	char	ch = 0;

	while (1)
	{
		ch = getchar();
		if (ch == '\n' || ch == EOF)
			break ;
	}
}

unsigned int	get_unum(void)
{
	unsigned int	value;

	fflush(stdout);
	scanf("%u", &value);
	clear_stdin();
	return (value);
}

int	store_number(int *slots)
{
	unsigned int	value = 0;
	unsigned int	slot = 0;

	printf(" Number: ");
	value = get_unum();
	printf(" Index: ");
	slot = get_unum();

	/* "Reserved" guard -- bypassed with an overflowing index (see walkthrough). */
	if (slot % 3 == 0 || (value >> 24) == 183)
	{
		puts(" *** ERROR! ***");
		puts("   This index is reserved for wil!");
		puts(" *** ERROR! ***");
		return (1);
	}

	slots[slot] = value;	/* BUG: slot is unbounded */
	return (0);
}

int	read_number(int *slots)
{
	unsigned int	slot = 0;

	printf(" Index: ");
	slot = get_unum();
	printf(" Number at data[%u] is %u\n", slot, slots[slot]);
	return (0);
}

int	main(int argc, char **argv, char **env)
{
	int		cmd_status = 0;
	char	command[20] = {0};
	int		slots[100] = {0};

	(void)argc;
	/* Wipe argv and env so no shellcode can be smuggled through them. */
	for (int i = 0; argv[i] != 0; i++)
		memset(argv[i], 0, strlen(argv[i]) - 1);
	for (int i = 0; env[i] != 0; i++)
		memset(env[i], 0, strlen(env[i]) - 1);

	puts("----------------------------------------------------\n"
		 "  Welcome to wil's crappy number storage service!   \n"
		 "----------------------------------------------------\n"
		 " Commands:                                          \n"
		 "    store - store a number into the data storage    \n"
		 "    read  - read a number from the data storage     \n"
		 "    quit  - exit the program                        \n"
		 "----------------------------------------------------\n"
		 "   wil has reserved some storage :>                 \n"
		 "----------------------------------------------------\n");

	while (1)
	{
		printf("Input command: ");
		cmd_status = 1;
		fgets(command, 20, stdin);
		command[strlen(command) - 1] = 0;
		if (strncmp("store", command, 5) == 0)
			cmd_status = store_number(slots);
		else if (strncmp("read", command, 4) == 0)
			cmd_status = read_number(slots);
		else if (strncmp("quit", command, 4) == 0)
			return (0);
		if (cmd_status != 0)
			printf(" Failed to do %s command\n", command);
		else
			printf(" Completed %s command successfully\n", command);
		bzero(command, 20);
	}
	return (0);
}
