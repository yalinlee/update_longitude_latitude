/****************************************************************************
Name: ias_work_queue

Description: This module provides a thread-safe queue implementation that is
meant to be used with the threadpool.  It can be used to queue work for the
threadpool threads to perform.  It can also be used to serialize work that
needs to be done by one thread.  For example, it is used to shuffle database
operations from the processing threads to the main thread since some database
libraries (ODBC) are only guaranteed to support doing database operations from
the thread that makes the database connection.

****************************************************************************/
#include <stdlib.h>
#include <errno.h>
#include "ias_const.h"      /* SUCCESS/ERROR definition */
#include "ias_logging.h"
#include "ias_work_queue.h"


/****************************************************************************
* Name: ias_work_queue_initialize
*
* Description: Initializes a work queue.
*
* Returns:
*   SUCCESS or ERROR
****************************************************************************/
int ias_work_queue_initialize
(
    IAS_WORK_QUEUE *queue       /* I/O: work queue to intialize */
)
{
    int ret = ERROR;
    int status;

    /* verify the queue is valid */
    if (!queue)
    {
        IAS_LOG_ERROR("Invalid queue pointer parameter detected");
        return ERROR;
    }

    queue->head = NULL;
    queue->tail = NULL;

    /* create a semaphore for tracking how much data is in the queue */
    status = IAS_THREAD_CREATE_SEMAPHORE(&queue->data_available_sem, 0);
    if (status == 0)
    {
        /* create a mutex to serialize access to the queue */
        status = IAS_THREAD_CREATE_MUTEX(&queue->lock);
        if (status != 0)
        {
            IAS_THREAD_DESTROY_SEMAPHORE(&queue->data_available_sem);
        }
        else
            ret = SUCCESS;
    }
    return ret;
}

/****************************************************************************
* Name: ias_work_queue_destroy
*
* Description: Cleans up the resources allocated to a work queue.
*
* Returns:
*   SUCCESS or ERROR
****************************************************************************/
void ias_work_queue_destroy
(
    IAS_WORK_QUEUE *queue       /* I/O: work queue to destroy */
)
{
    /* verify the queue is valid */
    if (!queue)
    {
        IAS_LOG_ERROR("Invalid queue pointer parameter detected");
        return;
    }

    IAS_THREAD_DESTROY_SEMAPHORE(&queue->data_available_sem);
    IAS_THREAD_DESTROY_MUTEX(&queue->lock);
}

/****************************************************************************
* Name: ias_work_queue_add
*
* Description: Adds a work packet (function to call and message) to the queue.
*
* Returns:
*   SUCCESS or ERROR
****************************************************************************/
int ias_work_queue_add
(
    IAS_WORK_QUEUE *queue,      /* I/O: queue to add a message to */
    IAS_WORK_QUEUE_FUNC func,   /* I: function to call to do the work */
    void *message               /* I: pointer to message to add to the queue */
)
{
    IAS_WORK_QUEUE_NODE *node;
    int status;

    /* verify the queue is valid */
    if (!queue)
    {
        IAS_LOG_ERROR("Invalid queue pointer parameter detected");
        return ERROR;
    }

    /* allocate memory for the queue node */
    node = malloc(sizeof(*node));
    if (!node)
    {
        IAS_LOG_ERROR("Memory allocation failed for queue node");
        return ERROR;
    }

    /* initialize the queue node members */
    node->next = NULL;
    node->func = func;
    node->message = message;

    /* lock the work queue */
    status =  IAS_THREAD_LOCK_MUTEX(&queue->lock);
    if (status != 0)
    {
        /* locking failed */
        IAS_LOG_ERROR("Mutex lock call failed with status %d", status);
        free(node);
        return ERROR;
    }

    /* add the node to the queue */
    if (queue->tail == NULL)
    {
        /* queue empty, so set the node as the only one in the queue */
        queue->head = node;
        queue->tail = node;
        node->next = NULL;
    }
    else
    {
        /* queue contains data, so add this node to the end of the list */
        queue->tail->next = node;
        queue->tail = node;
    }

    /* unlock the queue */
    status =  IAS_THREAD_UNLOCK_MUTEX(&queue->lock);
    if (status != 0)
    {
        /* unable to unlock the mutex */
        IAS_LOG_ERROR("Mutex unlock call failed with status %d", status);
        return ERROR;
    }

    /* post to the semaphore to indicate data is available */
    status =  IAS_THREAD_POST_SEMAPHORE(&queue->data_available_sem);
    if (status != 0)
    {
        /* unable to increment the semaphore */
        IAS_LOG_ERROR("Semaphore post call failed with status %d", status);
        return ERROR;
    }

    return SUCCESS;
}

/****************************************************************************
* Name: ias_work_queue_remove
*
* Description: Removes the next work packet (function and message) from the
*   queue.  If the queue is empty, the routine waits for data to become
*   available.
*
* Returns:
*   SUCCESS or ERROR
****************************************************************************/
int ias_work_queue_remove
(
    IAS_WORK_QUEUE *queue,      /* I/O: queue to add a message to */
    IAS_WORK_QUEUE_FUNC *func,  /* I: function to call to do the work */
    void** message              /* I/O: pointer to message returned */
)
{
    IAS_WORK_QUEUE_NODE *node;
    int status;

    /* verify the queue is valid */
    if (!queue)
    {
        IAS_LOG_ERROR("Invalid queue pointer parameter detected");
        return ERROR;
    }

    /* wait for data to be available */
    while (((status = IAS_THREAD_WAIT_SEMAPHORE(&queue->data_available_sem))
            == -1) && (errno == EINTR))
    {
        /* restart if interrupted by a signal handler */
        continue;
    }

    if (status != 0)
    {
        /* error waiting for the semaphore */
        IAS_LOG_ERROR("Sempaphore wait call failed with status %d", status);
        return ERROR;
    }

    /* lock the work queue */
    status = IAS_THREAD_LOCK_MUTEX(&queue->lock);
    if (status != 0)
    {
        /* unable to lock the mutex */
        IAS_LOG_ERROR("Mutex lock call failed with status %d", status);
        return ERROR;
    }

    /* remove the node from the head of the queue */
    node = queue->head;
    queue->head = node->next;

    /* if the queue is empty, set the tail to NULL */
    if (queue->head == NULL)
        queue->tail = NULL;

    /* unlock the queue */
    status = IAS_THREAD_UNLOCK_MUTEX(&queue->lock);
    if (status != 0)
    {
        /* unable to unlock the mutex */
        IAS_LOG_ERROR("Mutex unlock call failed with status %d", status);
        return ERROR;
    }

    /* if no node was retrieved, something is wrong */
    if (node == NULL)
    {
        IAS_LOG_ERROR("Invalid node retrieve from queue");
        return ERROR;
    }

    /* set the return values */
    *func = node->func;
    *message = node->message;

    /* free the queue node */
    free(node);

    return SUCCESS;
}

/* Function to determine whether the work queue has data in it.  Returns 
   non-zero if the queue is empty. */
/****************************************************************************
* Name: ias_work_queue_is_empty
*
* Description: Determines whether a work queue has data in it.
*
* Returns:
*   non-zero if the queue is empty, zero if the queue is not empty
****************************************************************************/
int ias_work_queue_is_empty
(
    IAS_WORK_QUEUE *queue       /* I: queue to check */
)
{
    /* verify the queue is valid */
    if (!queue)
    {
        IAS_LOG_WARNING("Invalid queue pointer parameter detected");
        /* treat this as the queue being empty */
        return 1;
    }

    if (queue->head == NULL)
    {
        /* the queue is empty */
        return 1;
    }
    else
        return 0;
}
