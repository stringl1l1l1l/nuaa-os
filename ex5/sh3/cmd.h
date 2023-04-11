#ifndef _CMD_H
#define _CMD_H

#include "define.h"
#include <stddef.h>
#include <stdlib.h>
// "echo abc >log"
// argc = 2
// argv = {"echo", "abc"}
// input = NULL
// output = "log"

struct cmd {
    int flag;
    int argc;
    char *argv[MAX_ARG_CNT];
    char *input;
    char *output;
};

extern void exec_cmd(struct cmd *cmd);
extern int builtin_cmd(struct cmd *cmd);
extern void exec_pipe_cmd(int cmdc, struct cmd *cmdv);

#endif
