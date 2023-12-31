#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include "utest.h"

typedef struct {
    int id;
    char *desc;
    void (*fp)();
    int ok;
} utest_t;

static int utest_capacity;
static utest_t *utest_array;
static int utest_count;
static int utest_log;

void utest_add(char *desc, void (*fp)())
{
    assert(utest_count <= utest_capacity);

    if (utest_capacity == 0) {
        utest_capacity = 2;
        utest_array = malloc(utest_capacity * sizeof(utest_t));
    }

    if (utest_count == utest_capacity) {
        utest_capacity *= 2;
        utest_array = realloc(utest_array, utest_capacity * sizeof(utest_t));
    }

    utest_t *utest = utest_array + utest_count;
    utest->id = utest_count;
    utest->desc = strdup(desc);
    utest->fp = fp;
    utest_count++;
}

void utest_parse_args(int argc, char *argv[], char *target_arg, void (*fp)())
{
    for (int i = 0; i < argc; i++) {
        char *arg = argv[i];
        if (strcmp(arg, target_arg) == 0) {
            fp();
            exit(EXIT_SUCCESS);
        }
    }
}

void utest_exec(utest_t *utest)
{
    printf("%03d: %-25s", utest->id, utest->desc);
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) { 
        // 重定向标准错误和标准输出到日志文件
        // dup2(utest_log, 1);
        dup2(utest_log, 2);
        close(utest_log);
        
        alarm(1);
        utest->fp();
        exit(EXIT_SUCCESS);
    }
    
    utest->ok = 0;
    int status = -1;
    int error = waitpid(pid, &status, 0);
    
    char *color_on = "\033[0;31m", *color_off = "\033[0m";
    char *result = "×";
    
    // 若没有出现任何错误，即子进程正常退出时：
    if (error > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
       utest->ok = 1;
       color_on = "\033[0;32m";
       result = "√";
    }
    printf("\t%s%s%s\n", color_on, result, color_off);
}

void utest_run(void)
{
    utest_log = open("utest.log", O_CREAT | O_APPEND | O_RDWR, 0777);
    if(utest_log < 0) {
        perror("open");
        puts("open utest.log failed");
        exit(EXIT_FAILURE);
    }
    
    int pass_all_test = 1;
    for (int i = 0; i < utest_count; i++) {
        utest_exec(&utest_array[i]);
        if(utest_array[i].ok == 0) 
            pass_all_test = 0;
    }
    
    puts("");
    if(pass_all_test) {
        puts("Pass all the test.");
        return;
    }
    
    puts("The following test failed:");
    for (int i = 0; i < utest_count; i++) {
        if(utest_array[i].ok == 0) 
            printf("%03d: %-25s\n", utest_array[i].id, utest_array[i].desc);
    }
    puts("See utest.log for details");
}
