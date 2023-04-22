#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define MAXN 100000

double sum_master = 0;
double sum_worker = 0;

void *leibniz(void *args)
{
    for(int i = MAXN / 2 + 1; i <= MAXN; i++)
    {
        sum_worker += ((i&1)? 1.0 : -1.0) / (2.0 * i - 1);
    }
}

int main()
{
    double res = 0;
    pthread_t worker_tid;
    pthread_create(&worker_tid, NULL, &leibniz, NULL);
    pthread_join(worker_tid, NULL);
    printf("worker: leibniz[%d, %d], sum = %lf\n", MAXN / 2 + 1, MAXN, sum_worker);
    for(int i = 1; i <= MAXN / 2; i++)
    {
        sum_master += ((i&1)? 1.0 : -1.0) / (2.0 * i - 1);
    }
    printf("master: leibniz[%d, %d], sum = %lf\n", 1, MAXN / 2, sum_master);
    res = sum_master + sum_worker;
    printf("PI ≈ %lf\n", res * 4.0);
}