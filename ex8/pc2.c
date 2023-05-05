/**
 * @file pc2.c
 * @author 陈立文
 * @brief  使用信号量解决生产者、计算者、消费者问题
 * @date 2023-05-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define CAPACITY 4
#define BUFFER1 0
#define BUFFER2 1

typedef struct
{
    int buffer[CAPACITY];
    int in;
    int out;
} Buffer;

Buffer buffers[2];

int buffer_is_empty(int index)
{
    return buffers[index].in == buffers[index].out;
}

int buffer_is_full(int index)
{
    return (buffers[index].in + 1) % CAPACITY == buffers[index].out;
}

int get_item(int index)
{
    int item;
    Buffer *curBuf = &buffers[index];
    item = curBuf->buffer[curBuf->out];
    curBuf->out = (curBuf->out + 1) % CAPACITY;
    return item;
}

void put_item(int item, int index)
{
    Buffer *curBuf = &buffers[index];
    curBuf->buffer[curBuf->in] = item;
    curBuf->in = (curBuf->in + 1) % CAPACITY;
}

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_cond_t wait_empty_buffer1;
pthread_cond_t wait_full_buffer1;
pthread_cond_t wait_empty_buffer2;
pthread_cond_t wait_full_buffer2;

#define ITEM_COUNT (CAPACITY * 2)

/**
 * @brief 消费者线程函数，从buffer2中取出字符并打印
 *
 */
void* consume(void* arg)
{
    int i;
    int item;

    for (i = 0; i < ITEM_COUNT; i++) {
        pthread_mutex_lock(&mutex2);
        while (buffer_is_empty(BUFFER2))
            pthread_cond_wait(&wait_full_buffer2, &mutex2);

        item = get_item(BUFFER2); // 消费一个数据
        printf("\t\tconsume item: %c\n", item);

        pthread_cond_signal(&wait_empty_buffer2);
        pthread_mutex_unlock(&mutex2);
    }
    return NULL;
}

/**
 * @brief 生产者线程，生产小写字母'a'到'h'到buffer1中
 *
 */
void* produce(void* arg)
{
    int i;
    int item;

    for (i = 0; i < ITEM_COUNT; i++) {
        pthread_mutex_lock(&mutex1);
        while (buffer_is_full(BUFFER1))
            pthread_cond_wait(&wait_empty_buffer1, &mutex1);

        item = 'a' + i;
        put_item(item, BUFFER1);
        printf("produce item: %c\n", item);

        pthread_cond_signal(&wait_full_buffer1);
        pthread_mutex_unlock(&mutex1);
    }
    return NULL;
}

void *calculate(void *arg)
{
    char item;
    for (int i = 0; i < ITEM_COUNT; i++) {
        // 从buffer1中读取一个数据
        pthread_mutex_lock(&mutex1);
        pthread_mutex_lock(&mutex2);
        while (buffer_is_empty(BUFFER1))
            pthread_cond_wait(&wait_full_buffer1, &mutex1);
        item = get_item(BUFFER1);

        pthread_cond_signal(&wait_empty_buffer1);

        // 向buffer2中写一个数据
        while (buffer_is_full(BUFFER2))
            pthread_cond_wait(&wait_empty_buffer2, &mutex2);

        item += 'A' - 'a';
        put_item(item, BUFFER2);
        printf("\tcalulate item: %c\n", item);

        pthread_cond_signal(&wait_full_buffer2);
        pthread_mutex_unlock(&mutex1);
        pthread_mutex_unlock(&mutex2);
    }
    return NULL;
}

int main()
{
    pthread_t consumer_tid;
    pthread_t calculater_tid;
    pthread_t producer_tid;

    pthread_mutex_init(&mutex1, NULL);
    pthread_cond_init(&wait_empty_buffer1, NULL);
    pthread_cond_init(&wait_full_buffer1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_cond_init(&wait_empty_buffer2, NULL);
    pthread_cond_init(&wait_full_buffer2, NULL);

    pthread_create(&consumer_tid, NULL, consume, NULL);
    pthread_create(&calculater_tid, NULL, calculate, NULL);
    pthread_create(&producer_tid, NULL, produce, NULL);

    pthread_join(consumer_tid, NULL);
    return 0;
}