#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

/*
 * Level09 (bonus, 64-bit) is a fake messaging app built around one struct:
 *
 *   struct s_note { char message_body[140]; char sender[40]; int body_len; };
 *
 * set_username copies the sender with an OFF-BY-ONE (i <= 40), writing one
 * byte past sender[] into body_len. By overwriting body_len with a big value
 * we make set_msg's strncpy copy far more than 140 bytes -- overflowing
 * message_body[] and smashing the saved return address. We point it at the
 * hidden secret_backdoor(), which runs whatever we type through system().
 */

struct	s_note
{
	char	message_body[140];	/* offset 0   */
	char	sender[40];		/* offset 140 */
	int		body_len;		/* offset 180 */
};

void	secret_backdoor(void)
{
	char	cmd_line[128];

	/* Whatever we feed here is handed straight to system(). */
	fgets(cmd_line, 128, stdin);
	system(cmd_line);
	return ;
}

void	set_username(struct s_note *note)
{
	char	input_line[128];

	bzero(input_line, 128);
	puts(">: Enter your username");
	printf(">>: ");
	fgets(input_line, 128, stdin);

	/* BUG: i <= 40 copies 41 bytes, the 41st landing in body_len. */
	int		i = 0;
	while (i <= 40 && input_line[i] != 0)
	{
		note->sender[i] = input_line[i];
		i++;
	}

	printf(">: Welcome, %s", note->sender);
	return ;
}

void	set_msg(struct s_note *note)
{
	char	input_line[1024];

	bzero(input_line, 1024);
	puts(">: Msg @Unix-Dude");
	printf(">>: ");
	fgets(input_line, 1024, stdin);
	/* body_len (corrupted above) controls how much we copy -> overflow. */
	strncpy(note->message_body, input_line, note->body_len);
	return ;
}

void	handle_msg(void)
{
	struct s_note	note;

	bzero(note.sender, 40);
	note.body_len = 140;	/* default -- we overwrite this */
	set_username(&note);
	set_msg(&note);
	puts(">: Msg sent!");
	return ;
}

int	main(void)
{
	puts("--------------------------------------------"
		 "\n|   ~Welcome to l33t-m$n ~	v1337		|\n"
		 "--------------------------------------------");
	handle_msg();
	return (0);
}
