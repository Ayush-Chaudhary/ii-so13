/* 2014
 * Maciej Szeptuch
 * IIUWr
 * ----------
 *
 *  Simple spinning semaphore.
 */

#ifndef __SEMAPHORE_SPIN_H__
#define __SEMAPHORE_SPIN_H__

#include <unistd.h>

struct semaphore_spin
{
    int sem;
};

inline
void semaphore_spin_init(struct semaphore_spin* semaphore)
{
    semaphore->sem = 1;
}

inline
void semaphore_spin_wait(struct semaphore_spin* semaphore)
{
    while(!__sync_bool_compare_and_swap(&semaphore->sem, 1, 0))
        sleep(0);
}

inline
void semaphore_spin_signal(struct semaphore_spin* semaphore)
{
    __sync_bool_compare_and_swap(&semaphore->sem, 0, 1);
}

#endif // __SEMAPHORE_SPIN_H__
