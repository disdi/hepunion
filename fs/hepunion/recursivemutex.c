/**
 * \file recursivemutex.c
 * \brief Recursive mutex for Linux
 * \author Pierre Schweitzer <pierre.jean.schweitzer@cern.ch>
 * \version 1.0
 * \date 13-Mar-2012
 * \copyright GNU General Public License - GPL
 *
 * Functions to provite a recursive mutex since Linux kernel
 * doesn't provide any reentrant lock mechanism
 */

#include "recursivemutex.h"

void recursive_mutex_init(recursive_mutex_t *mutex) {
	/* Simply init everything */
	atomic_set(&mutex->count, 0);
	spin_lock_init(&mutex->lock);
	mutex->owner = NULL;
}

void recursive_mutex_lock(recursive_mutex_t *mutex) {
	struct task_struct *task =current;
        struct thread_info* ti= task_thread_info(task);
	/* Increase reference count */
	int count = atomic_add_return(1, &mutex->count);
	/* If noone was locking it, lock */
	if (count == 1) {
		spin_lock(&mutex->lock);
		/* And set owner */
		mutex->owner = ti;
	} else {
		/* Otherwise, someone was locking, then
		 * ensure it's no ourselves.
		 * If it's ourselves, just do nothing
		 * If not, wait for spin lock
		 */
		if (mutex->owner != ti) {
			spin_lock(&mutex->lock);
		}
	}
}

void recursive_mutex_unlock(recursive_mutex_t *mutex) {
	/* Decrease reference count */
	int count = atomic_sub_return(1, &mutex->count);
	/* If count reached 0, no one is locking anymore */
	if (count == 0) {
		/* So release spin lock & lock */
		mutex->owner = NULL;
		spin_unlock(&mutex->lock);
	}
}
