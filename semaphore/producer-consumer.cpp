/* 2014
 * Maciej Szeptuch
 * IIUWr
 * ----------
 *
 *  producer-consumer problem solution using custom implemented semaphores.
 */

#include <cstdio>
#include <cctype>
#include <pthread.h>
#include <cerrno>
#include <queue>
#include <algorithm>
#include "semaphore_spin.h"
#include "semaphore_list.h"

#define MAX_THREADS 64

struct thread_info
{
    pthread_t   id;
    int         num;
};

struct thread_info      thread[MAX_THREADS];
struct semaphore_list   listlock;
std::queue<int>         jobs;

void *listlock_thread_producer(void *arg);
void *listlock_thread_consumer(void *arg);

int main(int argc, char *argv[])
{
    srand(time(nullptr));

    sigset_t        signal_mask;
    pthread_attr_t  attr;
    int             producer_threads;
    int             consumer_threads;
    int             t;

    int             err;

    if(argc != 3)
    {
        fprintf(stderr, "usage: %s num_producer_threads num_consumer_threads\n", argv[0]);
        exit(1);
    }

    producer_threads   = strtoul(argv[1], NULL, 0);
    consumer_threads    = strtoul(argv[2], NULL, 0);
    if( 0 > producer_threads || producer_threads > MAX_THREADS
    ||  0 > consumer_threads  || consumer_threads > MAX_THREADS
    ||  (!producer_threads && !consumer_threads))
    {
        fprintf(stderr, "invalid number of threads\n");
        exit(2);
    }

    if((err = pthread_attr_init(&attr)))
    {
        errno = err;
        perror("pthread_attr_init");
        exit(3);
    }

    /* INITIALIZE */
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGCONT);
    if((err = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL)))
    {
        errno = err;
        perror("pthread_sigmask");
        exit(4);
    }

    semaphore_list_init(&listlock);
    t = 0;
    while(t < producer_threads)
    {
        thread[t].num = t + 1;
        if((err = pthread_create(&thread[t].id, &attr, &listlock_thread_producer, &thread[t])))
        {
            errno = err;
            perror("listlock_producer: pthread_create");
            exit(5);
        }

        ++ t;
    }

    while(t < producer_threads + consumer_threads)
    {
        thread[t].num = t + 1;
        if((err = pthread_create(&thread[t].id, &attr, &listlock_thread_consumer, &thread[t])))
        {
            errno = err;
            perror("listlock_consumer: pthread_create");
            exit(5);
        }

        ++ t;
    }

    t = 0;
    while(t < producer_threads + consumer_threads)
    {
        if((err = pthread_join(thread[t].id, NULL)))
        {
            errno = err;
            perror("listlock: pthread_join");
            exit(6);
        }

        ++ t;
    }

    /* CLEANUP */
    if((err = pthread_attr_destroy(&attr)))
    {
        errno = err;
        perror("pthread_attr_destroy");
        exit(9);
    }

    return 0;
}

inline
void *listlock_thread_producer(void *arg)
{
    struct thread_info *tinfo = (struct thread_info *) arg;
    printf("Listlock producer thread id:%d staring\n", tinfo->num);

    int i = 0;
    while(i < 30)
    {
        int num = rand();
        sleep(1);
        semaphore_list_wait(&listlock);
        printf("Listlock producer thread id:%d produced: %d\n", tinfo->num, num);
        jobs.push(num);
        semaphore_list_signal(&listlock);
        ++ i;
    }

    printf("Listlock producer thread id:%d ending\n", tinfo->num);
    return NULL;
}

inline
void *listlock_thread_consumer(void *arg)
{
    struct thread_info *tinfo = (struct thread_info *) arg;
    printf("Listlock consumer thread id:%d starting\n", tinfo->num);

    while(true)
    {
        semaphore_list_wait(&listlock);
        if(!jobs.empty())
        {
            printf("Listlock consumer thread id:%d got the %d job\n", tinfo->num, jobs.front());
            jobs.pop();
            sleep(1);
        }
        semaphore_list_signal(&listlock);
    }

    printf("Listlock consumer thread id:%d ending\n", tinfo->num);
    return NULL;
}
