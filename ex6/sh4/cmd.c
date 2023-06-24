#include "cmd.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "define.h"


void exec_cmd(struct cmd *cmd)
{
    if(cmd->input) {
        int fd_input = open(cmd->input, O_RDONLY);
        dup2(fd_input, STDIN_FILENO);
        close(fd_input);
    }
    
    if(cmd->output) {
        if(cmd->output[0] != '>') {
            int fd_output = open(cmd->output, O_TRUNC | O_RDWR | O_CREAT, 0744);
            dup2(fd_output, STDOUT_FILENO);
            close(fd_output);
        }
        else {
            int fd_output = open(cmd->output, O_TRUNC | O_RDWR | O_APPEND, 0744);
            dup2(fd_output, STDOUT_FILENO);
            close(fd_output);
        }
    }
    int res = execvp(cmd->argv[0], cmd->argv);
    if(res < 0) {
        perror(cmd->argv[0]);
        exit(EXIT_FAILURE);
    }
}

int builtin_cmd(struct cmd *cmd)
{
    char *cmd_name = cmd->argv[0];
    if (!strcmp(cmd_name, "pwd")) {
        char buffer[PATH_MAX];
        char *res = getcwd(buffer, PATH_MAX);
        assert(res);
        write(STDOUT_FILENO, buffer, strlen(buffer));
        return 1;
    }
    else if (!strcmp(cmd_name, "cd")) {
        assert(cmd->argv[1]);
        int res = chdir(cmd->argv[1]);
        if (res == 0) {
            char buffer[PATH_MAX];
            char *res = getcwd(buffer, PATH_MAX);
            assert(res);
            write(STDOUT_FILENO, buffer, strlen(buffer));
        }
        return 1;
    }
    else if (!strcmp(cmd_name, "exit")) 
        exit(EXIT_SUCCESS);

    return 0;
}

void exec_pipe_cmd(int cmdc, struct cmd *cmdv)
{
    if(cmdc == 1) {
        exec_cmd(cmdv);
        exit(0);
    }
        
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    // 管道自带阻塞控制，父子进程读写操作可以并行执行
    // 子进程只写管道
    if(pid == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        exec_pipe_cmd(cmdc - 1, cmdv);
    }
    // 父进程只读管道
    else {
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        exec_cmd(cmdv + cmdc - 1);
    }
}

void read_and_cmp(char *path, char *expected)
{
    int fd = open(path, O_RDONLY);

    char buff[1024];
    int count = read(fd, buff, sizeof(buff));
    buff[count] = 0;

    close(fd);
    assert(strcmp(buff, expected) == 0);
}

void test_exec_cmd()
{
    unlink("test.out");

    pid_t pid = fork();
    if (pid == 0) { 
        struct cmd cmd = {
            2, {"echo", "hello", NULL}, NULL, "test.out"
        };
        exec_cmd(&cmd);
    }
    wait(NULL);

    read_and_cmp("test.out", "hello\n");
}

// // cat <test.in | sort | uniq | cat >test.out
// void test_exec_pipe_cmd()
// {
//     unlink("test.out");

//     pid_t pid = fork();
//     if (pid == 0) { 
//         exec_pipe_cmd(4, cmdv);
//     }
//     wait(NULL);

//     read_and_cmp("test.out", "1\n2\n3\n");
// }
