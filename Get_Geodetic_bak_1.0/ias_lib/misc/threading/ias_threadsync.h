#ifndef IAS_THREADSYNC_H
#define IAS_THREADSYNC_H

/* This header file provides a simple wrapper around the thread synchronization
   objects available.  This is to allow for easily changing the synchronization
   objects at a later date if the code needs to be ported to a system that
   doesn't support pthreads or semaphores. */

#include <unistd.h>    /* sysconf prototype */
#include <pthread.h>   /* pthread library */
#include <semaphore.h> /* semaphore library */

typedef pthread_mutex_t IAS_THREAD_MUTEX_TYPE;/* mutex type for posix threads */
typedef pthread_cond_t  IAS_THREAD_COND; /* condition type for posix threads */

/***************************************************************************/
/* mutex manipulation macros for pthreads.  All of them return zero when 
   successful. */
#define IAS_THREAD_CREATE_MUTEX(mutex) pthread_mutex_init(mutex,NULL)

#define IAS_THREAD_DESTROY_MUTEX(mutex) pthread_mutex_destroy(mutex)

#define IAS_THREAD_LOCK_MUTEX(mutex) pthread_mutex_lock(mutex)

#define IAS_THREAD_UNLOCK_MUTEX(mutex) pthread_mutex_unlock(mutex)

#define IAS_THREAD_CREATE_COND(cond) pthread_cond_init(cond,NULL)

#define IAS_THREAD_DESTROY_COND(cond) pthread_cond_destroy(cond)

/***************************************************************************/

typedef sem_t IAS_THREAD_SEMAPHORE_TYPE;     /* semaphore type for unix */

/***************************************************************************/
/* semaphore manipulation macros for posix.  All of them return zero when
   successful. */
#define IAS_THREAD_CREATE_SEMAPHORE(sem,initial_value) \
            sem_init(sem,0,(initial_value))

#define IAS_THREAD_DESTROY_SEMAPHORE(sem) sem_destroy(sem)

#define IAS_THREAD_POST_SEMAPHORE(sem) sem_post(sem)

#define IAS_THREAD_WAIT_SEMAPHORE(sem) sem_wait(sem)
/***************************************************************************/

#define IAS_THREAD_GET_NUM_PROCESSORS() (int)sysconf(_SC_NPROCESSORS_ONLN)
/* Macro to get the number of processors.  This can be used set the
   number of threads to fork. */

#endif
