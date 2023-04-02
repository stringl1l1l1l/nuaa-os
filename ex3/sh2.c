#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

#define MAX_STR_LEN 100
#define MAX_LINE_LEN 1000
#define MAX_ARG_CNT 10

#define RDT_IN 1
#define RDT_ADD 2
#define RDT_REWRT 3

void sh2();
char *pwd();
void mysys(char *commmad[]);

int new_stdin_fd = STD_IN;
int new_stdout_fd = STD_OUT;

/**
 * @brief 根据传入的参数执行各种重定向
 *
 * @param oldfd 待重定向的文件描述符
 * @param arg   带有重定向符号的字符串, 如">log.txt"
 * @return 返回重定向的文件描述符，没有重定向则返回-1
 */
int redirect(char *arg)
{
    int type = -1;
    int oldfd = -1;
    int ptr = 0; // 字符指针
    char fileStr[MAX_STR_LEN];
    char tmp_arg[MAX_STR_LEN];

    int len = strlen(arg);
    if (len == 0)
        return -1;

    strcpy(tmp_arg, arg);

    // 判断重定向类型
    if (tmp_arg[ptr] == '<') {
        type = RDT_IN;
        ptr++;
    } else if (tmp_arg[ptr] == '>') {
        ptr++;
        if (tmp_arg[ptr] == '>') {
            type = RDT_ADD;
            ptr++;
        } else
            type = RDT_REWRT;
    }

    // 获取需要重定向的文件名
    if (type == -1)
        return -1;
    else
        strcpy(fileStr, tmp_arg + ptr);
    if (strlen(fileStr) == 0)
        return -1;

    // 执行重定向, 执行前保存标准输入输出的FILE*
    if (type == RDT_IN) {
        oldfd = open(fileStr, O_RDWR);
        new_stdin_fd = dup(STD_IN);
        dup2(oldfd, STD_IN);
    } else if (type == RDT_REWRT) {
        oldfd = open(fileStr, O_TRUNC | O_RDWR | O_CREAT, 0777);
        new_stdout_fd = dup(STD_OUT);
        dup2(oldfd, STD_OUT);
    } else if (type == RDT_ADD) {
        oldfd = open(fileStr, O_RDWR | O_APPEND | O_CREAT, 0777);
        new_stdout_fd = dup(STD_OUT);
        dup2(oldfd, STD_OUT);
    }

    if (oldfd < 0) {
        perror(fileStr);
        exit(EXIT_FAILURE);
    }
    return oldfd;
}

/**
 * @brief 将给定字符串按分隔符分割，并将分割结果赋给res
 *
 * @param res       一个字符串指针数组，用于存储分割结果，最后一个会被置为NULL
 * @param target    待操作的字符串，不能为空
 * @param delimiter 分隔字符串
 * @return 返回分割出字符串的个数，不包括NULL
 */
int split(char *res[], const char *target, const char *delimiter)
{
    int len = strlen(target);
    if (len == 0)
        return 0;

    int i = 0;
    char tmp[MAX_STR_LEN];

    // 防止修改原字符串，复制一份字符串用于操作
    strcpy(tmp, target);

    // 将输入的命令字符串分割
    char *arg = strtok(tmp, delimiter);
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
 * @brief 自实现的shell, 实现了pwd, cd, exit命令, 以及执行结果的重定向
 *
 */
void sh2()
{
    char line[MAX_LINE_LEN];
    char *argv[MAX_STR_LEN];
    int cnt;
    int argc = 0;
    while (1) {
        // 打印一个>
        write(STD_OUT, "> ", 2);
        // 读取一行命令
        cnt = read(STD_IN, line, sizeof(line));
        // 将最后一个回车覆盖
        line[--cnt] = 0;

        // 开辟堆内存
        for (int i = 0; i < MAX_ARG_CNT; i++)
            argv[i] = malloc(MAX_STR_LEN * sizeof(char));

        // 判断命令是否为空，为空跳过循环
        if (strlen(line) == 0)
            continue;

        argc = split(argv, line, " ");

        int oldfd = redirect(argv[argc - 1]);
        // 重定向成功时，将最后一个参数置为NULL
        if (oldfd != -1)
            argv[argc - 1] = NULL;

        // 根据输入的命令类型执行不同的操作
        if (strcmp(argv[0], "pwd") == 0) {
            char *wd = pwd();
            // 释放pwd malloc的内存
            if (wd != NULL) {
                puts(wd);
                free(wd);
            }
        } else if (strcmp(argv[0], "cd") == 0) {
            int res = chdir(argv[1]);
            if (res == 0) {
                char *wd = pwd();
                printf("Change into directory [%s].\n", wd);
                // 释放pwd malloc的内存
                if (wd != NULL) {
                    puts(wd);
                    free(wd);
                }
            } else
                printf("Change directory fail.\n");
        } else if (strcmp(argv[0], "exit") == 0)
            exit(0);
        else
            mysys(argv);

        // 恢复标准输入输出
        dup2(new_stdin_fd, STD_IN);
        dup2(new_stdout_fd, STD_OUT);
        new_stdin_fd = STD_IN;
        new_stdout_fd = STD_OUT;

        // 关闭oldfd
        if (oldfd != -1)
            close(oldfd);
    }
    // 释放堆内存
    for (int i = 0; i < MAX_ARG_CNT; i++)
        free(argv[i]);
}

/**
 * @brief 自实现的pwd命令
 *
 * @return  char* 返回一个malloc出的字符串，表明当前目录，如果程序出错返回NULL，
 *          为了防止内存泄漏要记得free
 */
char *pwd()
{
    char *buffer = malloc(PATH_MAX * sizeof(char));
    char *res = getcwd(buffer, PATH_MAX);
    if (res == NULL) {
        printf("Fail to get current work directory.\n");
        return NULL;
    }
    return buffer;
}

/**
 * @brief 自实现的mysys函数, 实现了将命令字符串分割，并开启子进程执行传入命令的功能
 *
 * @param argv 待执行的命令字符串数组
 */
void mysys(char *argv[])
{
    pid_t pid = fork();
    if (pid == 0) {
        // 执行命令
        int execFlag = execvp(argv[0], argv);
        if (execFlag < 0) {
            perror(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    wait(NULL);
}

int main()
{
    sh2();
    return 0;
}
