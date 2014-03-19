/******************************************************************************
NAME:           ias_math_correlate_parallel

PURPOSE:        
The ias_math_correlate_parallel module implements a parallel correlation object.
Using the threadpool library, up to MAX_CORR_THREADS (or the number of 
processors available - whichever is less) are created to perform correlation.
The main thread that creates the parallel correlator is then responsible for
"feeding" the parallel correlator chips to correlate.

ROUTINES:
    ias_math_init_parallel_correlator
    ias_math_get_corr_chip_buffers
    ias_math_submit_chip_to_corr
    ias_math_close_parallel_correlator
    ias_math_correlate_thread
    ias_math_parallel_correlator_wait_for_results

NOTES:
- To use a parallel correlator, the application must first initialize it
  using the ias_math_init_parallel_correlator function.  Each of the other
  routines require the pointer to the structure returned by the init routine
  as a parameter.
- The application must load the chips to correlate into the buffers returned by 
  ias_math_get_corr_chip_buffers.  There are a limited number of chip buffers 
  so if there are none available, the main application thread is put to sleep 
  until a set of chip buffers is made available by the correlating threads.
- The ias_math_submit_chip_to_corr places the chips into a queue. The 
  correlation threads remove the chips from the queue and perform the 
  correlation.  The results of the correlation are not immediately available to
  the application since ias_math_submit_chip_to_corr returns before the 
  correlation is complete.
- Before any of the correlation results are used, the application must call
  ias_math_parallel_correlator_wait_for_results to make sure all the chips have
  been correlated.
- Normally, this module uses multiple threads.  The MAX_CORR_THREADS can be
  modified to zero to not use multiple threads (generally just for debugging).
- When small chips are being correlated it is very likely that the job will
  become I/O bound instead of CPU bound.
- It is assumed that a single thread will be submitting chips to correlate
  (i.e.  calling ias_math_submit_chip_to_corr).  If multiple threads are going
  to be doing this, a design change is needed to make sure the threadpool
  threads are only started once.

ALGORITHM REFERENCES:
None

******************************************************************************/
#include <unistd.h>   /* sysconf prototype */
#include <stdlib.h>   /* malloc/free prototypes */
#include "ias_const.h" /* SUCCESS/ERROR definitions */
#include "ias_logging.h" /* ias_logging prototype */
#include "ias_work_queue.h"
#include "ias_threadpool.h"
#include "ias_math_parallel_corr.h" /* prototypes for this module */
#include "ias_math.h" /* prototype */

#define MAX_CORR_THREADS 6
/* defines the maximum number of correlation threads to allow. Set to zero
   to single-thread the correlation. */


typedef struct
{
    int chip_index;          /* index to store results at in results array */
    float *search_image_ptr; /* search image */
    float *ref_image_ptr;    /* reference image */
    int search_size[2];      /* size of search image (samp,line) */
    int ref_size[2];         /* size of reference image (samp,line) */
    double min_corr;         /* minimum acceptable correlation strength */
    IAS_CORRELATION_FIT_TYPE fit_method;/* fit method to use */
    double max_disp;         /* maximum allowed diagnol displacement */
    double nominal_offset[2];/* nominal offset of UL corner of ref to search
                                (samp,line) */
    int abs_corr_coeff_flag; /* flag to use the absolute of the correlation
                                coefficients */
} CORRELATE_DATA_TYPE;

struct ias_parallel_correlator
{
    /* flags to check for each correlation */
    int check_edge_flag;
    int check_mult_peak_flag;
    int check_low_peak_flag;
    int check_max_disp_flag;
 
    /* maximum reference and search chip dimensions */
    int max_ref_chip_lines;
    int max_ref_chip_samples;
    int max_search_chip_lines;
    int max_search_chip_samples;

    /* free chip buffers tracking information */
    float *chip_buffer;               /* pointer to the chip buffer memory */
    int free_chip_buffer_queue_entries; /* number of entries allocated for the 
                                           free chip buffer */
    int ref_chip_size;     /* number of float values in the reference chips */
    int search_chip_size;  /* number of float values in the search chips */
    IAS_WORK_QUEUE free_chip_buffer_queue; /* queue for free chip buffers */

    /* queue for tracking work submitted to the correlator */
    IAS_WORK_QUEUE correlate_queue;

    struct ias_threadpool *threadpool; /* threadpool for the correlator */
    int threads_running;
    int threads;   /* number of correlation threads created */
    int error_flag; /* flag to indicate an error was encountered in a 
                       correlation thread.  All other correlation threads
                       should terminate as should the 
                       ias_math_get_corr_chip_buffers routine. */

    IAS_CORRELATION_RESULT_TYPE *results_ptr; /* pointer to the correlation
                                                 results storage */
    
};
/* structure that defines a parallel correlator */

/* LOCAL PROTOTYPES */

static int ias_math_correlate_thread(void *params_ptr, int thread_number);
/* prototype for the correlation thread function */


/******************************************************************************
FUNCTION NAME: initialize_free_chip_buffer_queue

PURPOSE:        
Initializes the queue for tracking free chip buffers and allocates memory for
the buffers.

RETURNS:
SUCCESS or ERROR

******************************************************************************/
static int initialize_free_chip_buffer_queue
(
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr
)
{
    int counter;
    int buffer_entries;
    int individual_buffer_size;

    /* allocate memory for three times as many chip buffers as there are
       threads to allow the main thread to keep some queued up */
    buffer_entries = 3 * (correlator_ptr->threads + 1);
    correlator_ptr->free_chip_buffer_queue_entries = buffer_entries;
    individual_buffer_size = correlator_ptr->ref_chip_size 
                           + correlator_ptr->search_chip_size;

    /* allocate memory for the free chip buffers */
    correlator_ptr->chip_buffer = (float *)malloc(buffer_entries *
               individual_buffer_size * sizeof(float));
    if (correlator_ptr->chip_buffer == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory for the chip buffers");
        return ERROR;
    }

    if (ias_work_queue_initialize(&correlator_ptr->free_chip_buffer_queue)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Error initializing free chip buffer queue");
        free (correlator_ptr->chip_buffer);
        return ERROR;
    }

    /* split the hunk of memory allocated for chip buffers into individual
       buffers and adds them to the buffer queue */
    for (counter = 0; counter < buffer_entries; counter++)
    {
        float *chip_buffer 
            = &correlator_ptr->chip_buffer[counter * individual_buffer_size];

        /* add the buffer to the free buffer queue */
        if (ias_work_queue_add(&correlator_ptr->free_chip_buffer_queue, NULL,
                               chip_buffer) != SUCCESS)
        {
            ias_work_queue_destroy(&correlator_ptr->free_chip_buffer_queue);
            free(correlator_ptr->chip_buffer);
            return ERROR;
        }
    }

    return SUCCESS;
}

/******************************************************************************
FUNCTION NAME: destroy_free_chip_buffer_queue

PURPOSE:        
Frees the resources allocated to the free chip buffer queue.

RETURNS:
nothing

******************************************************************************/
static void destroy_free_chip_buffer_queue
(
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr
)
{
    /* drain the entries from the free chip buffer queue so it can release
       the memory allocated there */
    while (!ias_work_queue_is_empty(&correlator_ptr->free_chip_buffer_queue))
    {
        IAS_WORK_QUEUE_FUNC func;
        void *message;
        if (ias_work_queue_remove(&correlator_ptr->free_chip_buffer_queue,
                    &func, &message) != SUCCESS)
        {
            /* if an error happens removing data from the queue, just log an
               error and bail out of the loop.  It will leak memory, but not
               much */
            IAS_LOG_ERROR("Error freeing the memory allocated to the free "
                          "chip buffer queue");
            break;
        }
    }
    ias_work_queue_destroy(&correlator_ptr->free_chip_buffer_queue);
    free(correlator_ptr->chip_buffer);
}

/******************************************************************************
FUNCTION NAME:        ias_math_init_parallel_correlator

PURPOSE:        
ias_math_init_parallel_correlator initializes an instance of the parallel 
correlator. All the multiprocessing resources are created and the memory for 
the chip buffers and queue structures is allocated.

RETURNS:
A pointer to the parallel correlator state structure is returned.  If an error
occurs, an error message is generated and NULL is returned.

NOTES:
- A successfully created parallel correlator must be destroyed using the 
  ias_math_close_parallel_correlator routine.
- The calling application must allocate and free the memory for the 
  result_ptr.

******************************************************************************/

IAS_PARALLEL_CORRELATOR_TYPE *ias_math_init_parallel_correlator
(
    int check_edge_flag,  /* I: set to non-zero to check the corr edge flag */
    int check_mult_peak_flag, /* I: set to non-zero to check multi peak flag*/
    int check_low_peak_flag, /* I: set to non-zero to check low peak flag */
    int check_max_disp_flag, /* I: set to non-zero to check max disp flag */
    int max_ref_chip_lines,  /* I: max lines in reference chip */
    int max_ref_chip_samples, /* I: max samples across reference chip */
    int max_search_chip_lines, /* I: max lines in search chip */
    int max_search_chip_samples, /* I: max samples across search chip */
    IAS_CORRELATION_RESULT_TYPE *results_ptr /* I: pointer to results array */
)
{
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr; /* pointer to the parallel 
                                    correlator data structure */
    long processors; /* number of processors on the machine */
    int ref_chip_size;   /* size of a reference chip (lines * samples) */
    int search_chip_size; /* size of a search chip (lines * samples) */
    
    /* allocate memory for the parallel correlator structure */
    correlator_ptr = malloc(sizeof(IAS_PARALLEL_CORRELATOR_TYPE));
    if (correlator_ptr == NULL)
    {
        /* couldn't allocate memory for the parallel correlator */
        IAS_LOG_ERROR("Error allocating memory for the parallel correlator");
        return NULL;
    }

    /* save the flags that need to be checked for each correlation point */
    correlator_ptr->check_edge_flag = check_edge_flag;
    correlator_ptr->check_mult_peak_flag = check_mult_peak_flag;
    correlator_ptr->check_low_peak_flag = check_low_peak_flag;
    correlator_ptr->check_max_disp_flag = check_max_disp_flag;

    /* save the pointer to the results array */
    correlator_ptr->results_ptr = results_ptr;
    
    /* save the maximum sizes of the search and reference chips */
    correlator_ptr->max_ref_chip_lines = max_ref_chip_lines;
    correlator_ptr->max_ref_chip_samples = max_ref_chip_samples;
    correlator_ptr->max_search_chip_lines = max_search_chip_lines;
    correlator_ptr->max_search_chip_samples = max_search_chip_samples;

    /* calculate the size of the reference and search chips */
    ref_chip_size = max_ref_chip_lines * max_ref_chip_samples;
    search_chip_size = max_search_chip_lines * max_search_chip_samples;
    
    correlator_ptr->ref_chip_size = ref_chip_size;
    correlator_ptr->search_chip_size = search_chip_size;

    /* clear the error flag */
    correlator_ptr->error_flag = 0;

    /* initialize the threads running field */
    correlator_ptr->threads_running = 0;

    /* determine the number of processors available on the machine */
    processors = sysconf(_SC_NPROCESSORS_ONLN);
   
    /* limit the number of threads to MAX_CORR_THREADS, or the number of
       processors on the box */
    if (processors > MAX_CORR_THREADS)
    {
        processors = MAX_CORR_THREADS;
    }
    else if (processors < 0)
    {
        processors = 1;
    }
    correlator_ptr->threads = (int)processors;

    /* initialize the free chip buffer queue */
    if (initialize_free_chip_buffer_queue(correlator_ptr) != SUCCESS)
    {
        IAS_LOG_ERROR("Error allocating memory for the free chip buffers");
        free (correlator_ptr);
        return NULL;
    }
    
    /* initialize the correlate queue */
    if (ias_work_queue_initialize(&correlator_ptr->correlate_queue)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Error initializing correlate queue");
        free (correlator_ptr->chip_buffer);
        return NULL;
    }

    /* create the threadpool for the correlator */
    correlator_ptr->threadpool 
        = ias_threadpool_initialize(correlator_ptr->threads);
    if (!correlator_ptr->threadpool)
    {
        IAS_LOG_ERROR("Error creating correlation threadpool");
        ias_work_queue_destroy(&correlator_ptr->correlate_queue);
        destroy_free_chip_buffer_queue(correlator_ptr);
        free (correlator_ptr);
        return NULL;   
    }

    /* return the correlator pointer */
    return correlator_ptr;
}

/******************************************************************************
FUNCTION NAME:        ias_math_get_corr_chip_buffers

PURPOSE:        
ias_math_get_corr_chip_buffers returns buffers for the search and reference 
chips that will be submitted to the parallel correlator.

RETURNS:
Pointers to the reference chip buffer and search chip buffer are returned. 
A SUCCESS/ERROR flag is also returned.

NOTES:
- The buffers returned are meant to be submitted to the parallel correlator.
  Getting buffers from this routine and not submitting them to the parallel
  correlator will quickly exhaust all the buffers available.
- The buffers will be free'd when the parallel correlator is closed.

ALGORITHM REFERENCES:
none

******************************************************************************/

int ias_math_get_corr_chip_buffers
(
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr, /* I: parallel correlator */
    float **ref_buffer_ptr,   /* O: pointer to reference chip buffer */
    float **search_buffer_ptr /* O: pointer to search chip buffer */
)
{
    IAS_WORK_QUEUE_FUNC func;
    void *message;
    float *buffer;

    /* get a buffer from the free chip buffer queue */
    if (ias_work_queue_remove(&correlator_ptr->free_chip_buffer_queue, &func,
            &message) != SUCCESS)
    {
        IAS_LOG_ERROR("Error retrieving buffer from the free chip buffer "
                      "queue");
        return ERROR;
    }
    buffer = message;

    /* if an error has occurred in any of the correlation threads, abort this
       routine since the application should be shutting down. */
    if (correlator_ptr->error_flag)
    {
        IAS_LOG_ERROR("Error reported by a correlation thread");
        return ERROR;
    }

    /* get the two buffers from the one pointer */
    *ref_buffer_ptr = buffer;
    *search_buffer_ptr = &buffer[correlator_ptr->ref_chip_size];

    return SUCCESS;
}

/******************************************************************************
FUNCTION NAME: correlate

PURPOSE: helper function to call the correlator

RETURNS:
A SUCCESS/ERROR flag is returned.

******************************************************************************/
static int correlate
(
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr, /* I: parallel correlator */
    CORRELATE_DATA_TYPE *corr_data, /* I: correlator input */
    IAS_CORRELATION_RESULT_TYPE *result_ptr /* I: pointer to results */
)
{
    int edge_flag;     /* edge error flag from the correlation routine */
    int mult_peak_flag;/* multiple peak error flag from the correlate routine*/
    int low_peak_flag; /* low peak error flag from the correlation routine */
    int max_disp_flag; /* max displacement exceeded error flag from corr */

    /* perform the correlation */
    if (ias_math_correlate_grey(corr_data->search_image_ptr, 
        corr_data->ref_image_ptr,
        corr_data->search_size, corr_data->ref_size, corr_data->min_corr,
        corr_data->fit_method, corr_data->max_disp,
        corr_data->nominal_offset, &result_ptr->strength,
        result_ptr->fit_offset, result_ptr->est_err, &result_ptr->diag_disp,
        &mult_peak_flag, &edge_flag, &low_peak_flag, 
        &max_disp_flag, corr_data->abs_corr_coeff_flag) == ERROR)
    {   
        /* error correlating the point */
        IAS_LOG_ERROR("Error correlating a data point");
        return ERROR;
    }

    /* Check the return flags to see if this correlation succeeded */
    if (((edge_flag==ERROR) && correlator_ptr->check_edge_flag) ||
        ((mult_peak_flag==ERROR) && correlator_ptr->check_mult_peak_flag) || 
        ((low_peak_flag==ERROR) && correlator_ptr->check_low_peak_flag) || 
        ((max_disp_flag==ERROR) && correlator_ptr->check_max_disp_flag))
    {
        /* correlation failure */
        result_ptr->valid = 0;
        result_ptr->fit_offset[0] = 0.0;
        result_ptr->fit_offset[1] = 0.0;
    }
    else
    {
        /* correlation success */
        result_ptr->valid = 1;
    }
    return SUCCESS;
}

/******************************************************************************
FUNCTION NAME:        ias_math_submit_chip_to_corr

PURPOSE:        
ias_math_submit_chip_to_corr places chips to correlate into the correlation 
queue. The chips will be removed from the queue by the correlation threads and 
correlated in the order they are put in.

RETURNS:
A SUCCESS/ERROR flag is returned.

NOTES:
- Either adds the chips to correlate to the correlate queue, or directly
  calls correlate if not multithreaded.

ALGORITHM REFERENCES:
none

******************************************************************************/

int ias_math_submit_chip_to_corr
(
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr, /* I: parallel correlator */
    int chip_index,          /* I: index to store results at in results array */
    float *search_image_ptr, /* I: search image */
    float *ref_image_ptr,    /* I: reference image */
    int search_size[2],      /* I: size of search image (samp,line) */
    int ref_size[2],         /* I: size of reference image (samp,line) */
    float min_corr,          /* I: minimum acceptable correlation strength */
    IAS_CORRELATION_FIT_TYPE fit_method,/* I: fit method to use */
    float max_disp,          /* I: maximum allowed diagonal displacement */
    float nominal_offset[2], /* I: nominal offset of UL corner of ref to search
                                   (samp,line) */
    int abs_corr_coeff_flag  /* I: flag to use the absolute of the correlation
                                   coefficients */    
)
{
    CORRELATE_DATA_TYPE *corr_data_ptr; /* pointer to current queue location */

    /* make sure the correlator threads are running - note assumes this will
       only be called from a single thread, otherwise some locking would be
       needed here. */
    if (!correlator_ptr->threads_running)
    {
        if (correlator_ptr->threads > 0)
        {
            if (ias_threadpool_start_function(correlator_ptr->threadpool, 
                    ias_math_correlate_thread, correlator_ptr) != SUCCESS)
            {
                IAS_LOG_ERROR("Error starting correlation threads");
                return ERROR;
            }
        }
        correlator_ptr->threads_running = 1;
    }

    /* allocate a buffer for the correlation work */
    corr_data_ptr = malloc(sizeof(*corr_data_ptr));
    if (!corr_data_ptr)
    {
        IAS_LOG_ERROR("Error allocating memory for correlation work");
        return ERROR;
    }

    /* insert the data into the queue */
    corr_data_ptr->chip_index = chip_index;
    corr_data_ptr->search_image_ptr = search_image_ptr;
    corr_data_ptr->ref_image_ptr = ref_image_ptr;
    corr_data_ptr->search_size[0] = search_size[0];
    corr_data_ptr->search_size[1] = search_size[1];
    corr_data_ptr->ref_size[0] = ref_size[0];
    corr_data_ptr->ref_size[1] = ref_size[1];
    corr_data_ptr->min_corr = min_corr;
    corr_data_ptr->fit_method = fit_method;
    corr_data_ptr->max_disp = max_disp;
    corr_data_ptr->nominal_offset[0] = nominal_offset[0];
    corr_data_ptr->nominal_offset[1] = nominal_offset[1];
    corr_data_ptr->abs_corr_coeff_flag = abs_corr_coeff_flag;

    if (correlator_ptr->threads > 0)
    {
        /* add to the queue in the multithreaded case */
        if (ias_work_queue_add(&correlator_ptr->correlate_queue, NULL,
                corr_data_ptr) != SUCCESS)
        {
            IAS_LOG_ERROR("Error adding work to the correlation queue");
            free(corr_data_ptr);
            return ERROR;
        }
    }
    else
    {
        /* directly correlate in the single threaded case */
        IAS_CORRELATION_RESULT_TYPE *result_ptr
                = &correlator_ptr->results_ptr[chip_index];
        
        /* perform the correlation */
        if (correlate(correlator_ptr, corr_data_ptr, result_ptr) != SUCCESS)
        {   
            /* error correlating the point */
            IAS_LOG_ERROR("Error correlating a data point");
            return ERROR;
        }

        /* return the chip buffers to the free buffer queue */
        if (ias_work_queue_add(&correlator_ptr->free_chip_buffer_queue, NULL,
                               corr_data_ptr->ref_image_ptr) != SUCCESS)
        {
            /* error returning the buffer to the queue */
            IAS_LOG_ERROR("Error returning the free chip buffer to the queue");
            return ERROR;
        } 

        /* free the corr_data_ptr since it isn't needed anymore here */
        free(corr_data_ptr);
    }

    return SUCCESS;
}

/******************************************************************************
FUNCTION NAME:        ias_math_correlate_thread

PURPOSE:        
ias_math_correlate_thread is the routine that removes chips from the correlation
queue and calls the correlation routine when compiled for multithreading mode.
The chip buffers are returned to the buffer queue when the correlation is 
completed on each set of chips.
There can be many copies of this routine running at the same time.

RETURNS:
A SUCCESS/ERROR flag is returned, cast to an int since pthread routines must
return a void pointer.

NOTES:
- This routine is not included when compiled for single threading.


ALGORITHM REFERENCES:
none

******************************************************************************/
static int ias_math_correlate_thread
(
    void *params_ptr,
    int thread_number
)
{
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr=
                        (IAS_PARALLEL_CORRELATOR_TYPE*)params_ptr;
    
    IAS_CORRELATION_RESULT_TYPE *result_ptr;  /* pointer to current results */
   
    /* loop through the thread's code until cancelled */
    while(1)
    {
        IAS_WORK_QUEUE_FUNC func;
        void *message;
        CORRELATE_DATA_TYPE *corr_data;  /* current correlation parameters */

        /* wait for a point to be ready to correlate */
        if (ias_work_queue_remove(&correlator_ptr->correlate_queue, &func,
                &message) != SUCCESS)
        { 
            IAS_LOG_ERROR("Error getting correlation work");
            /* set the error flag and post the buffer queue semaphore to 
               release the main thread if it is waiting for buffers */
            correlator_ptr->error_flag = 1;
            ias_work_queue_add(&correlator_ptr->free_chip_buffer_queue, NULL, 
                               NULL);
            return ERROR;
        }
        corr_data = (CORRELATE_DATA_TYPE *)message;

        /* if an error occurred in one of the other threads, exit */
        if (correlator_ptr->error_flag)
            break;
  
        /* if the message is NULL, that is the signal to quit */
        if (!corr_data)
            break;

        /* cache the pointer to the results */
        result_ptr = &correlator_ptr->results_ptr[corr_data->chip_index];

        /* perform the correlation */
        if (correlate(correlator_ptr, corr_data, result_ptr) != SUCCESS)
        {
            /* set the error flag and post the buffer queue semaphore to 
               release the main thread if it is waiting for buffers */
            correlator_ptr->error_flag = 1;
            ias_work_queue_add(&correlator_ptr->free_chip_buffer_queue, NULL, 
                               NULL);
            return ERROR;
        }

        /* return the chip buffers to the free buffer queue */
        if (ias_work_queue_add(&correlator_ptr->free_chip_buffer_queue, NULL,
                               corr_data->ref_image_ptr) != SUCCESS)
        {
            /* error returning the buffer to the queue */
            IAS_LOG_ERROR("Error returning the free chip buffer to the queue");
            /* set the error flag and post the buffer queue semaphore to 
               release the main thread if it is waiting for buffers */
            correlator_ptr->error_flag = 1;
            ias_work_queue_add(&correlator_ptr->free_chip_buffer_queue, NULL, 
                               NULL);
            return ERROR;
        } 

        /* free the corr_data pointer since they are allocated when added to
           the queue */
        free(corr_data);
    }

    return SUCCESS;
}

/******************************************************************************
FUNCTION NAME:        ias_math_close_parallel_correlator

PURPOSE:        
ias_math_close_parallel_correlator is the routine that needs to be called after
all the chips have been submitted to the correlator.  This routine will wait 
until all the threads have completed, then destroy this instance of the 
parallel correlator.

RETURNS:
A SUCCESS/ERROR flag is returned.

NOTES:
- The threads in the threadpool are not exited until this is called.

ALGORITHM REFERENCES:
none

******************************************************************************/

int ias_math_close_parallel_correlator
(
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr /* I: parallel correlator */
)
{
    if (correlator_ptr->threads > 0)
    {
        /* if the threads are in the correlation routine, tell them to exit */
        if (correlator_ptr->threads_running)
        {
            if (ias_math_parallel_correlator_wait_for_results(correlator_ptr)
                != SUCCESS)
            {
                IAS_LOG_ERROR("Error exiting correlator threads");
                return ERROR;
            }
        }
    }

    /* destroy the threadpool */
    ias_threadpool_destroy(correlator_ptr->threadpool);

    /* free the resources allocated */
    ias_work_queue_destroy(&correlator_ptr->correlate_queue);
    destroy_free_chip_buffer_queue(correlator_ptr);
    free (correlator_ptr);    
    
    return SUCCESS;
}


/******************************************************************************
FUNCTION NAME: ias_math_parallel_correlator_wait_for_results

PURPOSE:        
ias_math_parallel_correlator_wait_for_results is the routine that needs to 
be called to wait for results and prevent a race condition.

RETURNS:
A SUCCESS/ERROR flag is returned.

NOTES:
- The results of the correlation are not valid until this routine returns.

ALGORITHM REFERENCES:
none

******************************************************************************/
int ias_math_parallel_correlator_wait_for_results
(
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr /* I: parallel correlator */
)
{
    int status;
    int counter;

    /* If no threads are running, there is nothing to do.  This check was
       added to cover the case where the parallel correlator is created
       and no chips submitted before this routine is called.  Without this
       check, the shutdown messages would get put in the work queue and
       when work was finally submitted the threads that were started would
       immediately get the shutdown message instead of the chips to
       correlate. */
    if (!correlator_ptr->threads_running)
        return SUCCESS;

    /* no need to wait if not multithreaded */
    if (correlator_ptr->threads == 0)
        return SUCCESS;

    /* insert a dummy message once for each thread created so the threads
       stop running the correlator function when complete */
    for (counter = 0; counter < correlator_ptr->threads; counter++)
    {
        if (ias_work_queue_add(&correlator_ptr->correlate_queue, NULL, NULL)
            != SUCCESS)
        {
            /* error posting to the semaphore */
            IAS_LOG_ERROR("Error inserting exit messages to correlate queue");
            return ERROR;
        }
    }

    /* wait for the threads to complete */
    status = ias_threadpool_wait_for_completion(correlator_ptr->threadpool);
    correlator_ptr->threads_running = 0;
    return status;
}
