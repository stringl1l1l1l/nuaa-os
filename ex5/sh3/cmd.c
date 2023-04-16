#include "cmd.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "define.h"

extern int new_std_in;
extern int new_std_out;

void exec_cmd(struct cmd *cmd)
{
    pid_t pid = fork();

    assert(pid >= 0);
    if(pid == 0) {
        if (cmd->input && cmd->input[0]) {
            int fd_in = open(cmd->input, O_RDWR);
            close(STD_IN);
            dup2(fd_in, STD_IN);
            close(fd_in);
        }

        if (cmd->output && cmd->output[0]) {
            assert(cmd->flag & 3);

            int fd_out = -1;
            if(cmd->flag == FLAG_OUT_ADD)
                fd_out = open(cmd->output, O_RDWR | O_APPEND | O_CREAT, 0777);
            else
                fd_out = open(cmd->output, O_TRUNC | O_RDWR | O_CREAT, 0777);

            assert(fd_out != -1);
            close(STD_OUT);
            dup2(fd_out, STD_OUT);
            close(fd_out);
        }

        int res = execvp(cmd->argv[0], cmd->argv);
        if (res < 0) {
            perror(cmd->argv[0]);
            exit(EXIT_FAILURE);
        }
        exit(0);
    }
    else {
        wait(NULL);
    }
}

int builtin_cmd(struct cmd *cmd)
{
    char *cmd_name = cmd->argv[0];
    int flag = 1;
    if (!strcmp(cmd_name, "pwd")) {
        char buffer[PATH_MAX] = {0};
        char *res = getcwd(buffer, PATH_MAX);
        assert(res);
        write(STD_OUT, buffer, strlen(buffer));
    }
    else if (!strcmp(cmd_name, "cd")) {
        assert(cmd->argv[1]);
        int res = chdir(cmd->argv[1]);
        if (res == 0) {
            char buffer[PATH_MAX] = {0};
            char *res = getcwd(buffer, PATH_MAX);
            assert(res);
            write(STD_OUT, buffer, strlen(buffer));
        }
    }
    else if (!strcmp(cmd_name, "exit")) {
        exit(EXIT_SUCCESS);
    }
    else flag = 0;

    return flag;
}

void exec_pipe_cmd(int cmdc, struct cmd *cmdv)
{
    new_std_out = dup(STD_OUT);
    new_std_in = dup(STD_IN);

    int fd[2] = {-1, -1};
    pipe(fd);

    // last pipe read port is the next STD_IN
    // STD_OUT is the next pipe write port
    dup2(fd[1], STD_OUT);
    close(fd[1]);
    if (!builtin_cmd(&cmdv[0]))
        exec_cmd(&cmdv[0]);

    for(int i = 1; i < cmdc; i++) {
        close(STD_IN);
        dup2(fd[0], STD_IN);
        close(fd[0]);

        pipe(fd);
        close(STD_OUT);
        dup2(fd[1], STD_OUT);
        close(fd[1]);

        if (i == cmdc - 1) {
            close(fd[0]);
            close(STD_OUT);
            dup2(new_std_out, STD_OUT);
        }

        if (!builtin_cmd(&cmdv[i]))
                exec_cmd(&cmdv[i]);
    }

    close(STD_IN);
    close(STD_OUT);
    dup2(new_std_out, STD_OUT);
    dup2(new_std_in, STD_IN);
    close(new_std_in);
    close(new_std_out);
}