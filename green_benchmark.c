#include <stdio.h>
#include "green.h"
#include <sys/time.h>
#include <stdlib.h>

#define SIZE 128

green_cond_t empty;
green_cond_t full;
green_mutex_t mutex;

int array[SIZE] = {0};
int loop = 0;
int producer_index = 0;
int consumer_index = 0;
int count = 0;

void *produce(void *id) {
    int p;
    for (int i = 0; i < loop; i++) {
        green_mutex_lock(&mutex);
        while (count == SIZE) {
            green_cond_wait_with_mutex(&empty, &mutex);
        }
        p = rand() % SIZE;
        array[producer_index] = p;
        producer_index = (producer_index + 1) % SIZE;
        count++;
        printf("Producer %d produced %d\n", *(int *) id, p);
        green_cond_signal(&full);
        green_mutex_unlock(&mutex);
    }
}

void *consume(void *id) {
    int c;
    for (int i = 0; i < loop; i++) {
        green_mutex_lock(&mutex);
        while (count == 0) {
            green_cond_wait_with_mutex(&full, &mutex);
        }
        c = array[consumer_index];
        consumer_index = (consumer_index + 1) % SIZE;
        count--;
        printf("Consumer %d consume %d\n", *(int *) id, c);
        green_cond_signal(&empty);
        green_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[]) {
    loop = atoi(argv[1]);
    int test = 10;
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);
    for (int i = 0; i < test; ++i) {
        green_cond_init(&full);
        green_cond_init(&empty);
        green_mutex_init(&mutex);
        green_t p0;
        green_t p1;
        green_t p2;
        green_t p3;

        green_t c0;
        green_t c1;
        green_t c2;
        green_t c3;

        int a0 = 1;
        int a1 = 2;
        int a2 = 3;
        int a3 = 4;

        int a4 = -4;
        int a5 = -3;
        int a6 = -2;
        int a7 = -1;

        green_create(&p0, produce, &a0);
        green_create(&p1, produce, &a1);
        green_create(&p2, produce, &a2);
        green_create(&p3, produce, &a3);

        green_create(&c0, consume, &a4);
        green_create(&c1, consume, &a5);
        green_create(&c2, consume, &a6);
        green_create(&c3, consume, &a7);

        green_join(&p0, NULL);
        green_join(&p1, NULL);
        green_join(&p2, NULL);
        green_join(&p3, NULL);

        green_join(&c0, NULL);
        green_join(&c1, NULL);
        green_join(&c2, NULL);
        green_join(&c3, NULL);
    }


    gettimeofday(&t1, NULL);
    FILE *res_file;

    char f_name[20];
    sprintf(f_name, "green.csv");
    res_file = fopen(f_name, "w");
    fprintf(res_file, "%d, %.5f\n", loop,
            (unsigned long) (((t1.tv_sec * 1e6) + t1.tv_usec) - ((t0.tv_sec * 1e6) + t0.tv_usec)) / (double) (1e6*test));
    printf("%d, %.5f\n", loop,
           (unsigned long) (((t1.tv_sec * 1e6) + t1.tv_usec) - ((t0.tv_sec * 1e6) + t0.tv_usec)) / (double) (1e6*test));
    fclose(res_file);
}