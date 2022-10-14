# Report for sschell.c

### HanChen Yu & QiJun Liang

## Summary
This program `sschell.c`is a command-line interpreter. It takes input from the user in the form of a command line and executes them, which means support for output and input redirection and pipelining commands.

## Implementation
1.Using `fgets` to get the command input

2.Check command input is regular command or builtin command

3.Check whitespaces

4.Parse `pipe` and `redirect`

5.Parse the string to select the commands, arguments and redirection

6.Using the `fork()`,`exec()`and `wait` to display the output

## splitCommand, callCommand and pipeCommand
```c
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
```
In `splitCommand` uses `tmpcmd()` to put the characters that are automatically separated when a space is encountered into `callCommands`'s character group.

```c 
    int callcommand(int commandnum){
    pid_t pid = fork();
    if (pid == 0){
        exit( pipecommand(0, commandnum));
    } else if(pid > 0){
        int status;
        waitpid(pid, &status, 0);
        return  WEXITSTATUS(status);
    } else {
        return 10; //ERROR_FORK
    }
}
```
        
In `callCommand` using `fork()`to detects whether a subroutine is running, If `pid` > 0 will run the subroutine directly, or `pid` = 0 will run to `pipeCommand`, else will display `Error`.
```c
    int pipecommand(int left, int right){
    int result = 0;
    int pipe_exist = -1;
    int pipe_count = 0;
    for (int i = left; i < right; i++){
        if (strcmp(command[i], "|") == 0){
            pipe_exist = i;
            pipe_count++;
            break;
        }
    }
```
In `pipeCommand`.`left` represents the leftmost string, `right` represents the rightmost string. If `|` was encountered while reading the string will exist a `pipe = i`. `i` stands for the first pipe encountered.

## fork(), exec() and wait()
Changed from `system()` to `fork()`,`exec()`, and `wait()`. This is where the parent and child processes can be obtained using the method of bifurcation parameters. The parent process will wait for the child process to perform the `execvp` function judgment.

## Redirection
Create a function that checks a given command to see if it has a redirect symbol, and create a file that provides space for the output.

## Error management
```c
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
```
Creat a error functions, Put all the running errors in it. The error is identified using the `switch`, and after everythign runs done then put it back in the main function and display the error's name.

## Summary
It's important to understand how the `shell` is implemented, and it also means that there's a lot of space for `shell` improvement. Adding functionality to the `shell` in the right way can make it a superior system tool.
