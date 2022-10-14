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
Changed from `system()` to `fork()`,`exec()`, and `wait()`. This is where the parent and child processes can be obtained using the method of bifurcation parameters. The parent process will wait for the child process to perform the execvp function judgment.
