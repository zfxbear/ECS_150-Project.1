all: sshell

example: sshell.c
	gcc -g -Wall -Wextra -Werror sshell.c -o sshell

clean:
	rm -f sshell
