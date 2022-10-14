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

## callCommand, splitCommand and pipeCommand
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
        
`callCommand` using `fork()`to detects whether a subroutine is running, If not it will run `pipeCommand`.
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
In `pipeCommand`.`left` represents the leftmost string, `right` represents the rightmost string. If `|` was encountered while reading the string. 
