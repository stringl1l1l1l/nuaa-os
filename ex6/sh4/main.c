#include "cmd.h"
#include "parse.h"
#include "define.h"
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    struct cmd cmdv[100];
    char line[MAX_LINE_LEN];
    while(1) {
       write(STDOUT_FILENO, "> ", 3);
       int cnt = read(STDIN_FILENO, line, MAX_LINE_LEN);
       line[cnt - 1] = '\0';
       int cmdc = parse_pipe_cmd(line, cmdv);
    //    dump_pipe_cmd(cmdc, cmdv);
        if(cmdc == 1) {
            if(!builtin_cmd(cmdv)) {
                pid_t pid = fork();
                if(pid == 0)
                    exec_cmd(cmdv);
                 wait(NULL);
            }
        } 
        else {
            pid_t pid = fork();
                if(pid == 0)
                    exec_pipe_cmd(cmdc, cmdv);
                wait(NULL);
        }
    } 
    return 0;
}
