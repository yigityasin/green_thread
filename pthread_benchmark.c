#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>

#define SIZE 128

pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int array[SIZE] = {0};
int loop = 0;
int producer_index = 0;
int consumer_index = 0;
int count = 0;

void *produce(void *id) {
    int p;
    for (int i = 0; i < loop; i++) {
        pthread_mutex_lock(&mutex);
        while (count == SIZE) {
            pthread_cond_wait(&empty, &mutex);
        }
        p = rand() % SIZE;
        array[producer_index] = p;
        producer_index = (producer_index + 1) % SIZE;
        count++;
        printf("Producer %d produced %d\n", *(int *) id, p);
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
    }
}

void *consume(void *id) {
    int c;
    for (int i = 0; i < loop; i++) {
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            pthread_cond_wait(&full, &mutex);
        }
        c = array[consumer_index];
        consumer_index = (consumer_index + 1) % SIZE;
        count--;
        printf("Consumer %d consume %d\n", *(int *) id, c);
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
    }
}


int main(int argc, char *argv[]) {

    loop = atoi(argv[1]);
    int test = 10;
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);
    for (int i = 0; i < test; ++i) {
        pthread_t p0;
        pthread_t p1;
        pthread_t p2;
        pthread_t p3;

        pthread_t c0;
        pthread_t c1;
        pthread_t c2;
        pthread_t c3;

        int a0 = 1;
        int a1 = 2;
        int a2 = 3;
        int a3 = 4;

        int a4 = -4;
        int a5 = -3;
        int a6 = -2;
        int a7 = -1;


        pthread_create(&p0, NULL, produce, &a0);
        pthread_create(&p1, NULL, produce, &a1);
        pthread_create(&p2, NULL, produce, &a2);
        pthread_create(&p3, NULL, produce, &a3);

        pthread_create(&c0, NULL, consume, &a4);
        pthread_create(&c1, NULL, consume, &a5);
        pthread_create(&c2, NULL, consume, &a6);
        pthread_create(&c3, NULL, consume, &a7);

        pthread_join(p0, NULL);
        pthread_join(p1, NULL);
        pthread_join(p2, NULL);
        pthread_join(p3, NULL);

        pthread_join(c0, NULL);
        pthread_join(c1, NULL);
        pthread_join(c2, NULL);
        pthread_join(c3, NULL);
    }


    gettimeofday(&t1, NULL);
    FILE *res_file;

    char f_name[20];
    sprintf(f_name, "pthread.csv");
    res_file = fopen(f_name, "w");
    fprintf(res_file, "%d, %.5f\n", loop,
            (unsigned long) (((t1.tv_sec * 1e6) + t1.tv_usec) - ((t0.tv_sec * 1e6) + t0.tv_usec)) / (double) (1e6*test));
    printf("%d, %.5f\n", loop,
           (unsigned long) (((t1.tv_sec * 1e6) + t1.tv_usec) - ((t0.tv_sec * 1e6) + t0.tv_usec)) / (double) (1e6*test));
    fclose(res_file);
}