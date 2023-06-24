#include "parse.h"
#include "define.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


/**
 * @brief 解析指定命令字符串为cmd结构体
 * echo abc >log.txt
 * @param line  指定命令字符串
 * @param cmd   解析出的结构体的指针，要求已经开辟了内存
 */
void parse_cmd(char *line, struct cmd *cmd)
{
    char *part;
    char *save;
    int argc = 0;
    
    part = strtok_r(line, " ", &save);
    while(part != NULL) {
        if(part[0] == '<') {
            cmd->input = malloc(strlen(part));
            strcpy(cmd->input, part + 1);
        }
        else if(part[0] == '>') {
            cmd->output = malloc(strlen(part));
            strcpy(cmd->output, part + 1);
        }
        else {
            cmd->argv[argc] = malloc(strlen(part));
            strcpy(cmd->argv[argc], part);
            argc++;
        }
        part = strtok_r(NULL, " ", &save);
    }
    cmd->argv[argc] = NULL;
    cmd->argc = argc;
}

/**
 * @brief 打印指定的命令结构体
 *
 * @param cmd 指定的命令结构体
 */
void dump_cmd(struct cmd *cmd)
{
    assert(cmd);

    printf("argc = %d \n", cmd->argc);
    printf("argv = {");
    for (int i = 0; i < cmd->argc; i++) {
        if (i != cmd->argc - 1)
            printf("\"%s\", ", cmd->argv[i]);
        else
            printf("\"%s\"}\n", cmd->argv[i]);
    }
    printf("input = %s\n", (cmd->input && cmd->input[0]) ? cmd->input : "NULL");
    printf("output = %s\n", (cmd->output && cmd->output[0]) ? cmd->output : "NULL");
    // printf("flag = %d\n", cmd->flag);
}
/**
 * @brief
 *
 * @param line the command containing pipe
 * @param cmdv
 * @return int
 */
int parse_pipe_cmd(char *line, struct cmd *cmdv)
{
    char *save;
    char *part;
    int cmdc = 0;
    part = strtok_r(line, "|", &save);
    while(part != NULL) {
        parse_cmd(part, cmdv + cmdc);
        part = strtok_r(NULL, "|", &save);
        cmdc++;
    }
    return cmdc;
}

void dump_pipe_cmd(int cmdc, struct cmd *cmdv)
{
    int i;
    // printf("pipe cmd, cmdc = %d\n", cmdc);
    puts("____________________\n");
    for (i = 0; i < cmdc; i++) {
        struct cmd *cmd = cmdv + i;
        dump_cmd(cmd);
    puts("____________________\n");
    }
}

void test()
{
    struct cmd cmdv[MAX_CMD_CNT];
    char line[MAX_LINE_LEN] = { 0 };
    for (int i = 0; i < MAX_CMD_CNT; i++) {
        cmdv[i].input = NULL;
        cmdv[i].output = NULL;
        for (int j = 0; j < MAX_ARG_CNT; j++)
            cmdv[i].argv[j] = (char *)malloc(sizeof(char) * MAX_ARG_LEN);
    }

    int cnt = read(STD_IN, line, MAX_LINE_LEN * sizeof(char));
    line[cnt - 1] = 0;
    int cmdc = parse_pipe_cmd(line, cmdv);
    dump_pipe_cmd(cmdc, cmdv);
}

// echo abc
void test_parse_cmd_1()
{
    struct cmd cmd;
    char line[] = "echo abc xyz";
    parse_cmd(line, &cmd);

    assert(cmd.argc == 3);
    assert(strcmp(cmd.argv[0], "echo") == 0);
    assert(strcmp(cmd.argv[1], "abc") == 0);
    assert(strcmp(cmd.argv[2], "xyz") == 0);
    assert(cmd.argv[3] == NULL);
}

// echo abc >log
void test_parse_cmd_2()
{
}

// cat /etc/passwd | wc -l
void test_parse_pipe_cmd_1()
{
}

// cat <input | sort | cat >output
void test_parse_pipe_cmd_2()
{
}
