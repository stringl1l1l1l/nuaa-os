#include "cmd.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void exec_cmd(struct cmd *cmd)
{
}

int builtin_cmd(struct cmd *cmd)
{
}

void exec_pipe_cmd(int cmdc, struct cmd *cmdv)
{
}

// /**
//  * @brief 根据传入的参数执行各种重定向
//  *
//  * @param fd    存储文件描述符的数组，
//                 fd[0]为重定向到的文件描述符
//                 fd[1]为重定向后的标准输入
//                 fd[2]为重定向后的标准输出
//  * @param arg   带有重定向符号的字符串, 如">log.txt"
//  */
// void redirect(int fd[], char *arg)
// {
//     int type = -1;
//     int oldfd = -1, new_stdin_fd = -1, new_stdout_fd = -1;
//     int ptr = 0; // 字符指针
//     char fileStr[MAX_CMD_LEN] = { 0 };
//     char tmp_arg[MAX_ARG_LEN] = { 0 };
//     int len = strlen(arg);
//     assert(len);

//     strcpy(tmp_arg, arg);

//     // 判断重定向类型
//     if (tmp_arg[ptr] == '<') {
//         type = RDT_IN;
//         ptr++;
//     } else if (tmp_arg[ptr] == '>') {
//         ptr++;
//         if (tmp_arg[ptr] == '>') {
//             type = RDT_ADD;
//             ptr++;
//         } else
//             type = RDT_REWRT;
//     }

//     assert(type);
//     strcpy(fileStr, tmp_arg + ptr); // 跳过重定向符号，获取需要重定向的文件名
//     assert(fileStr[0]);

//     // 执行重定向, 执行前保存标准输入输出的FILE*
//     if (type == RDT_IN) {
//         oldfd = open(fileStr, O_RDWR);
//         new_stdin_fd = dup(STD_IN);
//         dup2(oldfd, STD_IN);
//     } else if (type == RDT_REWRT) {
//         oldfd = open(fileStr, O_TRUNC | O_RDWR | O_CREAT, 0777);
//         new_stdout_fd = dup(STD_OUT);
//         dup2(oldfd, STD_OUT);
//     } else if (type == RDT_ADD) {
//         oldfd = open(fileStr, O_RDWR | O_APPEND | O_CREAT, 0777);
//         new_stdout_fd = dup(STD_OUT);
//         dup2(oldfd, STD_OUT);
//     }

//     if (oldfd < 0) {
//         perror(fileStr);
//         exit(EXIT_FAILURE);
//     }

//     assert(oldfd && new_stdin_fd && new_stdout_fd);
//     fd[0] = oldfd;
//     fd[1] = new_stdin_fd;
//     fd[2] = new_stdout_fd;
// }