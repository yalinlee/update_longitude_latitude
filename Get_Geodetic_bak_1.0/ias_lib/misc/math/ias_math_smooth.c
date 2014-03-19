/******************************************************************************
PURPOSE:
to smooth data after the kalman filter

ROUTINES:
ias_math_smooth_gain
ias_math_smooth_state

******************************************************************************/
#include <stdlib.h>
#include "ias_logging.h"
#include "ias_math.h"

/******************************************************************************
NAME: ias_math_smooth_gain

PURPOSE: Smooth the gain matrix

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion
ERROR    Operation failed

******************************************************************************/
int ias_math_smooth_gain
(
    const double *P,   /* I: Filtered covariance matrix */
    const double *Pn,  /* I: predicted error covariance matrix at time K+1 */
    const double *S,   /* I: state transition matrix */
    double *A,         /* O: smoothing gain matrix */
    int m              /* I: size in m direction */
)
{
    double *t1, *St, *inv;
    int status;
   
    t1  = malloc( m * m * sizeof(double) );
    if (t1 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }
    St  = malloc( m * m * sizeof(double) );
    if (St == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        return ERROR;
    }
    inv = malloc( m * m * sizeof(double) );
    if (inv == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        free(St);
        return ERROR;
    }

    ias_math_transpose_matrix( S, St, m, m );
    status = ias_math_invert_matrix( Pn, inv, m );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(St);
        free(inv);
        return ERROR;
    }

    status = ias_math_multiply_matrix( P, St, t1, m, m, m, m );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(St);
        free(inv);
        return ERROR;
    }

    status = ias_math_multiply_matrix( t1, inv, A, m, m, m, m );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(St);
        free(inv);
        return ERROR;
    }

    free(t1);
    free(St);
    free(inv);
    return SUCCESS;
}

/******************************************************************************
NAME: ias_math_smooth_state

PURPOSE: Smooth the state matrix

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion
ERROR    Operation failed

******************************************************************************/
int ias_math_smooth_state
(
    const double *X,   /* I: state matrix */
    const double *Xk,  /* I: state matrix at time K */
    const double *XN,  /* I: estimate of state [X] up to N */
    const double *A,   /* I: smoothing gain matrix */
    double *XN1,       /* O: predicted error covariance at time K+1*/
    int m              /* I: size in m direction */
)
{
    double *t1, *t2;
    int status;
   
    t1  = malloc( m * m * sizeof(double) );
    if (t1 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }
    t2  = malloc( m * m * sizeof(double) );
    if (t2 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        return ERROR;
    }
 
    ias_math_subtract_matrix( XN, Xk, t1, m, 1 );
    status = ias_math_multiply_matrix( A, t1, t2, m, m, m, 1 );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(t2);
        return ERROR;
    }

    ias_math_add_matrix( X, t2, XN1, m, 1 );
 
    free(t1);
    free(t2);
    return SUCCESS;
}
