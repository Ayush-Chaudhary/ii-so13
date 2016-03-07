/* 2014
 * Maciej Szeptuch
 * IIUWr
 * ----------
 *
 *  Simple semaphore test.
 */

#include <stdio.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include "semaphore_spin.h"
#include "semaphore_list.h"

#define MAX_THREADS 64

struct thread_info
{
    pthread_t   id;
    int         num;
};

struct thread_info      thread[MAX_THREADS];
struct semaphore_spin   spinlock;
struct semaphore_list   listlock;

void *spinlock_thread_start(void *arg);
void *listlock_thread_start(void *arg);

int main(int argc, char *argv[])
{
    srand(time(0));

    sigset_t        signal_mask;
    pthread_attr_t  attr;
    int             spin_threads;
    int             list_threads;
    int             t;

    int             err;

    if(argc != 3)
    {
        fprintf(stderr, "usage: %s num_spinlock_threads num_listlock_threads\n", argv[0]);
        exit(1);
    }

    spin_threads = strtoul(argv[1], NULL, 0);
    list_threads = strtoul(argv[2], NULL, 0);
    if( 0 > spin_threads || spin_threads > MAX_THREADS
    ||  0 > list_threads || list_threads > MAX_THREADS
    ||  (!spin_threads && !list_threads))
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

    /* SPINLOCK TEST */
    semaphore_spin_init(&spinlock);
    t = 0;
    while(t < spin_threads)
    {
        thread[t].num = t + 1;
        if((err = pthread_create(&thread[t].id, &attr, &spinlock_thread_start, &thread[t])))
        {
            errno = err;
            perror("spinlock: pthread_create");
            exit(5);
        }

        ++ t;
    }

    t = 0;
    while(t < spin_threads)
    {
        if((err = pthread_join(thread[t].id, NULL)))
        {
            errno = err;
            perror("spinlock: pthread_join");
            exit(6);
        }

        ++ t;
    }

    /* LISTLOCK TEST */
    semaphore_list_init(&listlock);
    t = 0;
    while(t < list_threads)
    {
        thread[t].num = t + 1;
        if((err = pthread_create(&thread[t].id, &attr, &listlock_thread_start, &thread[t])))
        {
            errno = err;
            perror("listlock: pthread_create");
            exit(7);
        }

        ++ t;
    }

    t = 0;
    while(t < list_threads)
    {
        if((err = pthread_join(thread[t].id, NULL)))
        {
            errno = err;
            perror("listlock: pthread_join");
            exit(8);
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
void *spinlock_thread_start(void *arg)
{
    struct thread_info *tinfo = (struct thread_info *) arg;
    printf("Spinlock thread %d starting\n", tinfo->num);

    int i = 0;
    while(i < 30)
    {
        semaphore_spin_wait(&spinlock);
        printf("[%d] Spinlock thread %d got the spinlock\n", i, tinfo->num);
        int j = 0;
        int num = 1;
        for(j = 0; j < 10000000; ++ j)
            num *= rand();

        semaphore_spin_signal(&spinlock);
        ++ i;
    }

    printf("Spinlock thread %d ending\n", tinfo->num);
    return NULL;
}

inline
void *listlock_thread_start(void *arg)
{
    struct thread_info *tinfo = (struct thread_info *) arg;
    printf("Listlock thread %d starting\n", tinfo->num);

    int i = 0;
    while(i < 30)
    {
        semaphore_list_wait(&listlock);
        printf("[%d] Listlock thread %d got the listlock\n", i, tinfo->num);
        int j = 0;
        int num = 1;
        for(j = 0; j < 10000000; ++ j)
            num *= rand();

        semaphore_list_signal(&listlock);
        ++ i;
    }

    printf("Listlock thread %d ending\n", tinfo->num);
    return NULL;
}
