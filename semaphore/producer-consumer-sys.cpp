/* 2014
 * Maciej Szeptuch
 * IIUWr
 * ----------
 *
 *  producer-consumer problem solution using linux system semaphores.
 */

#include <cstdio>
#include <cctype>
#include <pthread.h>
#include <cerrno>
#include <queue>
#include <algorithm>
#include <semaphore.h>
#include <unistd.h>

#define MAX_THREADS 64

struct thread_info
{
    pthread_t   id;
    int         num;
};

struct thread_info  thread[MAX_THREADS];
sem_t               lock;
bool                locked;
std::queue<int>     jobs;

void *semaphore_thread_producer(void *arg);
void *semaphore_thread_consumer(void *arg);

int main(int argc, char *argv[])
{
    srand(time(nullptr));

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
    ||  0 > consumer_threads || consumer_threads > MAX_THREADS
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
    if(sem_init(&lock, 0, 0) == -1)
    {
        perror("sem_init");
        exit(4);
    }

    t = 0;
    while(t < producer_threads)
    {
        thread[t].num = t + 1;
        if((err = pthread_create(&thread[t].id, &attr, &semaphore_thread_producer, &thread[t])))
        {
            errno = err;
            perror("semaphore_producer: pthread_create");
            exit(5);
        }

        ++ t;
    }

    while(t < producer_threads + consumer_threads)
    {
        thread[t].num = t + 1;
        if((err = pthread_create(&thread[t].id, &attr, &semaphore_thread_consumer, &thread[t])))
        {
            errno = err;
            perror("semaphore_consumer: pthread_create");
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
            perror("semaphore: pthread_join");
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
void *semaphore_thread_producer(void *arg)
{
    struct thread_info *tinfo = (struct thread_info *) arg;
    printf("Semaphore producer thread id:%d staring\n", tinfo->num);

    int i = 0;
    while(i < 30)
    {
        int num = rand();
        sleep(1);
        while(locked) sem_wait(&lock);
        locked = true;
        printf("Semaphore producer thread id:%d produced: %d\n", tinfo->num, num);
        jobs.push(num);
        locked = false;
        sem_post(&lock);
        ++ i;
    }

    printf("Semaphore producer thread id:%d ending\n", tinfo->num);
    return NULL;
}

inline
void *semaphore_thread_consumer(void *arg)
{
    struct thread_info *tinfo = (struct thread_info *) arg;
    printf("Semaphore consumer thread id:%d starting\n", tinfo->num);

    while(true)
    {
        while(locked) sem_wait(&lock);
        locked = true;
        if(!jobs.empty())
        {
            printf("Semaphore consumer thread id:%d got the %d job\n", tinfo->num, jobs.front());
            jobs.pop();
            sleep(1);
        }

        locked = false;
        sem_post(&lock);
    }

    printf("Semaphore consumer thread id:%d ending\n", tinfo->num);
    return NULL;
}
