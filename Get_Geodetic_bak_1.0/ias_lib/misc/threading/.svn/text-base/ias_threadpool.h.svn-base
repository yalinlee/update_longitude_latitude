#ifndef IAS_THREADPOOL_H
#define IAS_THREADPOOL_H

#include "ias_work_queue.h"

/* forward reference to the ias_threadpool structure.  Nothing external to the
   threadpool implementation should need access to it. */
struct ias_threadpool;

struct ias_threadpool *ias_threadpool_initialize
(
    int size        /* I: number of threads to include in the pool */
);

int ias_threadpool_enable_work_queue
(
    struct ias_threadpool *pool     /* I: thread pool to use a queue in */
);

int ias_threadpool_add_to_work_queue
(
    struct ias_threadpool *pool,    /* I: thread pool to use a queue in */
    IAS_WORK_QUEUE_FUNC func,       /* I: function to do the work */
    void *message                   /* I: data for the function */
);

int ias_threadpool_get_thread_count
(
    const struct ias_threadpool *pool   /* I: thread pool to use */
);

int ias_threadpool_start_function
(
    struct ias_threadpool *pool,    /* I: thread pool to use */
    int func(void *, int),          /* I: function to start */
    void *params                    /* I: parameters to the routine */
);

int ias_threadpool_wait_for_completion
(
    struct ias_threadpool *pool     /* I: thread pool to wait for */
);

int ias_threadpool_run_function
(
    struct ias_threadpool *pool,    /* I: thread pool to use */
    int func(void *, int),          /* I: function to run */
    void *params                    /* I: parameters to the routine */
);

void ias_threadpool_destroy
(
    struct ias_threadpool *pool     /* I: thread pool to destroy */
);

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
);

int ias_thread_broadcast_condition
(
    IAS_THREAD_MUTEX_TYPE *mutex,   /* I: mutex controlling access to
                                          condition variable */
    IAS_THREAD_COND *condition      /* I: condition variable */
);

int ias_threadpool_get_worker_return_status
(
    const struct ias_threadpool *pool     /* I: thread pool to use */
);

#endif
