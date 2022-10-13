all: sshell_123

example: sshell.c
	
		gcc -g -Wall -Wextra -Werror -o sshell_123 sshell_123.c

clean:
	
		rm -f sshell_123