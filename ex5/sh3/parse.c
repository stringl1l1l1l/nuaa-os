#include "parse.h"
#include "define.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


/**
 * @brief 解析指定命令字符串为cmd结构体
 *
 * @param line  指定命令字符串
 * @param cmd   解析出的结构体的指针，要求已经开辟了内存
 */
void parse_cmd(char *line, struct cmd *cmd)
{
    assert(line);
    assert(cmd);

    int cnt = 0;
    const char delimiter[] = " ";
    char tmp[MAX_CMD_LEN] = {0};
    char *arg = NULL;
    int flag = 0;
    strcpy(tmp, line); // 防止修改原字符串，复制一份字符串用于操作

    while(1) {
        arg = strtok((!flag) ? tmp : NULL, delimiter);

        if(arg == NULL) break;
        flag = 1;

        // 若当前分割出的字符串含重定向符号
        if (arg[0] == '<') {
            if (arg[1]) {// 重定向符号后紧跟文件名 '<log.txt'
                cmd->input = (char *)malloc(sizeof(char) * MAX_ARG_LEN);
                assert(cmd->input);
                strcpy(cmd->input, arg + 1);
            }
            else // 重定向符号单独成参数 '< log.txt'
            {
                arg = strtok(NULL, delimiter);
                assert(arg);
                cmd->input = (char *)malloc(sizeof(char) * MAX_ARG_LEN);
                assert(cmd->input);
                strcpy(cmd->input, arg);
            }

        } else if (arg[1] && arg[0] == '>' && arg[1] == '>') {
            if (arg[2]){
                strcpy(cmd->output, arg + 2);
            }
            else {
                arg = strtok(NULL, delimiter);
                assert(arg);
                strcpy(cmd->output, arg);
            };
            cmd->flag = FLAG_OUT_ADD;

        } else if (arg[0] == '>') {
            if (arg[1]){
                strcpy(cmd->output, arg + 1);
            }
            else {
                arg = strtok(NULL, delimiter);
                assert(arg);
                strcpy(cmd->output, arg);
            };
            cmd->flag = FLAG_OUT_RWRT;

        } else {

            cmd->input[0] = 0;
            cmd->output[0] = 0;
            strcpy(cmd->argv[cnt++], arg);
            cmd->flag = 0;
        }// 深拷贝
    };
    free(cmd->argv[cnt]);
    cmd->argv[cnt] = NULL; // attention: necessary
    cmd->argc = cnt;
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
    printf("flag = %d\n", cmd->flag);
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
    assert(line && strlen(line));
    assert(cmdv);

    int cmdc = 0;
    char *cmd = NULL;
    char tmp[MAX_LINE_LEN] = {0};
    char *save;
    strcpy(tmp, line);

    // strtok cannot be used nestedly, containing static var
    // 使用线程安全版的strtok_r，会从save所指向的字符串开始拆分
    cmd = strtok_r(tmp, "|", &save);
    while (cmd != NULL) {
        parse_cmd(cmd, cmdv + cmdc);
        cmd = strtok_r(NULL, "|", &save);
        cmdc++;
    };
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