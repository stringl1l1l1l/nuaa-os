#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coro.h"
#include "fs_walk.h"

coro_t *consumer;
coro_t *producer;

char *target;

void usage()
{
    unsigned long res = 0;
    while(res = coro_resume(producer)) {
        char *path = (char *)res;
        char * ptr = path + strlen(path);
        while(*ptr != '/') ptr -= 1;
        if(strcmp(ptr + 1, target) == 0)  
            puts(path);
    }
    exit(0);
}

int main(int argc, char **argv)
{
    if(argc != 4) {
        puts("usage: ./find /usr/include -name stdio.h");
        return 0;
    }
    global_dir_path = argv[1];
    target = argv[3];
    consumer = coro_new(usage);
    producer = coro_new(fs_walk);
    coro_boot(consumer);
    return 0;
}
