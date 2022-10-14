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

## CallCommand, SplitCommand and PipeCommand
```c 
    int callcommand(int commandnum){
      pid_t pid = fork();
      if (pid == 0){
        exit( pipecommand(0, commandnum));
```
        
`callCommand` using `fork()`to detects whether a subroutine is running
