#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "coro.h"
#include "fs_walk.h"

#define MAX_LINE_LEN 1024

coro_t *consumer;
coro_t *producer;

char *target;

void usage()
{
    unsigned long res = 0;
    while(res = coro_resume(producer)) {
        char *path = (char *)res;
        
        FILE* file = fopen(path, "r");
        char line[MAX_LINE_LEN];   
             
        while (fgets(line, sizeof(line), file)) {
            if(strstr(line, target)!= NULL)
                printf("%s:%s", path, line);
        }
    }
    exit(0);
}

int main(int argc, char **argv)
{
    if(argc != 4) {
        puts("./grep -r linux /usr/include");
        return 0;
    }
    global_dir_path = argv[3];
    target = argv[2];
    consumer = coro_new(usage);
    producer = coro_new(fs_walk);
    coro_boot(consumer);
    return 0;
}