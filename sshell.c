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
int pipe_count = 0;
int pushd_exist = -1;
char orginal_path[CMDLINE_MAX];
char push_path[CMDLINE_MAX];

int pipecommand(int left, int right);
int redirectioncommand(int left, int right);

/* This is the error management*/
enum {
	NORMAL_RESULT, // normal result
	ERROR_FORK, // when fork is error
	ERROR_MISSING_COMMAND, // when the command is missing
	ERROR_NOINPUT, // there is no input file 
	ERROR_NOOUTPUT, // there is no output file
	ERROR_NOINPUT_FILE, // input file is not exist
	ERROR_NOOUTPUT_FILE, // output file is not exist
	ERROR_MISCON_INPUT, // have the redirection input and the pipe
	ERROR_MISCON_OUTPUT // have the redirection output and the pipe

};

int splitcommand(char tmpcmd[CMDLINE_MAX]){
    int splitnum = 0;
    int len = strlen(tmpcmd);
    int i , j;
    for (i = 0, j = 0; i < len; i++){
	/* in for loop when it have ' ' we are split the command between them*/
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

/* This is the function that we get the current pwd*/
void pwd(){
    char pwdstring[CMDLINE_MAX];
    getcwd(pwdstring,sizeof(pwdstring));
    printf("%s\n",pwdstring);
}

/* cd function is to change the dirctory when we use cd command */
int cd(int commandnum){
    if (chdir(command[1]) >= 0){
        return 0;
    }else {
        fprintf(stderr,"Error: cannot cd into directory\n");
        return 1;
    }
}

/* This is the main command for the regular command*/
int callcommand(int commandnum){
    pid_t pid = fork();
	/* This is the children process */
    if (pid == 0){
        exit( pipecommand(0, commandnum));
    } else if(pid > 0){
		/* when there is a parent process we wait to the children process */
        int status;
        waitpid(pid, &status, 0);
        return  WEXITSTATUS(status);
    } else {
        return ERROR_FORK; //The fork has error
    }
}

/* This is the pipefunciton that can know there is a pipe or not */
int pipecommand(int left, int right){
    int result = 0;
	/* We define that no pipe exist is -1 */
    int pipe_exist = -1;
	int pipe_count;
	/* find the piep */
    for (int i = left; i < right; i++){
		/* we know the pipe when there is a '|' in  */
        if (strcmp(command[i], "|") == 0){
			/* put the poisition in command[] in the pipe_exist */
            pipe_exist = i;
            pipe_count++;
            break;
        }
    }
	/* When there is no pipe */
    if (pipe_exist == -1){
        return redirectioncommand(left, right);
		/* There is no command after '|' pipe */
    } else if (pipe_exist+1 == right){
        return ERROR_MISSING_COMMAND; //ERROR_MISSING_COMMAND
    }

    int fds[2];
    pipe(fds);
    pid_t pid = fork();
    if (pid < 0){
        result = 10;
    } else if (pid == 0){
		/* child process the single command */
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);
		/* we go to redirectioncommnad to check the redireciton command */
        result = redirectioncommand(left, pipe_exist);
        exit(result);
    } else {
		/* parent precess the rest of command */
        int status;
        waitpid(pid, &status, 0);
		/* put the exit value in the exit result arrey */
        WEXITSTATUS(status);

		/* keep doing the command since the pipe is not the end of the command*/
        if (pipe_exist < right){
            close(fds[1]);
            dup2(fds[0], STDIN_FILENO);
            close(fds[0]);
            result = pipecommand(pipe_exist+1, right);
        }
    }
    return result;
}

/* This is the redirectioncammand to check the redirection no pipe in 
and left is the begin of the command or the after the '|'
and right is the end of the command or the before of the next '|' */
int redirectioncommand(int left, int right){
    char *inFILE = NULL, *outFILE = NULL;
    int in_num = 0, out_num = 0;
    int end = right;

	/*  we check is there have the redirection */
    for (int i = left; i < right; i++){
        if (strcmp(command[i], "<") == 0){
            in_num++;
            if (i+1 < right){
                inFILE = command[i+1];
            } else {
		/* There's no files after '<' */
                return ERROR_NOINPUT; 
            }
            if (end == right){
                end = i;
            }
        } else if (strcmp(command[i], ">") == 0){
            out_num++;
            if (i+1 < right){
                outFILE = command[i+1];
            } else {
		/* There's no command after '>' */
                return ERROR_NOOUTPUT;
            }
            if (end == right){
                end = i;
            }
        }
    }

	/* the input file is not exist or cannot find  */
    if (in_num == 1){
        int fp = open(inFILE, O_RDONLY);
        if (fp < 0){
            return ERROR_NOINPUT_FILE;
        }
    } else if (out_num == 1){
	/* the output file is not exist or cannot fidn */
        int fp = open(outFILE, O_CREAT|O_WRONLY|O_TRUNC, 0777);
        if (fp < 0){
            return ERROR_NOOUTPUT_FILE;
        }
    }

	/* When the input or output cammand has more than one '> ' or '<' */
    if (in_num > 1){
        return ERROR_NOINPUT;
    } else if (out_num > 1){
        return ERROR_NOOUTPUT;
    }

	/* To check is there have the redirection command with pipe command */
    if (in_num == 1 && pipe_exist != -1){
        return ERROR_MISCON_INPUT;
    } else if (out_num == 1 && pipe_exist != -1){
        return ERROR_MISCON_OUTPUT;
    }

    int result = 0;
    pid_t pid = fork();
    if (pid < 0){
        result = 10;
    } else if (pid == 0){
		/* We are doing the redirection input and output */
        if (in_num == 1){
			int fp = open(inFILE, O_RDONLY);
            dup2(fp, STDIN_FILENO);
        }
        if (out_num == 1){
			int fp = open(outFILE, O_CREAT|O_WRONLY|O_TRUNC, 0777);
            dup2(fp, STDOUT_FILENO);
        }
	/* execv the command  */
	/* we have a mock command to set with pointer(*) */
        char* mockcommand[CMDLINE_MAX];
        for (int i = left; i < end; i++){
            mockcommand[i] = command[i];
        }
        mockcommand[end] = NULL;
        int retval = execvp(mockcommand[left],mockcommand+left);
		/* To check the command is missing or not */
        if (retval != 0){
            return ERROR_MISSING_COMMAND;
            //exit(1);
        }
		//exit(retval);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }

	return result;
}

/* This is the directory command*/
int directory (int commandnum){
	int result = 0;
	/* we remember the orginal path */
	if (pushd_exist == -1){
		getcwd(orginal_path,sizeof(orginal_path));
	}
	/* We set the pushd command and mark push exist */
	if (strcmp(command[0],"pushd") == 0){
		pushd_exist = 0;
		if (chdir(command[1]) >= 0){
			getcwd(push_path,sizeof(push_path));
			return 0;
		}else {
			fprintf(stderr,"Error: no such directory\n");
			return 1;
		}
		/* To print the dirs all pwd in push and orginal */
	} else if (strcmp(command[0],"dirs") == 0){
		if (pushd_exist == 0){
			fprintf(stderr,"%s\n",push_path);
			fprintf(stderr,"%s\n",orginal_path);
			return 0;
		} else {
			fprintf(stderr,"%s\n",orginal_path);
			return 0;
		}
	}else if (strcmp(command[0],"popd") == 0){
		/* error when there is no dirctory pushd */
		if (pushd_exist == -1){
			fprintf(stderr,"Error: directory stack empty\n");
			return 1;
		} else {
			/* we pop the dir when we have the push dictory  */
			chdir(orginal_path);
			pushd_exist = -1;
			return 0;
		}
	}

	return result;
}


int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
				int result; 

                /* Print prompt */
                printf("sshell@ucd@$ ");
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
					/* exit command */
                    if (strcmp(command[0], "exit") == 0) {
                            fprintf(stderr, "Bye...\n");
                            fprintf(stderr, "+ completed 'exit' [0]\n");
                            exit(0);
                    } else if (strcmp(command[0],"pwd") == 0){
						/* pwd command */
                            pwd();
                            fprintf(stderr, "+ completed 'pwd' [0]\n");
                    } else if (strcmp(command[0],"cd") == 0){
						/* cd command */
                            int result = cd(commandnum);
                            fprintf(stderr, "+ completed '%s' [%d]\n", cmd, result);
                    } else if (strcmp (command[0],"dirs") == 0 || strcmp(command[0],"pushd") == 0 || strcmp(command[0],"popd") == 0) {
							/* dirs, pushd ,popd command */
							int result = directory(commandnum);
							fprintf(stderr, "+ completed '%s' [%d]\n", cmd, result);
					} else {
						/* other regular command and the print the error informaiton */
                            result = callcommand(commandnum);
                            switch (result){
								case ERROR_FORK:
									fprintf(stderr, "Error fork\n");
									break;
								case ERROR_MISSING_COMMAND:
									fprintf(stderr, "Error: missing command\n");
									break;
								case ERROR_NOINPUT:
									fprintf(stderr, "Error: no input file\n");
									break;
								case ERROR_NOOUTPUT:
									fprintf(stderr, "Error: no output file\n");
									break;
								case ERROR_NOINPUT_FILE:
									fprintf(stderr, "Error: cannot open input file\n");
									break;
								case ERROR_NOOUTPUT_FILE:
									fprintf(stderr, "Error: cannot open output file\n");
									break;
								case ERROR_MISCON_INPUT:
									fprintf(stderr, "Error: mislocated input redirection");
									break;
								case ERROR_MISCON_OUTPUT:
									fprintf(stderr, "Error: mislocated output redirection");
									break;
									}
                            fprintf(stderr, "+ completed '%s' [%d]\n", cmd, result);
                    }
                }

        }

        return EXIT_SUCCESS;
}

