#include "coro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

coro_t* this_coro;

coro_t* coro_new(void (*entry)())
{
    coro_t* coro = malloc(sizeof(coro_t));
    coro->stack = malloc(CORO_STACK_SIZE);
    coro->back = NULL;
    void* sp = coro->stack + CORO_STACK_SIZE;
    cpu_init(&coro->cpu, (unsigned long)entry, (unsigned long)sp);
    return coro;
}

void coro_delete(coro_t* coro)
{
    free(coro->stack);
    free(coro);
}

/**
 * @brief 切换当前协程到that_coro，并保护现场
 * 
 * @param that_coro 要切换到的目标协程
 * @return unsigned long 返回that_coro的当前value
 */
unsigned long coro_resume(coro_t* that_coro)
{
    cpu_t* source = &this_coro->cpu;
    cpu_t* target = &that_coro->cpu;

    that_coro->back = this_coro;
    this_coro = that_coro;
    return cpu_switch(source, target, 0);
}

/**
 * @brief 返回到切换前的协程
 * 
 * @param value 
 */
void coro_yield(unsigned long value)
{
    coro_t* that_coro = this_coro->back;
    this_coro->back = NULL;

    cpu_t* source = &this_coro->cpu;
    cpu_t* target = &that_coro->cpu;

    this_coro = that_coro;
    cpu_switch(source, target, value);
}


void coro_boot(coro_t* that_coro)
{
    this_coro = coro_new(NULL);
    coro_resume(that_coro);
}
