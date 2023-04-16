#ifndef DEFINE_H
#define DEFINE_H

#define STD_IN  0
#define STD_OUT 1
#define STD_ERR 2

static int new_std_in  = 0;
static int new_std_out = 1;
static int new_std_err = 2;

#define RDT_IN      1
#define RDT_ADD     2
#define RDT_REWRT   3

#define MAX_ARG_CNT     10
#define MAX_ARG_LEN     50
#define MAX_CMD_CNT     10
#define MAX_CMD_LEN     100
#define MAX_LINE_LEN    1000

#define FLAG_OUT_ADD     0b01
#define FLAG_OUT_RWRT    0b10

#define PATH_MAX     150

#endif