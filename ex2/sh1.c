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
        int c;
        c = read(STD_IN, line, sizeof(line)); // 0 是标准输入
        line[c - 1] = 0;

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
            mysys(line);
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

    strcpy(commandStr, command);
    printf("%s\n", commandStr);

    pid_t pid = fork();
    if (pid == 0) {
        // printf("enter into child process\n");
        // splite command string
        int i = 0;
        char* arg = strtok(commandStr, delimiter);
        while (arg != NULL) {
            // printf("%s\n", arg);
            argv[i] = malloc(MAX_STR_LEN * sizeof(char));
            strcpy(argv[i++], arg);
            arg = strtok(NULL, delimiter);
        }
        argv[i] = NULL;
        // execute command
        int execFlag = execvp(argv[0], argv);
        // free unused memory
        for (int j = 0; j < i; j++)
            free(argv[j]);
        if (execFlag < 0) {
            printf("Execute command [%s] fail\n", commandStr);
            exit(EXIT_FAILURE);
        }
    }
    wait(NULL);
    // printf("back to parent");
}

int main()
{
    sh1();
}
