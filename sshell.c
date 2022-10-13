#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

char command[CMDLINE_MAX][CMDLINE_MAX];
int pipe_exist = -1;
int pipe_exit_result[3];

int pipecommand(int left, int right);
int redirectioncommand(int left, int right);

int splitcommand(char tmpcmd[CMDLINE_MAX]){
	int splitnum = 0;
	int len = strlen(tmpcmd);
	int i , j;
	for (i = 0, j = 0; i < len; i++){
		if (tmpcmd[i] != ' '){
			command[splitnum][j++] = tmpcmd[i];
		} else {
			if (j != 0){
				command[splitnum][j] = '\0';
				splitnum++;
				j = 0;
			}
		}
	}
	if (j != 0){
		command[splitnum][j] = '\0';
		splitnum++;
	}
	return splitnum;
}


void pwd(){
	char pwdstring[CMDLINE_MAX];
	getcwd(pwdstring,sizeof(pwdstring));
	printf("%s\n",pwdstring);
}

int cd(int commandnum){
	if (chdir(command[1]) >= 0){
		return 0;
	}else {
		fprintf(stderr,"Error: cannot cd into directory\n");
		return 1;
	}
}

int callcommand(int commandnum){
	pid_t pid = fork();
	if (pid == 0){
		int result = pipecommand(0, commandnum);
	} else if(pid > 0){
		int status;
		waitpid(pid, &status, 0);
		return WEXITSTATUS(status);
	} else {
		return 10; //ERROR_FORK
	}
}

int pipecommand(int left, int right){
	int result = 0;
	//int pipe_exist = -1;
	int pipe_count = 0;
	for (int i = left; i < right; i++){
		if (strcmp(command[i], "|") == 0){
			pipe_exist = i;
			pipe_count++;
			break;
		}
	}

	if (pipe_exist == -1){
		return redirectioncommand(left, right);
	} else if (pipe_exist+1 == right){
		return 11; //ERROR_MISSING_COMMAND
		pipe_exit_result[pipe_exist] = pipe_exist;
	}

	int fds[2];
	pipe(fds);
	pid_t pid = fork();
	if (pid < 0){
		result = 10;
	} else if (pid == 0){ // child process the single command
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO);
		close(fds[1]);
		result = redirectioncommand(left, pipe_exist);
		exit(result);
	} else { // parent precess the rest of command
		int status;
		waitpid(pid, &status, 0);
		WEXITSTATUS(status);

		if (pipe_exist < right){
			close(fds[1]);
			dup2(fds[0], STDIN_FILENO);
			close(fds[0]);
			result = pipecommand(pipe_exist+1, right);
		}
	}
}

int redirectioncommand(int left, int right){
	char *inFILE = NULL, *outFILE = NULL;
	int in_num = 0, out_num = 0;
	int end = right;

	for (int i = left; i < right; i++){
		if (strcmp(command[i], "<") == 0){
			in_num++;
			if (i+1 < right){
				inFILE = command[i+1];
			} else {
				return 12; // no input file;
			}
			if (end == right){
				end = i;
			}
		} else if (strcmp(command[i], ">") == 0){
			out_num++;
			if (i+1 < right){
				outFILE = command[i+1];
			} else {
				return 13;
			}
			if (end == right){
				end = i;
			}
		}
	}

	if (in_num == 1){
		int fp = open(inFILE, O_CREAT|O_WRONLY|O_TRUNC, 0777);
		if (fp < 0){
			return 14;
		}
	} else if (out_num == 1){
		int fp = open(outFILE, O_CREAT|O_WRONLY|O_TRUNC, 0777);
		if (fp < 0){
			return 15;
		}
	}

	if (in_num > 1){
		return 12;
	} else if (out_num > 1){
		return 13;
	}
	if (in_num == 1 && pipe_exist != -1){
		return 16;
	} else if (out_num == 1 && pipe_exist != -1){
		return 17;
	}

	int result = 0;
	pid_t pid = fork();
	if (pid < 0){
		result = 10;
	} else if (pid == 0){
		if (in_num == 1){
			freopen(inFILE, "r", stdin);
		}
		if (out_num == 1){
			freopen(outFILE, "w",stdout);
		}

		char* mockcommand[CMDLINE_MAX];
		for (int i = left; i < end; i++){
			mockcommand[i] = command[i];
		}
		mockcommand[end] = NULL;
		int retval = execvp(mockcommand[left],mockcommand+left);
		if (retval != 0){
			return 11;
			exit(1);
		}
	} else {
		int status;
		waitpid(pid, &status, 0);
		WEXITSTATUS(status);
	}
}

int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;

                /* Print prompt */
                printf("sshell@ucd$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

				/* Split Command and count */
				int commandnum = splitcommand(cmd);

                /* Builtin command */
				if (commandnum != 0){
					if (strcmp(command[0], "exit") == 0) {
							fprintf(stderr, "Bye...\n");
							fprintf(stderr, "+ completed 'exit' [0]\n");
							exit(0);
					} else if (strcmp(command[0],"pwd") == 0){
							pwd();
							fprintf(stderr, "+ completed 'pwd' [0]\n");	
					} else if (strcmp(command[0],"cd") == 0){
							int result = cd(commandnum);
							fprintf(stderr, "+ completed '%s' [%d]\n", cmd, result);
					} else {
							int result = callcommand(commandnum);
							if (result == 10){
								fprintf(stderr, "Error fork\n");
								break;								
							} else if (result == 11){
								fprintf(stderr, "Error: missing command\n");
								break;
							} else if (result == 12){
								fprintf(stderr, "Error: no input file\n");
								break;
							} else if (result == 13){
								fprintf(stderr, "Error: no output file\n");
								break;
							} else if (result == 14){
								fprintf(stderr, "Error: cannot open input file\n");
								break;
							} else if (result == 15){
								fprintf(stderr, "Error: cannot open output file\n");
								break;
							} else if (result == 16){
								fprintf(stderr, "Error: mislocated input redirection");
								break;
							} else if (result == 17){
								fprintf(stderr, "Error: mislocated output redirection");
								break;
							}
							fprintf(stderr, "+ completed '%s' [%d]\n", cmd, result);
					}
				} 

        }

        return EXIT_SUCCESS;
}


