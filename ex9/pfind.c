/**
 * @file pfind.c
 * @author 陈立文
 * @brief 并行查找，主线程作为生产者生产任务到任务队列，消费者线程并行执行任务队列中的任务，
 *        使用条件变量作为线程同步工具
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
pthread_cond_t cond;
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
    pthread_mutex_lock(&mutex);
    
    while(queue_is_empty(tq)) 
        pthread_cond_wait(&cond, &mutex);
        
    Task *item = &tq->que[tq->out];
    tq->out = (tq->out + 1) % CAPATICY;
    
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    return item;
}

void put_item(TaskQueue *tq, Task item)
{
    pthread_mutex_lock(&mutex);
    
    while(queue_is_full(tq)) 
        pthread_cond_wait(&cond, &mutex);
        
    tq->que[tq->in] = item;
    tq->in = (tq->in + 1) % CAPATICY;
    
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}

void add_norm_task(TaskQueue *tq, char *path, char *target) {
    Task newTask;
    newTask.is_end = 0;
    strcpy(newTask.path, path);
    strcpy(newTask.string, target);
    put_item(&taskqueue, newTask);
}

void add_end_task(TaskQueue *tq) {
    Task newTask;
    newTask.is_end = 1;
    strcpy(newTask.path, "");
    strcpy(newTask.string, "");
    put_item(&taskqueue, newTask);
}

void *worker_entry(void *arg)
{
    while (1) {
        //从任务队列中获取一个任务 task;
        Task *task = get_item(&taskqueue);
        if (task->is_end) {
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
            break;
        }
        //执行该任务;
        find_file(task->path, task->string);
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
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0)
            continue;

        if (strcmp(entry->d_name, "..") == 0)
            continue;

        char newPath[256];
        sprintf(newPath, "%s/%s", path, entry->d_name);
        
        if (entry->d_type == DT_DIR) 
            find_dir(newPath, target);
            
        if (entry->d_type == DT_REG)
            add_norm_task(&taskqueue, newPath, target);
        
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
    pthread_cond_init(&cond, NULL);

    for(int i = 0; i < WORKER_NUMBER; i++)
        pthread_create(&thread_pool[i], NULL, worker_entry, NULL);

    // 主线程中生产新任务
    struct stat info;
    stat(path, &info);

    if (S_ISDIR(info.st_mode))
        find_dir(path, string);
    else
        find_file(path, string);

    for(int i = 0; i < WORKER_NUMBER; i++)
            add_end_task(&taskqueue);

    for(int i = 0; i < WORKER_NUMBER; i++)
        pthread_join(thread_pool[i], NULL);

    return 0;
}