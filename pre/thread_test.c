#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int another_shared = 0;

void* thread_run(void* arg)
{
    int* calculator = (int*)arg;
    printf("hello world, thid == %ld,the value is %d\n", pthread_self(),*calculator);
    for (int i = 0;i < 1000;i++) {
        *calculator += 1;
        another_shared += 1;
    }
}

int main(void)
{
    int calculator = 0;
    pthread_t thid1;
    pthread_t thid2;
    pthread_create(&thid1, NULL, thread_run, &calculator);
    pthread_create(&thid2, NULL, thread_run, &calculator);

    pthread_join(thid1, NULL);
    pthread_join(thid2, NULL);

    printf("calculator is %d \n", calculator);
    printf("another_shared is %d \n", another_shared);
}