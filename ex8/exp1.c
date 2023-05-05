#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define CAPACITY 4
int buffer[CAPACITY];
int in;
int out;

int buffer_is_empty()
{
    return in == out;
}

int buffer_is_full()
{
    return (in + 1) % CAPACITY == out;
}

int get_item()
{
    int item;

    item = buffer[out];
    out = (out + 1) % CAPACITY;
    return item;
}

void put_item(int item)
{
    buffer[in] = item;
    in = (in + 1) % CAPACITY;
}

pthread_mutex_t mutex;
pthread_cond_t wait_empty_buffer;
pthread_cond_t wait_full_buffer;

#define ITEM_COUNT (CAPACITY * 2)

void* consume(void* arg)
{
    int i;
    int item;

    for (i = 0; i < ITEM_COUNT; i++) {
        /***进入区***/
        pthread_mutex_lock(&mutex); // 加锁，上锁的代码可理解为原子操作。由于多个线程同时访问临界资源，所以必须互斥访问
        while (buffer_is_empty()) // 如果缓冲区为空，消费者线程阻塞
            pthread_cond_wait(&wait_full_buffer, &mutex);
        /***进入区***/

        /***临界区***/
        item = get_item(); // 消费一个数据
        printf("    consume item: %c\n", item);
        /***临界区***/

        /***退出区***/
        pthread_cond_signal(&wait_empty_buffer); // 消费了一个数据，缓冲区出现空余，可以生产，恢复被阻塞的生产者线程
        pthread_mutex_unlock(&mutex);
        /***退出区***/
    }
    return NULL;
}

void* produce(void* arg)
{
    int i;
    int item;

    for (i = 0; i < ITEM_COUNT; i++) {

        pthread_mutex_lock(&mutex);
        while (buffer_is_full())
            pthread_cond_wait(&wait_empty_buffer, &mutex);

        item = 'a' + i;
        put_item(item);
        printf("produce item: %c\n", item);

        pthread_cond_signal(&wait_full_buffer);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main()
{
    pthread_t consumer_tid;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&wait_empty_buffer, NULL);
    pthread_cond_init(&wait_full_buffer, NULL);

    pthread_create(&consumer_tid, NULL, consume, NULL);
    produce(NULL); // 主线程中生产
    pthread_join(consumer_tid, NULL);
    return 0;
}