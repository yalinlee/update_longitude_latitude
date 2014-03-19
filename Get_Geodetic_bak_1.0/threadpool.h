///*
// * threadpool.h
// *
// *  Created on: Mar 10, 2014
// *      Author: LYL
// */


#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

/***************************************************************************************
 * @file threadpool.h
 * @brief Threadpool Header File
 *
 *              _______________________________________________________
 * 			   /													   \
 *             |   JOB QUEUE        | job1 | job2 | job3 | job4 | ..   |
 *             |-------------------------------------------------------|                                                      |
 *             |   threadpool      | thread1 | thread2 | ..            |
 *             \_______________________________________________________/
 *
 *    Description:       Jobs are added to the job queue. Once a thread in the pool
 *                       is idle, it is assigned with the first job from the queue(and
 *                       erased from the queue). It's each thread's job to read from
 *                       the queue serially(using lock) and executing each job
 *                       until the queue is empty.
 ****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define QUEUE_SIZE 8192
#define THREAD_SHUTDOWN_GRACEFUL 1

/******************************************************************************
 *  @struct ways of shutdown for threadpool
 *  @brief The enum of shutdown ways for threadpool
 *******************************************************************************/
typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown  = 2
} threadpool_shutdown_t;



/******************************************************************************
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 *****************************************************************************/

typedef struct threadpool_task_t{
    void (*function)(void *);
    void *argument;
} threadpool_task_t;




/******************************************************************************
 *  @struct threadpool
 *  @brief The threadpool struct
 *
 *  @var notify       Condition variable to notify worker threads.
 *  @var threads      Array containing worker threads ID.
 *  @var thread_count Number of threads
 *  @var queue        Array containing the task queue.
 *  @var queue_size   Size of the task queue.
 *  @var head         Index of the first element.
 *  @var tail         Index of the next element.
 *  @var count        Number of pending tasks
 *  @var shutdown     Flag indicating if the pool is shutting down
 *  @var started      Number of started threads
 *******************************************************************************/
typedef struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  threadpool_task_t *queue;
  int thread_count;
  int queue_size;
  int head;
  int tail;
  int count;
  int shutdown;
  int started;
} threadpool_t;



/******************************************************************************
 *  @struct threadpool_error
 *  @brief The enum of threadpool error
 *
 *******************************************************************************/
typedef enum {
    threadpool_invalid        = -1,
    threadpool_lock_failure   = -2,
    threadpool_queue_full     = -3,
    threadpool_shutdown       = -4,
    threadpool_thread_failure = -5
} threadpool_error_t;



/******************************************************************************
 * @function threadpool_create
 * @brief Creates a threadpool_t object.
 * @param thread_count Number of worker threads.
 * @param queue_size   Size of the queue.
 * @return a newly created thread pool or NULL
 ******************************************************************************/
threadpool_t *threadpool_create(int thread_count);



/*******************************************************************************
 * @function threadpool_add
 * @brief add a new task in the queue of a thread pool
 * @param pool     Thread pool to which add the task.
 * @param function Pointer to the function that will perform the task.
 * @param argument Argument to be passed to the function.
 * @return 0 if all goes well, negative values in case of error (@see
 * threadpool_error_t for codes).
 *******************************************************************************/
int threadpool_add(threadpool_t *pool, void (*routine)(void *),
                   void *arg);





/******************************************************************************
 * @function threadpool_destroy
 * @brief Stops and destroys a thread pool.
 * @param pool  Thread pool to destroy.
 *
 * Known values for flags are 0 (default) and threadpool_graceful in
 * which case the thread pool doesn't accept any new tasks but
 * processes all pending tasks before shutdown.
 ******************************************************************************/
int threadpool_destroy(threadpool_t *pool);

#endif /* _THREADPOOL_H_ */
