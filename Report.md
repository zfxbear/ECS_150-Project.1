# Report for sschell.c

### HanChen Yu & QiJun Liang

## Summary
This program `sschell.c`, can Execution of user-supplied commands with optional arguments, Selection of typical builtin commands, Redirection of the standard output of commands to files and Composition of commands via piping. 

## Implementation
1.Using `fgets` to get the command input

2.Check command input is regular command or builtin command

3.Check whitespaces

4.Parse `pipe` and `redirect`

5.Parse the string to select the commands, arguments and redirection

6.Using the `fork()`,`exec()`and `wait` to display the output
