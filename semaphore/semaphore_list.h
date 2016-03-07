/* 2014
 * Maciej Szeptuch
 * IIUWr
 * ----------
 *
 *  Simple semaphore implementation with list.
 */
#ifndef __SEMAPHORE_LIST_H__
#define __SEMAPHORE_LIST_H__

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "semaphore_spin.h"

struct node
{
    pthread_t   thread;
    struct node *next;
};

struct semaphore_list
{
    int                     sem;
    struct semaphore_spin   list_semaphore;
    struct node             *first;
    struct node             *last;
};

inline
void semaphore_list_init(struct semaphore_list* semaphore)
{
    semaphore->sem      = 1;
    semaphore->first    = NULL;
    semaphore->last     = NULL;
    semaphore_spin_init(&semaphore->list_semaphore);
}

inline
void semaphore_list_wait(struct semaphore_list* semaphore)
{
    struct node *node = NULL;
    sigset_t    signal_mask;
    int         signal;

    semaphore_spin_wait(&semaphore->list_semaphore);
    if(__sync_bool_compare_and_swap(&semaphore->sem, 1, 0))
    {
        semaphore_spin_signal(&semaphore->list_semaphore);
        return;
    }

    node = (struct node *) malloc(sizeof(struct node));
    node->thread = pthread_self();
    node->next = NULL;
    if(!semaphore->last)
        semaphore->first = semaphore->last = node;

    else
        semaphore->last = semaphore->last->next = node;

    semaphore_spin_signal(&semaphore->list_semaphore);

    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGCONT);
    sigwait(&signal_mask, &signal);
}

inline
void semaphore_list_signal(struct semaphore_list* semaphore)
{
    semaphore_spin_wait(&semaphore->list_semaphore);
    struct node *node = semaphore->first;
    if(node)
    {
        semaphore->first = semaphore->first->next;
        if(!semaphore->first)
            semaphore->last = NULL;

        pthread_kill(node->thread, SIGCONT);
        free(node);
    }

    else
        __sync_bool_compare_and_swap(&semaphore->sem, 0, 1);

    semaphore_spin_signal(&semaphore->list_semaphore);
}

#endif // __SEMAPHORE_LIST_H__
