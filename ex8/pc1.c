/**
 * @file pc1.c
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

sema_t mutex1_sema;
sema_t mutex2_sema;
sema_t empty_buffer1_sema;
sema_t empty_buffer2_sema;
sema_t full_buffer1_sema;
sema_t full_buffer2_sema;

#define ITEM_COUNT (CAPACITY * 2)

/**
 * @brief 消费者线程函数，从buffer2中取出字符并打印
 *
 */
void *consume(void *arg)
{
    int i;
    int item;

    for (i = 0; i < ITEM_COUNT; i++) {
        sema_wait(&full_buffer2_sema);
        sema_wait(&mutex2_sema);

        item = get_item(BUFFER2);
        printf("\t\tconsume item: %c\n", item);

        sema_signal(&mutex2_sema);
        sema_signal(&empty_buffer2_sema);
    }

    return NULL;
}

/**
 * @brief 生产者线程，生产小写字母'a'到'h'到buffer1中
 *
 */
void *produce()
{
    int i;
    int item;

    for (i = 0; i < ITEM_COUNT; i++) {
        sema_wait(&empty_buffer1_sema);
        sema_wait(&mutex1_sema);

        item = i + 'a';
        put_item(item, BUFFER1);
        printf("produce item: %c\n", item);

        sema_signal(&mutex1_sema);
        sema_signal(&full_buffer1_sema);
    }

    return NULL;
}


void *calculate(void *arg)
{
    char item;
    for (int i = 0; i < ITEM_COUNT; i++) {
        // 从buffer1中读取一个数据
        sema_wait(&full_buffer1_sema);
        sema_wait(&mutex1_sema);
        sema_wait(&mutex2_sema);

        item = get_item(BUFFER1);
        sema_signal(&empty_buffer1_sema);

        // 向buffer2中写入计算后的数据
        sema_wait(&empty_buffer2_sema);

        item += 'A' - 'a';
        put_item(item, BUFFER2);
        printf("\tcalculate item: %c\n", item);

        sema_signal(&mutex2_sema);
        sema_signal(&mutex1_sema);
        sema_signal(&full_buffer2_sema);
    }
    return NULL;
}

int main()
{
    pthread_t consumer_tid;
    pthread_t calculater_tid;
    pthread_t producer_tid;

    sema_init(&mutex1_sema, 0);
    sema_init(&mutex2_sema, 0);
    // -1不能丢，信号量的值为CAPACITY代表已经满了
    sema_init(&empty_buffer1_sema, CAPACITY - 1);
    sema_init(&empty_buffer2_sema, CAPACITY - 1);
    sema_init(&full_buffer1_sema, 0);
    sema_init(&full_buffer2_sema, 0);

    pthread_create(&consumer_tid, NULL, consume, NULL);
    pthread_create(&calculater_tid, NULL, calculate, NULL);
    pthread_create(&producer_tid, NULL, produce, NULL);

    pthread_join(consumer_tid, NULL);
    return 0;
}