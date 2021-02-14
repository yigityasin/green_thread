#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include "green.h"


#define PERIOD 100
#define FALSE 0
#define TRUE 1
#define STACK_SIZE 4096

static sigset_t block;
static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, FALSE};
static green_t *running = &main_green;

static void init() __attribute__((constructor));

void init() {
    getcontext(&main_cntx);
    sigemptyset(&block);
    sigaddset(&block, SIGVTALRM);

    struct sigaction act = {0};
    struct timeval interval;
    struct itimerval period;

    act.sa_handler = timer_handler;
    assert(sigaction(SIGVTALRM, &act, NULL) == 0);

    interval.tv_sec = 0;
    interval.tv_usec = PERIOD;
    period.it_interval = interval;
    period.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &period, NULL);
}

void green_thread() {

    sigprocmask(SIG_UNBLOCK, &block, NULL);
    green_t *this = running;
    (*this->fun)(this->arg);
    sigprocmask(SIG_BLOCK, &block, NULL);

    if (this->join) {
        enqueue_thread(&running, this->join);
    }

    this->zombie = TRUE;
    dequeue_thread(&running);
    setcontext(running->context);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

int green_yield() {
    sigprocmask(SIG_BLOCK, &block, NULL);
    green_t *susp = running;
    enqueue_thread(&running, susp);
    dequeue_thread(&running);
    swapcontext(susp->context, running->context);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_join(green_t *thread, void **res) {
    if (!thread->zombie) {
        green_t *susp = running;
        sigprocmask(SIG_BLOCK, &block, NULL);
        if (thread->join == NULL) {
            thread->join = susp;
        } else {
            green_t *temp = thread->join;
            while (temp->next) {
                temp = temp->next;
            }
            temp->next = susp;
        }
        dequeue_thread(&running);
        swapcontext(susp->context, running->context);
    }
    free(thread->context->uc_stack.ss_sp);
    free(thread->context);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_create(green_t *new, void *(*fun)(void *), void *arg) {
    sigprocmask(SIG_BLOCK, &block, NULL);
    ucontext_t *cntx = (ucontext_t *) malloc(sizeof(ucontext_t));
    getcontext(cntx);
    void *stack = malloc(STACK_SIZE);
    cntx->uc_stack.ss_sp = stack;
    cntx->uc_stack.ss_size = STACK_SIZE;
    makecontext(cntx, green_thread, 0);
    new->context = cntx;
    new->fun = fun;
    new->arg = arg;
    new->next = NULL;
    new->join = NULL;
    new->retval = NULL;
    new->zombie = FALSE;
    enqueue_thread(&running, new);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

void green_cond_init(green_cond_t *cond) {
    cond->susp_que = NULL;
}

void green_cond_wait(green_cond_t *cond) {
    sigprocmask(SIG_BLOCK, &block, NULL);
    green_t *susp = running;
    enqueue_thread(&(cond->susp_que), susp);
    dequeue_thread(&running);
    swapcontext(susp->context, running->context);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

void green_cond_wait_with_mutex(green_cond_t *cond, green_mutex_t *mutex) {
    sigprocmask(SIG_BLOCK, &block, NULL);
    green_t *susp = running;
    enqueue_thread(&(cond->susp_que), susp);

    if (mutex != NULL) {
        mutex->taken = FALSE;
        enqueue_thread(&running, mutex->susp);
        mutex->susp = NULL;
    }

    dequeue_thread(&running);
    swapcontext(susp->context, running->context);

    if (mutex != NULL) {
        if (mutex->taken) {
            while (mutex->taken) {

                green_t *s = running;
                enqueue_thread(&(mutex->susp), s);
                dequeue_thread(&running);
                swapcontext(s->context, running->context);
            }
        } else {
            mutex->taken = TRUE;
        }
    }
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

void green_cond_signal(green_cond_t *cond) {
    sigprocmask(SIG_BLOCK, &block, NULL);
    green_t *awake = dequeue_thread(&(cond->susp_que));
    enqueue_thread(&running, awake);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}


void timer_handler(int time) {
    green_yield();
}

int green_mutex_init(green_mutex_t *mutex) {
    mutex->taken = FALSE;
    mutex->susp = NULL;
    return 0;
}

int green_mutex_lock(struct green_mutex_t *mutex) {
    sigprocmask(SIG_BLOCK, &block, NULL);
    green_t *susp = running;
    if (mutex->taken) {
        while (mutex->taken) {
            enqueue_thread(&(mutex->susp), susp);
            dequeue_thread(&running);
            swapcontext(susp->context, running->context);
        }
        mutex->taken = TRUE;

    } else {
        mutex->taken = TRUE;
    }
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_mutex_unlock(struct green_mutex_t *mutex) {
    sigprocmask(SIG_BLOCK, &block, NULL);
    enqueue_thread(&running, mutex->susp);
    mutex->susp = NULL;
    mutex->taken = FALSE;
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

