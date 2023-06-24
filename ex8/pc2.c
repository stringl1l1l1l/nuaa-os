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

/********信号量相关定义********/
typedef struct {
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} sema_t;

/**
 * @brief 信号量初始化
 *
 * @param sema 信号量数据结构
 * @param value 信号量的初始值
 */
void sema_init(sema_t *sema, int value)
{
    sema->value = value;
    pthread_mutex_init(&sema->mutex, NULL);
    pthread_cond_init(&sema->cond, NULL);
}

/**
 * @brief 如果信号量的值 <= 0，则等待条件变量，并将信号量的值-1
 *
 * @param sema 信号量数据结构
 */
void sema_wait(sema_t *sema)
{
    pthread_mutex_lock(&sema->mutex);
    while (sema->value <= 0)
        pthread_cond_wait(&sema->cond, &sema->mutex);
    sema->value--;
    pthread_mutex_unlock(&sema->mutex);
}

/**
 * @brief 将信号量的值+1, 并唤醒等待条件变量的线程
 *
 * @param sema 信号量数据结构
 */
void sema_signal(sema_t *sema)
{
    pthread_mutex_lock(&sema->mutex);
    ++sema->value;
    pthread_cond_signal(&sema->cond);
    pthread_mutex_unlock(&sema->mutex);
}
/********信号量相关定义********/
typedef struct
{
    int data[CAPACITY];
    int in;
    int out;
    sema_t full;
    sema_t empty;
} Buffer;

Buffer buffers[2];

void buffer_init(Buffer *buffer) {
    buffer->out = 0;
    buffer->in = 0;
    sema_init(&buffer->full, 0);
    sema_init(&buffer->empty, CAPACITY);
}

int buffer_get_item(Buffer *buffer)
{
    sema_wait(&buffer->full);
    
    int item = buffer->data[buffer->out];
    buffer->out = (buffer->out + 1) % CAPACITY;
    
    sema_signal(&buffer->empty);
    return item;
}

void buffer_put_item(Buffer *buffer, int item)
{
    sema_wait(&buffer->empty);
        
    buffer->data[buffer->in] = item;
    buffer->in = (buffer->in + 1) % CAPACITY;
    
    sema_signal(&buffer->full);
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
    
    pthread_join(producer_tid, NULL);
    pthread_join(calculater_tid, NULL);
    pthread_join(consumer_tid, NULL);
    
    
    return 0;
}