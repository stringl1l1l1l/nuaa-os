#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

#define MAX_STR_LEN 100
#define MAX_LINE_LEN 512
#define MAX_ARG_CNT 10

/**
 * @brief 将给定字符串按分隔符分割，并将分割结果赋给res
 *
 * @param res       一个字符串指针数组，用于存储分割结果，最后一个会被置为NULL。
 *    				注意：函数不会为该数组开辟内存，请调用者开辟内存。
 * @param target    待操作的字符串，不能为空。函数操作target副本，不会修改target的内容。
 * @param delimiter 分隔符
 * @return 返回分割出字符串的个数，不包括NULL。
 */
int split(char* res[], const char* target, const char* delimiter)
{
    int len = strlen(target);
    if (len == 0)
        return 0;

    int i = 0;
    char tmp[MAX_STR_LEN];

    // 防止修改原字符串，复制一份字符串用于操作
    strcpy(tmp, target);

    // 将输入的命令字符串分割
    char* arg = strtok(tmp, delimiter);
    while (arg != NULL) {
        strcpy(res[i++], arg);
        // 为什么不能直接指针赋值？因为strtok返回的是指向tmp中字符的指针，而tmp位于栈帧中，生命周期仅限于本函数，
        // 离开这个函数它的内存就消失了，指向它的指针也就成为了悬空指针
        // res[i++] = arg;
        arg = strtok(NULL, delimiter);
    }
    res[i] = NULL;
    return i;
}

/**
 * @brief 	该函数创建一个管道，在子进程中重定向标准输出到管道写端，将分割后的命令的执行结果写入管道；
 *			在父进程中将管道内容读出，并写入父进程的标准输出。
 *
 * @param command 待执行的未分割的命令字符串
 */
void read_pipe(char* command)
{
    int fd[2] = { -1, -1 };
    int red = -1, wrt = -1;
    char buf[MAX_LINE_LEN] = { 0 };
    char* argv[MAX_ARG_CNT];
    // 开辟堆内存
    for (int i = 0; i < MAX_ARG_CNT; i++)
        argv[i] = malloc(MAX_STR_LEN * sizeof(char));

    pipe(fd);
    red = fd[0];
    wrt = fd[1];

    int pid = fork();
    if (pid == 0) {
        // 关闭子进程的管道读端
        close(red);
        // 重定向标准输出到管道写端
        dup2(wrt, STD_OUT);
        // 及时关闭无用的管道写口
        close(wrt);
        // 分割命令字符串并执行
        split(argv, command, " ");
        execvp(argv[0], argv);
    }
    // 父进程等待子进程结束，此时子进程已将执行结果写入管道
    wait(NULL);
    // 重定向标准输入到管道读端
    dup2(red, STD_IN);
    // 关闭父进程管道读写端
    close(red);
    close(wrt);
    // 将管道中的数据通过管道读端读到buf中，并打印到标准输出
    int cnt = -1;
    do {
        cnt = read(STD_IN, buf, MAX_LINE_LEN * sizeof(char));
        write(STD_OUT, buf, cnt * sizeof(char));
    } while (cnt > 0);

    // 释放堆内存，防止内存泄漏
    for (int i = 0; i < MAX_ARG_CNT; i++)
        free(argv[i]);
}

int main()
{
    printf("--------------------------------------------------\n");
    read_pipe("echo HELLO WORLD");
    printf("--------------------------------------------------\n");
    read_pipe("ls /");
    printf("--------------------------------------------------\n");
    return 0;
}
