#include <stdio.h>
#include <stdlib.h>
#include "coro.h"

#define N 7

coro_t *consumer;
coro_t *producer;
coro_t *filter;


void produce() {
    puts("PRODUCE");
    for(int i = 0; i <= N; i++) {
        printf("produce %d\n", i);
        coro_yield(i);
    }
}

void filt() {
    puts("\tFILTER");
    for(int i = 0; i <= N; i++) {
        int value = coro_resume(producer);
        int new_val = value * 10;
        printf("\tfilter %d -> %d\n", value, new_val);
        coro_yield(new_val);
    }
}

void consume() {
    puts("\t\tCONSUME");
    for(int i = 0; i <= N; i++) {
        int value = coro_resume(filter);
        printf("\t\tconsume %d\n", value);
    }
    puts("\t\tEND");
    exit(0);
}

int main()
{
    consumer = coro_new(consume);
    producer = coro_new(produce);
    filter = coro_new(filt);
    coro_boot(consumer);
    return 0;
}
