/**
 * @file pc2.c
 * @author 陈立文
 * @brief  使用条件变量解决生产者、计算者、消费者问题
 * @date 2023-05-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define CAPACITY 4

typedef struct
{
    int data[CAPACITY];
    int in;
    int out;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} Buffer;

Buffer buffers[2];

void buffer_init(Buffer *buffer) {
    buffer->out = buffer->in = 0;
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->cond, NULL);
}

int buffer_is_empty(Buffer *buffer)
{
    return buffer->in == buffer->out;
}

int buffer_is_full(Buffer *buffer)
{
    return (buffer->in + 1) % CAPACITY == buffer->out;
}

int buffer_get_item(Buffer *buffer)
{
    pthread_mutex_lock(&buffer->mutex);
    while(buffer_is_empty(buffer)) 
        pthread_cond_wait(&buffer->cond, &buffer->mutex);
        
    int item = buffer->data[buffer->out];
    buffer->out = (buffer->out + 1) % CAPACITY;
    
    pthread_cond_broadcast(&buffer->cond);
    pthread_mutex_unlock(&buffer->mutex);
    
    return item;
}

void buffer_put_item(Buffer *buffer, int item)
{
    pthread_mutex_lock(&buffer->mutex);
    while(buffer_is_full(buffer)) 
        pthread_cond_wait(&buffer->cond, &buffer->mutex);
        
    buffer->data[buffer->in] = item;
    buffer->in = (buffer->in + 1) % CAPACITY;
    
    pthread_cond_broadcast(&buffer->cond);
    pthread_mutex_unlock(&buffer->mutex);
}

#define ITEM_COUNT (CAPACITY * 2)

/**
 * @brief 消费者线程函数，从buffer2中取出字符并打印
 *
 */
void* consume(void* arg)
{
    for(int i = 0; i < ITEM_COUNT; i++) {
        int item = buffer_get_item(&buffers[1]);
        printf("\t\tconsume item: %c\n", item); 
    }
    return NULL;
}

/**
 * @brief 生产者线程，生产小写字母'a'到'h'到buffer1中
 *
 */
void* produce(void* arg)
{
    for (int i = 0; i < ITEM_COUNT; i++) {
        int item = i + 'a'; 
        buffer_put_item(&buffers[0], item);
        printf("produce item: %c\n", item);
    }
    return NULL;
}

void *calculate(void *arg)
{
    for (int i = 0; i < ITEM_COUNT; i++) {
        int item = buffer_get_item(&buffers[0]);
        item += 'A' - 'a';
        buffer_put_item(&buffers[1], item);
        printf("\tcalculate item: %c\n", item);
    }
    return NULL;
}

int main()
{
    pthread_t consumer_tid;
    pthread_t calculater_tid;
    pthread_t producer_tid;
    
    for(int i = 0; i < 2; i++) 
        buffer_init(&buffers[i]);
    
    
    pthread_create(&calculater_tid, NULL, calculate, NULL);
    pthread_create(&consumer_tid, NULL, consume, NULL);
    pthread_create(&producer_tid, NULL, produce, NULL);
    
    pthread_join(consumer_tid, NULL);
    pthread_join(calculater_tid, NULL);
    return 0;
}