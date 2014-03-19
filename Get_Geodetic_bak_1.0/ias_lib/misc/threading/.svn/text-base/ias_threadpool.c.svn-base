/******************************************************************************
* NAME: ias_threadpool.c
*
* PURPOSE: threadpool creates a pool of worker threads that can all be assigned
*   to a single task.  The pattern of use is:
*       Create the thread pool
*       Assign work to the pool (as many times as needed)
*       Destroy the thread pool
*   The thread pool should just be created once for the application execution.
*   To not use threading at all (for the purpose of debugging applications),
*   pass zero into the initialize routine of the thread pool.
* 
******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "ias_const.h"  /* SUCCESS/ERROR definitions */
#include "ias_logging.h"
#include "ias_threadpool.h"


struct thread_info
{
    struct ias_threadpool* pool;    /* pointer to thread pool */
    int thread_number;              /* number for this thread */
};

struct ias_threadpool
{
    int threadpool_size;        /* number of threads in the thread pool.  This
                                   is zero if no threading wanted (for 
                                   debugging applications) */
    int threads_to_start;       /* number of threads that need to start yet */
    int threads_are_running;    /* flag to indicate threads are running */
    int exit_threads;           /* flag that all the threads should exit */
    pthread_mutex_t threads_to_start_mutex;
            /* mutex to control access to threads_to_start and exit_threads */
    pthread_cond_t threads_to_start_cond;
            /* condition variable for threads_to_start and exit_threads */
    int threads_completed;
            /* number of threads that have completed their work */
    pthread_mutex_t threads_completed_mutex;
            /* mutex to control access to threads_completed */
    pthread_cond_t threads_completed_cond;
            /* condition variable for threads_completed */
    pthread_t *thread_ids;
            /* pointer to array that holds the ids of the threads created */
    struct thread_info *thread_info;
            /* pointer to array that holds the thread number for each thread
               and a pointer to the threadpool structure */
    void *worker_params;        /* pointer to worker thread parameters */
    int (*work_func)(void *, int);
            /* pointer to function to be called by the worker threads */
    int worker_return_status;
            /* return status from the worker threads */
    IAS_WORK_QUEUE *work_queue; /* pointer to an optional work queue */
};

/****************************************************************************
* Name: complete_thread
*
* Description: Helper function to mark a thread as completed.
*
* Returns:
*   SUCCESS or ERROR
****************************************************************************/
static int complete_thread
(
    struct ias_threadpool *pool     /* I: thread pool to stop threads in */
)
{
    int status;

    /* lock the mutex for the threads_completed variable */
    pthread_mutex_lock(&pool->threads_completed_mutex);

    /* ignore the status of the lock call since even if it fails, incrementing
       the completed count is better than returning without doing so */
    pool->threads_completed++;

    if (pool->threads_completed == pool->threadpool_size)
    {
        pool->threads_are_running = 0;
        status = pthread_cond_signal(&pool->threads_completed_cond);
        if (status != 0)
        {
            IAS_LOG_ERROR("Condition variable signal call failed with "
                          "status %d", status);
            pthread_mutex_unlock(&pool->threads_completed_mutex);
            return ERROR;
        }
    }

    status = pthread_mutex_unlock(&pool->threads_completed_mutex);
    if (status != 0)
    {
        IAS_LOG_ERROR("Mutex unlock call failed with status %d", status);
        return ERROR;
    }

    /* if there is a work queue, push a dummy message into the queue to make
       sure the run_function routine isn't stuck waiting for data from the
       queue. */
    if (pool->work_queue)
        ias_work_queue_add(pool->work_queue, NULL, NULL);

    return SUCCESS;
}

/****************************************************************************
* Name: worker
*
* Description: worker thread function
*
* Inputs:
*   thread id
****************************************************************************/
static void *worker(void *param)
{
    struct thread_info *info = (struct thread_info*)param;
    int thread_number = info->thread_number;
    struct ias_threadpool *pool = info->pool;
    int status;

    /* loop until all told to exit */
    while (!pool->exit_threads)
    {
        /* wait for work to be assigned */
        status = pthread_mutex_lock(&pool->threads_to_start_mutex);
        if (status != 0)
        {
            IAS_LOG_ERROR("Mutex lock call failed with status %d", status);
            complete_thread(pool);
            pool->worker_return_status = ERROR;
            return NULL;
        }

        while (!pool->threads_to_start && !pool->exit_threads)
        {
            status = pthread_cond_wait(&pool->threads_to_start_cond,
                                       &pool->threads_to_start_mutex);
            if (status != 0)
            {
                IAS_LOG_ERROR("Condition wait call failed with status %d",
                              status);
                pthread_mutex_unlock(&pool->threads_to_start_mutex);
                complete_thread(pool);
                pool->worker_return_status = ERROR;
                return NULL;
            }
        }
        pool->threads_to_start--;

        status = pthread_mutex_unlock(&pool->threads_to_start_mutex);
        if (status != 0)
        {
            IAS_LOG_ERROR("Mutex unlock call failed with status %d", status);
            complete_thread(pool);
            pool->worker_return_status = ERROR;
            return NULL;
        }

        if (pool->exit_threads)
            break;

        /* call the routine to do the work */
        status = pool->work_func(pool->worker_params, thread_number);
        if (status != SUCCESS)
            pool->worker_return_status = ERROR;

        /* indicate the thread is done with its work */
        if (complete_thread(pool) != SUCCESS)
        {
            IAS_LOG_ERROR("Error completing thread");
            pool->worker_return_status = ERROR;
            return NULL;
        }
    }

    return NULL;
}

/****************************************************************************
* Name: stop_threads
*
* Description: helper routine to stop any running threads
*
* Inputs:
*   number of threads to stop
****************************************************************************/
static void stop_threads
(
    struct ias_threadpool *pool,    /* I: thread pool to stop threads in */
    int num_threads                 /* I: number of threads to stop */
)
{
    int thread;

    /* tell the worker threads to exit */
    pthread_mutex_lock(&pool->threads_to_start_mutex);
    pool->exit_threads = 1;
    pthread_cond_broadcast(&pool->threads_to_start_cond);
    pthread_mutex_unlock(&pool->threads_to_start_mutex);

    /* wait for the threads to exit */
    for (thread = 0; thread < num_threads; thread++)
    {
        pthread_join(pool->thread_ids[thread], NULL);
    }
}

/****************************************************************************
* Name: ias_threadpool_initialize
*
* Description: Initializes the thread pool by:
*       - Initializing all the mutexes and condition variables
*       - Creating the threads
*
* Returns:
*   NULL if a failure occurs
*   pointer to the thread pool structure if it is initialized correctly
****************************************************************************/
struct ias_threadpool *ias_threadpool_initialize
(
    int size        /* I: number of threads to include in the pool.  Use zero
                          if no threading wanted (to debug applications) */
)
{
    int thread;
    struct ias_threadpool *pool;

    /* allocate memory for the threadpool structure */
    pool = malloc(sizeof *pool);
    if (!pool)
    {
        IAS_LOG_ERROR( "Allocating memory for threadpool");
        return NULL;
    }
    memset(pool, 0, sizeof(*pool));

    pool->threadpool_size = size;
    pool->work_queue = NULL;

    /* if the pool size is zero, no multithreading wanted, so just return */
    if (pool->threadpool_size == 0)
        return pool;

    pool->exit_threads = 0;

    /* flag that no threads have work to do when they start */
    pool->threads_to_start = 0;
    pool->threads_completed = 0;
    pool->threads_are_running = 0;

    /* allocate memory for the array to hold pthread ids */
    pool->thread_ids = malloc(pool->threadpool_size 
                              * sizeof(*pool->thread_ids));
    if (pool->thread_ids == NULL)
    {
        IAS_LOG_ERROR( "Allocating memory for thread ids");
        free(pool);
        return NULL;
    }

    if (pthread_mutex_init(&pool->threads_to_start_mutex, NULL) != 0)
    {
        free(pool->thread_ids);
        free(pool);
        IAS_LOG_ERROR( "Creating threads_to_start mutex");
        return NULL;
    }
    if (pthread_cond_init(&pool->threads_to_start_cond, NULL) != 0)
    {
        free(pool->thread_ids);
        pthread_mutex_destroy(&pool->threads_to_start_mutex);
        free(pool);
        IAS_LOG_ERROR( "Creating threads_to_start condition variable");
        return NULL;
    }

    if (pthread_mutex_init(&pool->threads_completed_mutex, NULL) != 0)
    {
        free(pool->thread_ids);
        pthread_mutex_destroy(&pool->threads_to_start_mutex);
        pthread_cond_destroy(&pool->threads_to_start_cond);
        free(pool);
        IAS_LOG_ERROR( "Creating threads_completed mutex");
        return NULL;
    }
    if (pthread_cond_init(&pool->threads_completed_cond, NULL) != 0)
    {
        free(pool->thread_ids);
        pthread_mutex_destroy(&pool->threads_to_start_mutex);
        pthread_cond_destroy(&pool->threads_to_start_cond);
        pthread_mutex_destroy(&pool->threads_completed_mutex);
        free(pool);
        IAS_LOG_ERROR( "Creating threads_completed condition variable");
        return NULL;
    }

    /* allocate memory for the array to hold thread numbers */
    pool->thread_info = malloc(pool->threadpool_size 
                                  * sizeof(*pool->thread_info));
    if (pool->thread_info == NULL)
    {
        free(pool->thread_ids);
        pthread_cond_destroy(&pool->threads_completed_cond);
        pthread_mutex_destroy(&pool->threads_to_start_mutex);
        pthread_cond_destroy(&pool->threads_to_start_cond);
        pthread_mutex_destroy(&pool->threads_completed_mutex);
        free(pool);
        IAS_LOG_ERROR( "Allocating memory for thread numbers");
        return NULL;
    }

    /* create the threads in the pool */
    for (thread = 0; thread < size; thread++)
    {
        pool->thread_info[thread].thread_number = thread;
        pool->thread_info[thread].pool = pool;

        if (pthread_create(&pool->thread_ids[thread], NULL, worker,
                           &pool->thread_info[thread]) != 0)
        {
            /* stop already started threads */
            if (thread > 0)
                stop_threads(pool, thread);

            /* free resources */
            free(pool->thread_ids);
            free(pool->thread_info);
            pthread_mutex_destroy(&pool->threads_to_start_mutex);
            pthread_cond_destroy(&pool->threads_to_start_cond);
            pthread_mutex_destroy(&pool->threads_completed_mutex);
            pthread_cond_destroy(&pool->threads_completed_cond);
            free(pool);
            IAS_LOG_ERROR( "Starting threads");
            return NULL;
        }
    }

    return pool;
}

/****************************************************************************
* Name: ias_threadpool_enable_work_queue
*
* Description: Enables the use of a work queue for the thread pool.  This is to
*   allow things like database work to be done from the main thread.
*
* Returns:
*   ERROR if a failure occurs
*   SUCCESS if the work queue is successfully enabled
****************************************************************************/
int ias_threadpool_enable_work_queue
(
    struct ias_threadpool *pool     /* I: thread pool to use a queue in */
)
{
    /* make sure pool is valid */
    if (!pool)
    {
        IAS_LOG_ERROR( "Invalid threadpool parameter detected");
        return ERROR;
    }

    /* can only enable a work queue once */
    if (pool->work_queue)
    {
        IAS_LOG_ERROR( "Attempting to enable a work queue twice");
        return ERROR;
    }

    /* allocate the queue memory */
    pool->work_queue = malloc(sizeof(*pool->work_queue));
    if (!pool->work_queue)
    {
        IAS_LOG_ERROR( "Allocating memory for threadpool work queue");
        return ERROR;
    }

    /* initialize the work queue */
    if (ias_work_queue_initialize(pool->work_queue) != SUCCESS)
    {
        free(pool->work_queue);
        pool->work_queue = NULL;
        return ERROR;
    }

    return SUCCESS;
}

/****************************************************************************
* Name: ias_threadpool_add_to_work_queue
*
* Description: Adds work to the work queue for a thread pool.
*
* Returns:
*   ERROR if a failure occurs
*   SUCCESS if the work queue is successfully enabled
****************************************************************************/
int ias_threadpool_add_to_work_queue
(
    struct ias_threadpool *pool,    /* I: thread pool to use a queue in */
    IAS_WORK_QUEUE_FUNC func,       /* I: function to do the work */
    void *message                   /* I: data for the function */
)
{
    int status = ERROR;

    /* make sure pool is valid */
    if (!pool)
    {
        IAS_LOG_ERROR( "Invalid threadpool parameter detected");
        return ERROR;
    }

    if (pool->threadpool_size == 0)
    {
        /* not threading, so call the routine directly */
        status = func(message);
    }
    else
    {
        /* if a queue is enabled, add the work to the queue */
        if (pool->work_queue)
            status = ias_work_queue_add(pool->work_queue, func, message);
    }

    return status;
}

/****************************************************************************
* Name: threadpool_start_function
*
* Description: Starts running a routine on all the threads in the thread pool.
*   This allows starting the threads without waiting for them to complete.
*
* Returns:
*   ERROR if a failure occurs starting the threads
*   SUCCESS if all the threads started successfully
*
* Notes:
*   - This routine can be called by the ias_threadpool_run_function routine
*     or external routines when applications need to be able to start the
*     threads in a threadpool without waiting for them to complete their work.
****************************************************************************/
int ias_threadpool_start_function
(
    struct ias_threadpool *pool,    /* I: thread pool to use */
    int func(void *, int),          /* I: function to start */
    void *params                    /* I: parameters to the routine */
)
{
    /* make sure pool is valid */
    if (!pool)
    {
        IAS_LOG_ERROR( "Invalid threadpool parameter detected");
        return ERROR;
    }

    /* make sure no threads are running */
    if (pool->threads_are_running)
    {
        IAS_LOG_ERROR("Tried to start threads when they are already running");
        return ERROR;
    }

    pool->work_func = func;
    pool->worker_params = params;

    /* no threads should be running, so skip locking the mutex to initialize
       these.  The worker_return_status is initialized to SUCCESS with the
       assumption that all worker threads will succeed.  If any of them
       encounter an error, they will set it to ERROR and can do so without
       any locking since there will be no race condition involved. */
    pool->threads_completed = 0;
    pool->worker_return_status = SUCCESS;
    pool->threads_are_running = 1;

    /* if the thread pool size is not zero (no threading), notify the 
       worker threads to start */
    if (pool->threadpool_size > 0)
    {
        int status;

        status = pthread_mutex_lock(&pool->threads_to_start_mutex);
        if (status != 0)
        {
            IAS_LOG_ERROR("Mutex lock call failed with status %d", status);
            return ERROR;
        }

        pool->threads_to_start = pool->threadpool_size;

        status = pthread_cond_broadcast(&pool->threads_to_start_cond);
        if (status != 0)
        {
            IAS_LOG_ERROR("Condition broadcast call failed with status %d",
                          status);
            pthread_mutex_unlock(&pool->threads_to_start_mutex);
            return ERROR;
        }

        status = pthread_mutex_unlock(&pool->threads_to_start_mutex);
        if (status != 0)
        {
            IAS_LOG_ERROR("Mutex unlock call failed with status %d", status);
            return ERROR;
        }
    }

    return SUCCESS;
}


/****************************************************************************
* Name: threadpool_wait_for_completion
*
* Description: Wait for all the threads in a thread pool to complete
*
* Returns:
*   ERROR if a failure occurs in any of the threads
*   SUCCESS if all the threads complete successfully
****************************************************************************/
int ias_threadpool_wait_for_completion
(
    struct ias_threadpool *pool     /* I: thread pool to wait for */
)
{
    int status;

    /* make sure pool is valid */
    if (!pool)
    {
        IAS_LOG_ERROR( "Invalid threadpool parameter detected");
        return ERROR;
    }

    /* wait for the threads to complete */
    status = pthread_mutex_lock(&pool->threads_completed_mutex);
    if (status != 0)
    {
        IAS_LOG_ERROR("Mutex lock call failed with status %d", status);
        return ERROR;
    }
    while (pool->threads_are_running
           && (pool->threads_completed != pool->threadpool_size))
    {
        status = pthread_cond_wait(&pool->threads_completed_cond,
                                   &pool->threads_completed_mutex);
        if (status != 0)
        {
            IAS_LOG_ERROR("Condition wait call failed with status %d", status);
            pthread_mutex_unlock(&pool->threads_completed_mutex);
            return ERROR;
        }
    }
    pool->threads_are_running = 0;

    status = pthread_mutex_unlock(&pool->threads_completed_mutex);
    if (status != 0)
    {
        IAS_LOG_ERROR("Mutex unlock call failed with status %d", status);
        return ERROR;
    }

    return pool->worker_return_status;
}

/****************************************************************************
* Name: threadpool_run_function
*
* Description: Runs a routine on all the threads in the thread pool.
*
* Returns:
*   ERROR if a failure occurs in any of the threads
*   SUCCESS if all the threads complete successfully
****************************************************************************/
int ias_threadpool_run_function
(
    struct ias_threadpool *pool,    /* I: thread pool to use */
    int func(void *, int),          /* I: function to run */
    void *params                    /* I: parameters to the routine */
)
{
    int status;

    /* make sure pool is valid */
    if (!pool)
    {
        IAS_LOG_ERROR( "Invalid threadpool parameter detected");
        return ERROR;
    }

    /* start the threads */
    status = ias_threadpool_start_function(pool, func, params);

    if (pool->threadpool_size == 0)
    {
        return pool->work_func(pool->worker_params, 1);
    }

    /* if a work queue is being used, wait for messages and process them */
    if (pool->work_queue)
    {
        int threads_done = 0;
        while (threads_done < pool->threadpool_size)
        {
            IAS_WORK_QUEUE_FUNC func;
            void *message;

            /* get the next item from the work queue */
            if (ias_work_queue_remove(pool->work_queue, &func, &message)
                != SUCCESS)
            {
                /* error getting info from the work queue */
                IAS_LOG_ERROR("In MULTIPROC getting data from the work queue");
                status = ERROR;
                break;
            }
            else if (message == NULL)
            {
                /* no message, so it must be an indication that a thread
                   exited */
                threads_done++;
            }
            else
            {
                /* call the function all the threads are supposed to run */
                if (func(message) != SUCCESS)
                {
                    status = ERROR;
                    pool->worker_return_status = ERROR;
                }
            }
        }
    }

    /* wait for the threads to complete */
    if (ias_threadpool_wait_for_completion(pool) != SUCCESS)
        return ERROR;

    return status;
}

/****************************************************************************
* Name: threadpool_destroy
*
* Description: Shuts down all the threads in the thread pool and frees the
*   resources allocated for it.
*
****************************************************************************/
void ias_threadpool_destroy
(
    struct ias_threadpool *pool     /* I: thread pool to destroy */
)
{
    /* make sure pool is valid */
    if (!pool)
    {
        IAS_LOG_WARNING( "Invalid threadpool parameter detected");
        return;
    }

    /* free up the resources if threading was enabled for the pool */
    if (pool->threadpool_size > 0)
    {
        /* stop the worker threads */
        stop_threads(pool, pool->threadpool_size);

        /* destroy the resources */
        pthread_mutex_destroy(&pool->threads_to_start_mutex);
        pthread_cond_destroy(&pool->threads_to_start_cond);
        pthread_mutex_destroy(&pool->threads_completed_mutex);
        pthread_cond_destroy(&pool->threads_completed_cond);
        free(pool->thread_ids);
        free(pool->thread_info);
    }
    if (pool->work_queue)
    {
        ias_work_queue_destroy(pool->work_queue);
        free(pool->work_queue);
    }
    free(pool);
}

/****************************************************************************
* Name: ias_threadpool_get_thread_count
*
* Description: Gets the number of thread running in a threadpool.
*
* Returns:
*   Number of threads in pool
*
****************************************************************************/
int ias_threadpool_get_thread_count
(
    const struct ias_threadpool *pool     /* I: thread pool to use */
)
{
    /* make sure pool is valid */
    if (!pool)
    {
        IAS_LOG_WARNING( "Invalid threadpool parameter detected");
        return 0;
    }

    return pool->threadpool_size;
}


/****************************************************************************
* Name: ias_thread_wait_on_condition
*
* Description: Waits for a condition variable to meet the proper condition.
*
* Returns:
*   SUCCESS or ERROR
*
* Notes:
*   - The caller is responsible for locking and unlocking the mutex.  The
*     reason is to prevent a race condition where the calling thread is
*     supposed to block but another thread has changed the condition variable
*     in the interim.
****************************************************************************/
int ias_thread_wait_on_condition
(
    IAS_THREAD_MUTEX_TYPE *mutex,   /* I: mutex controlling access to
                                          condition variable */
    IAS_THREAD_COND *condition,     /* I: condition variable to check */
    int (*check_function)(void *),  /* I: pointer to checking function */
    void *check_parameter           /* I: pointer to the parameter to
                                          check, if the check function has
                                          no need to differentiate among
                                          parameters, NULL may be passed */
)
{
    while (!check_function( check_parameter ))
    {
        int status;
        status = pthread_cond_wait(condition, mutex);
        if (status != 0)
        {
            IAS_LOG_ERROR("Condition variable wait call failed with status %d",
                          status);
            return ERROR;
        }
    }
    return SUCCESS;
}


/****************************************************************************
* Name: ias_thread_broadcast_condition
*
* Description: Broadcasts a condition to any threads waiting on the condition.
*
* Returns:
*   SUCCESS or ERROR
*
* Notes:
*   - The caller is responsible for locking and unlocking the mutex.  The
*     reason is to prevent a race condition where the calling thread is
*     supposed to block but another thread has changed the condition variable
*     in the interim.
****************************************************************************/
int ias_thread_broadcast_condition
(
    IAS_THREAD_MUTEX_TYPE *mutex,   /* I: mutex controlling access to
                                          condition variable */
    IAS_THREAD_COND *condition      /* I: condition variable */
)
{
    int status;

    status = pthread_cond_broadcast(condition);
    if (status != 0)
    {
        IAS_LOG_ERROR("Condition variable broadcast call failed with "
                      "status %d", status);
        pthread_mutex_unlock(mutex);
        return ERROR;
    }

    return SUCCESS;
}


/****************************************************************************
* Name: ias_threadpool_get_worker_return_status
*
* Description: Returns the worker_return_status member of the threadpool 
*              structure
*
* Returns:
*   SUCCESS or ERROR
****************************************************************************/
int ias_threadpool_get_worker_return_status
(
    const struct ias_threadpool *pool     /* I: thread pool to use */
)
{
    return pool->worker_return_status;
}
