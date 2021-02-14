#ifndef GREEN_H
#define GREEN_H

#include <ucontext.h>
#include <stdlib.h>

typedef struct green_t {
    ucontext_t *context;

    void *(*fun)(void *);

    void *arg;
    struct green_t *next;
    struct green_t *join;
    void *retval;
    int zombie;
} green_t;
typedef struct queue {
    green_t *front, *rear;
} queue;

typedef struct green_cond_t {
    green_t *susp_que;
} green_cond_t;

typedef struct green_mutex_t {
    volatile int taken;
    green_t *susp;
} green_mutex_t;


int green_create(green_t *thread, void *(*fun)(void *), void *arg);

int green_yield();

int green_join(green_t *thread, void **val);

void green_cond_init(green_cond_t *);

void green_cond_wait(green_cond_t *);

void green_cond_wait_with_mutex(green_cond_t *, green_mutex_t *);

void green_cond_signal(green_cond_t *);

void timer_handler(int);

int green_mutex_init(struct green_mutex_t *mutex);

int green_mutex_lock(struct green_mutex_t *mutex);

int green_mutex_unlock(struct green_mutex_t *mutex);

static void enqueue_thread(green_t **q, green_t *new_thread) {
    green_t *temp = *q;
    if (temp == NULL) {
        *q = new_thread;
    } else {
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = new_thread;
    }
}

static green_t *dequeue_thread(green_t **q) {
    green_t *ret = *q;
    if (ret) {
        *q = ret->next;
        ret->next = NULL;
    }
    return ret;
}


#endif