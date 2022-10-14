#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<errno.h>
#include <unistd.h>
#include<fcntl.h>
#include<sys/wait.h>gezac

#define CMDLINE_MAX 512
#define ENVPATH "/bin/"
char command[CMDLINE_MAX][CMDLINE_MAX];
void clearCommand()
{
	int i = 0;
	for(i = 0; i < CMDLINE_MAX; i++)
		memset(command[i],'\0',CMDLINE_MAX);

}
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
		printf("Error: cannot cd into directory\n");
		return 1;
	}
}

void clearbuf()
{
	char ch;
	while((ch = getchar())!=EOF&&ch !='\n');
}
int main(void)
{
        char cmd[CMDLINE_MAX];
        while(1)
	{
		memset(cmd,'\0',CMDLINE_MAX);
		clearCommand();
		char *nl;
                int retval;
		
                /* Print prompt */
                printf("sshell@ucd$ ");
                fflush(stdout);
		//clearbuf();
                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)*nl = '\0';

		/* Split Command and count */
		int commandnum = splitcommand(cmd);
		int fd[2];
		if(pipe(fd) < 0)
		{
			printf("pipe create fail\n");
			return 0;
		}	
		int pid;
		char *mcmd[255]={NULL};
		int i = 0;
		int flag = 0;
		int j = 0;
		while(command[i][0]!='\0')
		{
			if(strcmp("|",command[i])==0 || strcmp(">",command[i])==0 || strcmp(">>",command[i])==0)
			{
				
				if(strcmp("|",command[i])==0)
					flag = 1;
				if(strcmp(">",command[i])==0)
					flag = 2;
				if(strcmp(">>",command[i])==0)
					flag = 3;
				i+=1;
				while(command[i][0]!='\0')
				{
					mcmd[j++] = command[i];
					i++;
				}	
				mcmd[j]=NULL;
			}
			i++;
		}				
		//parents process
		if((pid = fork()) > 0)
		{
			wait(NULL);
			if(flag != 0)
			{
				close(fd[1]);
				close(STDIN_FILENO);
				dup(fd[0]);
				char pathname[255]="";
				strcpy(pathname,ENVPATH);
				strcat(pathname,mcmd[0]);
				if(flag == 1)
				{
					if(execv(pathname,mcmd) < 0)
					{
						fprintf(stderr,"Parent:%s\n",strerror(errno));
						exit(0);	
					}
				}
				char buf[1024]="";
				read(fd[0],buf,1024);
				if(flag == 2)
				{
					int fp = open(mcmd[0],O_RDWR|O_CREAT|O_TRUNC,0777);
					if(fp < 0)
					{
						fprintf(stderr,"open fail\n");
						return 0;	
					}
					write(fp,buf,1024);			
					break;
				}
				if(flag == 3)
				{
					int fp = open(mcmd[0],O_RDWR|O_CREAT|O_APPEND,0777);
					if(fp < 0)
					{
						fprintf(stderr,"open fail\n");
						return 0;	
					}
					write(fp,buf,1024);
					break;
				}
			}			
		}
		else if(pid == 0)//child process
		{
                	/* Builtin command */
			if (commandnum != 0)
			{
				if (strcmp(command[0], "exit") == 0) 
				{
				//	fprintf(stderr, "Bye...\n");
				//	fprintf(stderr, "+ completed 'exit' [0]\n");
					exit(0);
				}
				if (strcmp(command[0],"pwd") == 0)
				{
					pwd();
					fprintf(stderr, "+ completed 'pwd' [0]\n");	
				} 
				else if (strcmp(command[0],"cd") == 0)
				{
					int result = cd(commandnum);
					fprintf(stderr, "+ completed '%s' [%d]\n", cmd, result);
				}
				else//recall
				{
					if(flag != 0)
					{
						close(fd[0]);
						close(STDOUT_FILENO);
						dup(fd[1]);
					}
					char* scmd[255] ={NULL};
					int i = 0,j = 0;
					while(command[i][0] != '\0')
					{
						if(strcmp(command[i],"|")==0 || strcmp(command[i],">")==0 || strcmp(command[i],">>")==0)
						{
							scmd[i] == NULL;
							break;
						}
						scmd[i] = command[i];
						i++;
					}
					scmd[i+1] = NULL;
					char pathname[255] = "";
					strcpy(pathname,ENVPATH);
					strcat(pathname,scmd[0]);
					if(execv(pathname,scmd)<0)
					{
						fprintf(stderr,"child:error:%s\n",strerror(errno));
						exit(0);
					}
				}

			}
		}	
		if (strcmp(command[0], "exit") == 0) 
		{
			fprintf(stderr, "Bye...\n");
			fprintf(stderr, "+ completed 'exit' [0]\n");
			exit(0);
		}

        }
        return EXIT_SUCCESS;
}



