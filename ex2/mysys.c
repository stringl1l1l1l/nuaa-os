#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_STR_LEN 100
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
        if (execFlag < 0) {
            printf("execute %s fail\n", argv[0]);
        }
    }
    wait(NULL);
    printf("back to parent");
}

int main()
{
    printf("--------------------------------------------------\n");
    mysys("echo HELLO WORLD");
    printf("--------------------------------------------------\n");
    mysys("ls /");
    printf("--------------------------------------------------\n");
    return 0;
}