#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define N 100

typedef struct Param
{
    int *a;
    int l;
    int r;
} Param;

void random_array(int arr[], int n, int beg, int end)
{
    srand((unsigned)time(NULL));
    for(int i = 0; i < n; i++)
        arr[i] = rand() % end + beg;
}

void merge(int a[], int l, int mid, int r)
{
    int *temp = (int *)malloc(sizeof(int) * N);
    int i = l, j = mid + 1, k = 0;
    while(i <= mid && j <= r)
    {
        if(a[i] <= a[j]) temp[k++] = a[i++];
        else temp[k++] = a[j++];
    }
    while(i <= mid) temp[k++] = a[i++];
    while(j <= r) temp[k++] = a[j++];

    for(int i = 0; i < k; i++)
        a[i + l] = temp[i];
    free(temp);
}

void *select_sort(void *args)
{
    Param *param;
    param = (Param *)args;
    int *a = param->a;
    int l = param->l, r = param->r;

    for(int i = l; i <= r; i++)
    {
        int k = i;
        for(int j = i + 1; j <= r ;j++)
        {
            if(a[k] > a[j])
                k = j;
        }
        if(k != i)
        {
            int temp = a[i];
            a[i] = a[k];
            a[k] = temp;
        }
    }
}

void print_array(int a[], int beg, int end, const char *msg)
{
    printf("%s: [", msg);
    for(int i = beg; i <= end; i++) {
        if((i - beg) % 5 == 0) puts("");
        printf("%d\t",a[i]);
    }
    puts("\n]\n");
}

int main()
{
    int a[N] = {0};
    random_array(a, N, 0, 400);
    print_array(a, 0, N - 1, "Origin random array");

    int l = 0, mid = N >> 1, r = N - 1;
    Param param1, param2;
    param1.a = param2.a = a;
    param1.l = l, param1.r = mid;
    param2.l = mid + 1, param2.r = r;

    pthread_t td1_tid, td2_tid;
    pthread_create(&td1_tid, NULL, select_sort, &param1);
    pthread_create(&td2_tid, NULL, select_sort, &param2);

    char msg[100] = {0};
    pthread_join(td1_tid, NULL);
    sprintf(msg, "Thread1: a[%d - %d]",l ,mid);
    print_array(a, l, mid, msg);

    pthread_join(td2_tid, NULL);
    sprintf(msg, "Thread2: a[%d - %d]",mid + 1 ,r);
    print_array(a, mid + 1, r, msg);

    merge(a, l, mid, r);
    print_array(a, 0, N - 1, "Sorted array");
}