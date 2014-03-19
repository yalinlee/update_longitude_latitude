/**
 * @file threadpool.c
 * @brief Threadpool implementation file
 */

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "threadpool.h"

/**
 * @function void *threadpool_thread(void *threadpool)
 * @brief the worker thread
 * @param threadpool the pool which own the thread
 */


/* function prototype */
static void *threadpool_thread(void *threadpool);

int threadpool_free(threadpool_t *pool);





/* *********************************************************************************
 * NAME:			threadpool_create
 *
 * PURPOSE:	create a thread pool and intialise the parameters
 *
 * RETURN: pointer point to a newly created thread pool or NULL
 * *********************************************************************************/

threadpool_t* threadpool_create(int threadCount)
{
	threadpool_t* pool;
	int i;
	int status;

	/* Check for negative or otherwise very big input parameters */
	if (!threadCount || threadCount<1)
	{
		printf("Error:parameter threadCount should not be negative or zero!\n");
		return NULL;
	}

	/* Allocate memory for the thread pool */
	pool = (threadpool_t*)malloc(sizeof(threadpool_t));
	if(pool == NULL)
	{
		printf("Error:Failed to allocate memory for the thread pool!\n");
		goto err;
	}

	/* Initialize the thread pool */
	pool->thread_count = 0;
	pool->queue_size = QUEUE_SIZE;
	pool->head = pool->tail = pool->count = 0;
	pool->shutdown = 0;
	pool->started = 0;

	/* Allocate thread and task queue */
	pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * threadCount);
	if(pool->threads == NULL)
	{
		printf("Error:Failed to allocate memory for the threads in the pool!\n"
				"");
		goto err;
	}

    pool->queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * pool->queue_size);
	if(pool->queue == NULL )
    {
		printf("Error:Failed to allocate memory for the task queue!\n");
		goto err;
	}

	/* Initialize mutex and conditional variable first */
	if((pthread_mutex_init(&(pool->lock),NULL) != 0)   ||
	   (pthread_cond_init(&(pool->notify),NULL) != 0)  )
	{
		printf("Error:Failed to initialize mutex and conditional variable!\n");
		goto err;
	}

	/* Start work threads */
	for(i = 0; i < threadCount; i++)
	{
		status = pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void*)pool);
		if(status != 0)
		{
			threadpool_destroy(pool);
			return NULL;
		}
		pool->thread_count++;
		pool->started++;
	}

	return pool;

err:
	if(pool)
	{
		threadpool_free(pool);
	}
	return NULL;
}


/* *********************************************************************************
 * NAME:			threadpool_add
 *
 * PURPOSE:	add a new task in the queue of a thread pool
 *
 * RETURN: 0 if all goes well, negative values in case of error (@see
 *         threadpool_error_t for codes)
 * *********************************************************************************/
int threadpool_add(threadpool_t* pool, void(*function)(void *), void* arguments)
{
	int err = 0;
	int next;
	int status;

	/* Check the thread pool and function */
	if(pool == NULL || function == NULL)
	{
		printf("Error:failed to get pool or function!\n");
		return threadpool_invalid;
	}

	/* LOCK the pool*/
	if(pthread_mutex_lock(&(pool->lock)) != 0)
	{
		printf("Error:failed to lock the pool!\n");
		return threadpool_lock_failure;
	}

	next = pool->tail + 1;
	next = (next == pool->queue_size)? 0 : next;

	do
	{
		/* check whether the queue is full */
		if(pool->count == pool->queue_size)
		{
			printf("Error:the queue is full!\n");
			err = threadpool_queue_full;
		}

		/* check whether we shutdown */
		if(pool->shutdown)
		{
			err = threadpool_shutdown;
			break;
		}

		/* add task to the queue */
		pool->queue[pool->tail].function = function;
		pool->queue[pool->tail].argument = arguments;
		pool->tail = next;
		pool->count += 1;

		/* pthread cond broadcast */
		status = pthread_cond_signal(&(pool->notify));
		if(status != 0)
		{
			printf("Error:failed to broadcast cond signal!\n");
			err = threadpool_lock_failure;
			break;
		}
	}while(0);

	/* UNLOCK the pool */
	status = pthread_mutex_unlock(&(pool->lock));
	if(status != 0)
	{
		printf("Error:failed to unlock mutex!\n");
		err = threadpool_lock_failure;
	}

	return err;
}




/* *********************************************************************************
 * NAME:			threadpool_thread
 *
 * PURPOSE: the thread that run the task
 *
 * RETURN:  void
 * *********************************************************************************/
static void* threadpool_thread(void* threadpool)
{
	threadpool_t* pool = (threadpool_t*) threadpool;
	threadpool_task_t task;

	for( ; ;)
	{
		/* Lock must be taken when waiting on condition variable */
		pthread_mutex_lock(&(pool->lock));

		/* waiting on condition variable, check for spurious wakeuos */
		while((pool->count == 0) && (!pool->shutdown))
		{
			pthread_cond_wait(&(pool->notify),&(pool->lock));
		}

		/* if shutdowm immdiately, the thread pool will not longger get the task
		 * remains in the task queue
		 */
		if((pool->shutdown == immediate_shutdown) ||
				((pool->shutdown == graceful_shutdown) && (pool->count == 0)))
		{
			break;
		}

		/* Grab our task */
		task.function = pool->queue[pool->head].function;
		task.argument = pool->queue[pool->head].argument;
		pool->head += 1;
		pool->head = (pool->head == pool->queue_size)? 0 : pool->head;
		pool->count--;

		/* Unlock */
		pthread_mutex_unlock(&(pool->lock));

		/* Get to work */
		(*(task.function))(task.argument);
	}

	pool->started--;

    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return(NULL);
}



/* *********************************************************************************
 * NAME:			threadpool_destroy
 *
 * PURPOSE: Stops and destroys a thread pool
 *
 * RETURN:  0 if all goes well, negative values in case of error (@see
 *         threadpool_error_t for codes)
 * *********************************************************************************/
int threadpool_destroy(threadpool_t* pool)
{
	int i, err = 0;
	int status;

	if(pool == NULL)
	{
		printf("Error:invalid pool!\n");
		return threadpool_invalid;
	}

	status = pthread_mutex_lock(&(pool->lock));
	if(status != 0)
	{
		printf("Error:failed to lock the pool!\n");
		return threadpool_lock_failure;
	}

	do
	{
		/* check whether already shutdown */
		if(pool->shutdown)
		{
			printf("Error:Already shut down!\n");
			err = threadpool_shutdown;
			break;
		}

		pool->shutdown = (THREAD_SHUTDOWN_GRACEFUL)? graceful_shutdown : immediate_shutdown;

		/* Wake up all work thread */
		if((pthread_cond_broadcast(&(pool->notify)) != 0) ||
				(pthread_mutex_unlock(&(pool->lock)) != 0))
		{
			printf("Error:failed to wake up all work thread!\n");
			err = threadpool_lock_failure;
			break;
		}

		/* Join all work */
		for(i = 0; i < pool->thread_count; i++)
		{
			status = pthread_join(pool->threads[i],NULL);
			if(status != 0)
			{
				printf("Error:failed to join thread %d!\n",(int)&pool->threads[i]);
				err = threadpool_thread_failure;
			}
		}
	}while(0);

	/* Only if everything went well do we deallocate the pool */
	if(!err)
	{
		threadpool_free(pool);
	}

	return err;
}


/* *********************************************************************************
 * NAME:			threadpool_free
 *
 * PURPOSE: free a thread pool
 *
 * RETURN:  0 if all goes well, -1 in case of error
 * *********************************************************************************/
int threadpool_free(threadpool_t* pool)
{
	if(pool == NULL || pool->started > 0)
	{
		return -1;
	}

	if(pool->threads)
	{
		free(pool->threads);
		free(pool->queue);

		/* Because we allocate pool->threads after initializing the
		   mutex and condition variable, we're sure they're
		   initialized. Let's lock the mutex just in case. */
		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		pthread_cond_destroy(&(pool->notify));
	}
	free(pool);
	return 0;
}


