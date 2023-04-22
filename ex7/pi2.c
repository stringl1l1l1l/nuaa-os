#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXN 100000
#define CPU_CNT 10
#define WORK_AMT (MAXN / CPU_CNT)

// 计算第[beg，end]项的和
typedef struct Param
{
    int beg;
    int end;
} Param;

typedef struct Result
{
    double sum;
} Result;

void *leibniz(void *args)
{
    Param *param;
    Result *res;

    param = (Param*)args;
    res = (Result*)malloc(sizeof(Result));
    res->sum = 0;

    for(int i = param->beg; i <= param->end; i++)
        res->sum += ((i&1)? 1.0 : -1.0) / (2.0 * i - 1);

    return res;
}

int main()
{
    double res = 0;
    for(int i = 0;i < CPU_CNT; i++){
        pthread_t worker_tid;
        Param param;
        Result *result;

        param.beg = i * WORK_AMT + 1;
        param.end = (i + 1) * WORK_AMT;

        pthread_create(&worker_tid, NULL, &leibniz, &param);
        pthread_join(worker_tid, (void **)&result);
        res += result->sum;
        printf("Thread%d: leibniz[%d, %d], sum = %lf\n", i, param.beg, param.end, result->sum);
        free(result);
    }
    printf("PI ≈ %lf\n", res * 4.0);
}