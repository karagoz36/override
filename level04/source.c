#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/prctl.h>	/* PR_SET_PDEATHSIG (Linux) */

/*
 * Level04 (32-bit) is a gets() overflow with an anti-shellcode twist:
 * main() forks. The CHILD runs gets() into a small buffer. The PARENT uses
 * ptrace to watch the child's syscalls and kills it the instant the child
 * issues execve (syscall 11). So injecting execve shellcode fails.
 *
 * The escape: ret2libc into system("/bin/sh"). system() forks its own
 * grandchild to run the exec, and the parent only traces our direct child,
 * so the grandchild's exec is never seen.
 */
int	main(void)
{
	int		worker = fork();
	char	shell_input[32];
	int		peeked = 0;
	int		wait_status = 0;

	bzero(shell_input, 32);

	if (worker == 0)
	{
		/* Child: die with the parent, then submit to being traced. */
		prctl(PR_SET_PDEATHSIG, SIGKILL);
		ptrace(PTRACE_TRACEME, 0, 0, 0);

		puts("Give me some shellcode, k");
		gets(shell_input);	/* BUG: unbounded read -> stack overflow */
		return (0);
	}

	/*
	 * Parent: PTRACE_PEEKUSR at user-area offset 44 reads the child's
	 * ORIG_EAX (the attempted syscall number). 11 == execve.
	 */
	while ((peeked = ptrace(PTRACE_PEEKUSR, worker, 44, 0)) != 11)
	{
		wait(&wait_status);
		if (WIFEXITED(wait_status) || WIFSIGNALED(wait_status))
		{
			puts("child is exiting...");
			return (0);
		}
	}
	puts("no exec() for you");
	kill(worker, 9);
	return (0);
}
