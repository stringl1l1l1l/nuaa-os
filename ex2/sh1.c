#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define STD_IN 0
#define STD_OUT 1
#define MAX_STR_LEN 100
#define MAX_LINE_LEN 1000

int sh1();
char* pwd();
void mysys(char* commmad);

int sh1()
{
    while (1) {
        char str[] = "> ";
        write(STD_OUT, str, sizeof(str));

        char line[MAX_LINE_LEN];
        char tmp[MAX_LINE_LEN];
        int c;
        c = read(STD_IN, line, sizeof(line)); // 0 是标准输入
        line[c - 1] = 0;
        // 判断命令是否为空
        if (strlen(line) == 0)
            continue;

        strcpy(tmp, line);
        char* head = strtok(line, " ");
        if (strcmp(head, "pwd") == 0) {
            char* wd = pwd();
            if (wd != NULL) {
                printf("%s\n", wd);
                free(wd);
            }
        } else if (strcmp(head, "cd") == 0) {
            char* path = strtok(NULL, " ");
            int res = chdir(path);
            if (res == 0)
                printf("Change into directory [%s].\n", pwd());
            else
                printf("Change directory fail.\n");
        } else if (strcmp(head, "exit") == 0)
            exit(0);
        else
            mysys(tmp);
    }
    return 0;
}

char* pwd()
{
    char* buffer = malloc(PATH_MAX * sizeof(char));
    char* res = getcwd(buffer, PATH_MAX);
    if (res == NULL) {
        printf("Fail to get current work directory.\n");
        return NULL;
    }
    return buffer;
}

void mysys(char* command)
{
    const char* delimiter = " ";
    char* argv[MAX_STR_LEN];
    char commandStr[MAX_STR_LEN];
    // 复制一份命令字符串
    strcpy(commandStr, command);

    pid_t pid = fork();
    if (pid == 0) {
        // printf("enter into child process\n");
        int i = 0;
        char* arg = strtok(commandStr, delimiter);
        // 将输入的命令字符串分割
        while (arg != NULL) {
            argv[i++] = arg;
            arg = strtok(NULL, delimiter);
        }
        argv[i] = NULL;
        // 执行命令
        int execFlag = execvp(argv[0], (char* const*)argv);

        if (execFlag < 0) {
            perror(commandStr);
            exit(EXIT_FAILURE);
        }
        // 释放内存
        for (int j = 0; argv[j] != NULL; j++)
            free(argv[j]);
    }
    wait(NULL);
}

int main()
{
    sh1();
}
