#include "cmd.h"
#include "parse.h"
#include "define.h"
#include <assert.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

void initStruct(struct cmd *cmdv)
{
    // initialize heap memory
    for (int i = 0; i < MAX_CMD_CNT; i++) {
        cmdv[i].input = (char *)malloc(sizeof(char) * MAX_ARG_LEN);
        cmdv[i].output = (char *)malloc(sizeof(char) * MAX_ARG_LEN);
        for (int j = 0; j < MAX_ARG_CNT; j++)
            cmdv[i].argv[j] = (char *)malloc(sizeof(char) * MAX_ARG_LEN);
    }
}

void deleteStruct(struct cmd *cmdv)
{
    // initialize heap memory
    for (int i = 0; i < MAX_CMD_CNT; i++) {
        free(cmdv[i].input);
        free(cmdv[i].output);
        for (int j = 0; j < MAX_ARG_CNT; j++)
            free(cmdv[i].argv[j]);
    }
}

int main()
{
    struct cmd cmdv[MAX_CMD_CNT];
    char line[MAX_LINE_LEN] = { 0 };

    while (1) {
        initStruct(cmdv);
        write(STD_OUT, "> ", 3);
        int cnt = read(STD_IN, line, MAX_LINE_LEN * sizeof(char)); // read command line
        line[cnt - 1] = 0;// erase the tail '\n'
        int cmdc = parse_pipe_cmd(line, cmdv);
        // dump_pipe_cmd(cmdc, cmdv);

        if(cmdc > 1) {
            exec_pipe_cmd(cmdc, cmdv);
        }
        else {
            if(!builtin_cmd(&cmdv[0]))
                exec_cmd(&cmdv[0]);
        }
        puts("");
        deleteStruct(cmdv);
    }
    return 0;
}
