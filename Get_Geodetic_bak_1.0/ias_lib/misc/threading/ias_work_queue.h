#ifndef IAS_WORK_QUEUE_H
#define IAS_WORK_QUEUE_H

#include "ias_threadsync.h"

typedef int (*IAS_WORK_QUEUE_FUNC)(void *); /* function to do work in work
                                               queue */

typedef struct ias_work_queue_node
{
    struct ias_work_queue_node *next;/* next node in singly linked list */
    IAS_WORK_QUEUE_FUNC func;        /* function to call for work */
    void *message;                   /* pointer to structure with parameters
                                        for func */
} IAS_WORK_QUEUE_NODE;
/* defines the data to store in a node of the work queue */

typedef struct work_queue
{
    IAS_WORK_QUEUE_NODE *head;      /* head of the singly linked list */
    IAS_WORK_QUEUE_NODE *tail;      /* tail of the singly linked list */
    IAS_THREAD_SEMAPHORE_TYPE data_available_sem; 
                                    /* semaphore to track data in the list */
    IAS_THREAD_MUTEX_TYPE lock;     /* mutex to serialize access to the queue */
} IAS_WORK_QUEUE;

int ias_work_queue_initialize
(
    IAS_WORK_QUEUE *queue       /* I/O: work queue to intialize */
);

void ias_work_queue_destroy
(
    IAS_WORK_QUEUE *queue       /* I/O: work queue to destroy */
);

int ias_work_queue_add
(
    IAS_WORK_QUEUE *queue,      /* I/O: queue to add a message to */
    IAS_WORK_QUEUE_FUNC func,   /* I: function to call to do the work */
    void *message               /* I: pointer to message to add to the queue */
);

int ias_work_queue_remove
(
    IAS_WORK_QUEUE *queue,      /* I/O: queue to add a message to */
    IAS_WORK_QUEUE_FUNC *func,  /* I: function to call to do the work */
    void** message              /* I/O: pointer to message returned */
);

int ias_work_queue_is_empty
(
    IAS_WORK_QUEUE *queue       /* I: queue to check */
);

#endif
