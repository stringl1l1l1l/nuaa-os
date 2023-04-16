#include "utest.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    int id;
    int ok;
    char* desc;
    void (*fp)();
} utest_t;

static int utest_capacity;
static utest_t* utest_array;
static int utest_count;
static int utest_log;

/**
 * @brief 在测试单元数组中添加一个对象，并进行必要的空间开辟和扩容
 *
 * @param desc 测试单元的名称，一般和测试函数名一致
 * @param fp   测试单元的测试函数
 */
void utest_add(char* desc, void (*fp)())
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

    utest_t* utest = utest_array + utest_count;
    utest->id = utest_count;
    utest->desc = strdup(desc);
    utest->fp = fp;
    utest_count++;
    utest_log = open("utest.log", O_CREAT | O_APPEND | O_RDWR, 0777);
}

/**
 * @brief 在给定参数列表中查找指定参数是否存在，若存在，执行一个函数
 *
 * @param argc          总参数个数
 * @param argv          参数列表
 * @param target_arg    待查找的参数
 * @param fp            待执行函数的指针
 */
void utest_parse_args(int argc, char* argv[], char* target_arg, void (*fp)())
{
    for (int i = 0; i < argc; i++) {
        char* arg = argv[i];
        if (strcmp(arg, target_arg) == 0) {
            fp();
            exit(EXIT_SUCCESS);
        }
    }
}

/**
 * @brief 执行指定测试单元的测试，并给出一些提示信息
 *
 * @param utest 指定测试单元
 */
void utest_exec(utest_t* utest)
{
    assert(utest);

    // 重定向标准错误到日志文件
    dup2(utest_log, 2);
    close(utest_log);
    utest->fp();
}

void utest_run(void)
{
    for (int i = 0; i < utest_count; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            alarm(3);
            utest_exec(utest_array + i);
            exit(0);
        }
        int status = -1;
        int error = waitpid(pid, &status, 0);
        if (error > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("%03d: %s", utest_array[i].id, utest_array[i].desc);
            printf("\t%s\n", "\033[0;32m√\033[0m");
        } else {
            printf("%03d: %s", utest_array[i].id, utest_array[i].desc);
            printf("\t%s\n", "\033[0;31m×\033[0m");
        }
    }
}
