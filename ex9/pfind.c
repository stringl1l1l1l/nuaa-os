/**
 * @file pfind.c
 * @author 陈立文
 * @brief 并行查找，主线程作为生产者生产任务到任务队列，消费者线程并行执行任务队列中的任务，
 *        使用条件变量作为线程调度工具
 * @date 2023-05-07
 */

/*
    1. 创建一个任务队列;
    - 初始化时，任务队列为空
    2. 创建 WORKER_NUMBER 个子线程;
    3. 对目录 path 进行递归遍历:
    - 遇见叶子节点时
    - 把叶子节点的路径加入到任务队列中
    4. 创建 WORER_NUMBER 个特殊任务
    - 特殊任务的 is_end 为真
    * 子线程读取到特殊任务时
    * 表示主线程已经完成递归遍历，不会再向任务队列中放置任务
    * 此时，子线程可以退出
    - 把这些特殊任务加入到任务队列中
    5. 等待所有的子线程结束;
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

/**数据定义**/
#define WORKER_NUMBER 4
#define CAPATICY 4

typedef struct task{
 int is_end;
 char path[128];
 char string[128];
} Task;

typedef struct taskqueue
{
    Task que[CAPATICY];
    int in;
    int out;
} TaskQueue;

TaskQueue taskqueue;
pthread_mutex_t mutex;
pthread_cond_t empty_queue;
pthread_cond_t full_queue;
/**数据定义**/

void find_file(char *path, char *target);
void find_dir(char *path, char *target);

int queue_is_empty(TaskQueue *tq)
{
    return tq->in == tq->out;
}

int queue_is_full(TaskQueue *tq)
{
    return (tq->in + 1) % CAPATICY == tq->out;
}

Task* get_item(TaskQueue *tq)
{
    Task *item;

    item = &tq->que[tq->out];
    tq->out = (tq->out + 1) % CAPATICY;
    return item;
}

void put_item(TaskQueue *tq, Task item)
{
    tq->que[tq->in] = item;
    tq->in = (tq->in + 1) % CAPATICY;
}

void *worker_entry(void *arg)
{
    while (1) {
        pthread_mutex_lock(&mutex);
        while (queue_is_empty(&taskqueue))
            pthread_cond_wait(&full_queue, &mutex);
        //从任务队列中获取一个任务 task;
        Task *task = get_item(&taskqueue);
        if (task->is_end) {
            pthread_cond_signal(&empty_queue);
            pthread_mutex_unlock(&mutex);
            break;
        }
        //执行该任务;
        // printf("\texecute:\t%s %s\n",task->path, task->string);
        find_file(task->path, task->string);
        pthread_cond_signal(&empty_queue);
        pthread_mutex_unlock(&mutex);
    }
}

void find_file(char *path, char *target)
{
    FILE *file = fopen(path, "r");
    if(file == NULL)
    {
        perror(path);
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, target))
            printf("%s: %s", path, line);
    }

    fclose(file);
}

void find_dir(char *path, char *target)
{
    DIR *dir = opendir(path);
    if(dir == NULL)
    {
        perror(path);
        return;
    }
    struct dirent *entry;
    // readdir用于读取目录所有项，成功读取则返回下一个入口点，否则返回NULL
    while (entry = readdir(dir)) {
        if (strcmp(entry->d_name, ".") == 0)
            continue;

        if (strcmp(entry->d_name, "..") == 0)
            continue;

        if (entry->d_type == DT_DIR) {
            char newPath[256] = {0};
            strcpy(newPath, path);
            strcat(newPath, "/");
            strcat(newPath, entry->d_name);
            find_dir(newPath, target);
        }

        if (entry->d_type == DT_REG) {
            char newPath[256] = {0};
            strcpy(newPath, path);
            strcat(newPath, "/");
            strcat(newPath, entry->d_name);
            // find_file(newPath, target);

            // 新任务加入到任务队列
            pthread_mutex_lock(&mutex);
            while (queue_is_full(&taskqueue))
                pthread_cond_wait(&empty_queue, &mutex);

            Task newTask;
            newTask.is_end = 0;
            strcpy(newTask.path, newPath);
            strcpy(newTask.string ,target);

            put_item(&taskqueue, newTask);
            // printf("newTask:\t%s %s\n", newPath, target);

            pthread_cond_signal(&full_queue);
            pthread_mutex_unlock(&mutex);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    char *path = argv[1];
    char *string = argv[2];

    if (argc != 3) {
        puts("Usage: pfind file string");
        return 0;
    }
    pthread_t thread_pool[WORKER_NUMBER];
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&empty_queue, NULL);
    pthread_cond_init(&full_queue, NULL);

    for(int i = 0; i < WORKER_NUMBER; i++)
        pthread_create(&thread_pool[i], NULL, worker_entry, NULL);

    // 主线程中生产新任务
    struct stat info;
    stat(path, &info);

    if (S_ISDIR(info.st_mode)) {
        // 递归地加入新任务
        find_dir(path, string);

        // 加入n个结束任务
        Task endTask;
        endTask.is_end = 1;
        strcpy(endTask.path, "NULL");
        strcpy(endTask.string, "NULL");

        for(int i = 0; i < WORKER_NUMBER; i++) {
            pthread_mutex_lock(&mutex);
            while (queue_is_full(&taskqueue))
                pthread_cond_wait(&empty_queue, &mutex);

            put_item(&taskqueue,endTask);

            pthread_cond_signal(&full_queue);
            pthread_mutex_unlock(&mutex);
        }
    }
    else
        find_file(path, string);

    for(int i = 0; i < WORKER_NUMBER; i++)
        pthread_join(thread_pool[i], NULL);

    return 0;
}