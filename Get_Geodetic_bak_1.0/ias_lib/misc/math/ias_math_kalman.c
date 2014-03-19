/******************************************************************************
PURPOSE:
The kalman filter process

ROUTINES:
ias_math_predict_state
ias_math_compute_kalman_gain
ias_math_compute_predicted_error_covar
ias_math_update_filter_state
ias_math_update_filter_error_covar

NOTES:
  This file contains the matrix operations needed for Kalman 
  filtering.  For the notes that preceed each function the 
  following definitions are used.

  [X]  -> State matrix
  [S]  -> State transition matrix
  [P]  -> Error covariance matrix  
  [Q]  -> Process noise matrix
  [R]  -> Covariance matrix of state vector noise
  [H]  -> Matrix relating state matrix to measurement vector
  [K]  -> Kalman gain matrix
  [Z]  -> Measurement matrix
  [I]  -> Identity matrix
  [_]  -> Predicted 
  [ ]' -> Transpose of matrix
  [ ]^ -> Inverse of matrix

ALGORITHM REFERENCES:
Introduction to Random Signal Analysis and Kalman Filtering, 
by Robert Grover Brown (p. 195)

******************************************************************************/
#include <stdlib.h>
#include "ias_logging.h"
#include "ias_math.h"

/******************************************************************************
NAME: ias_math_predict_state

PURPOSE:
  Predict the state for k+1 given the state at k and the
  corresponding state transition matrix.  

   [X]k+1 = [S][X]k

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion
ERROR    Operation failed

******************************************************************************/
int ias_math_predict_state
(
    const double *S,   /* I: State transition matrix */
    const double *Xk,  /* I: State at K */
    double *Xk1,       /* O: State at K+1 */
    int m              /* I: size of matrix */
)
{
    int status;        /* Return status */


    status = ias_math_multiply_matrix( S, Xk, Xk1, m, m, m, 1 );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        return ERROR;
    }
    return SUCCESS;
}
/******************************************************************************
NAME: ias_math_compute_kalman_gain

PURPOSE:
  Calculate Kalman Gain matrix.
  
  [K] = [P_][H]'([H][P_][H]'+[R])^

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion
ERROR    Operation failed

******************************************************************************/
int ias_math_compute_kalman_gain
(
    const double *Pn, /* I: Predicted error covariance matrix */
    const double *H,  /* I: size of state matrix */
    const double *R,  /* I: Covariance of measured noise matrix */
    double *K,        /* O: Kalman gain matrix */
    int m,            /* I: size in m direction */
    int n             /* I: size in n direction */
)
{
    double *t1, *t2, *t3, *Ht, *inv;
    int status;

    t1  = malloc( m * n * sizeof(double) );
    if (t1 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }
    t2  = malloc( n * n * sizeof(double) );
    if (t2 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        return ERROR;
    }
    inv = malloc( n * n * sizeof(double) );
    if (inv == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        free(t2);
        return ERROR;
    }
    t3  = malloc( n * n * sizeof(double) );
    if (t3 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        free(t2);
        free(inv);
        return ERROR;
    }
    Ht  = malloc( m * n * sizeof(double) );
    if (Ht == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        free(t2);
        free(t3);
        free(inv);
        return ERROR;
    }

    ias_math_transpose_matrix( H, Ht, n, m );
    status = ias_math_multiply_matrix( Pn, Ht, t1, m, m, m, n );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(t2);
        free(t3);
        free(inv);
        free(Ht);
        return ERROR;
    }
    status = ias_math_multiply_matrix( H, t1, t2, n, m, m, n );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(t2);
        free(t3);
        free(inv);
        free(Ht);
        return ERROR;
    }
    ias_math_add_matrix( t2, R, t3, n, n );
    status = ias_math_invert_matrix( t3, inv, n );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_invert_matrix");
        free(t1);
        free(t2);
        free(t3);
        free(inv);
        free(Ht);
        return ERROR;
    }
    status = ias_math_multiply_matrix( t1, inv, K, m, n, n, n );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(t2);
        free(t3);
        free(inv);
        free(Ht);
        return ERROR;
    }

    free(t1);
    free(t2);
    free(t3);
    free(Ht);
    free(inv);
    return SUCCESS;
}
/******************************************************************************
NAME: ias_math_compute_predicted_error_covar

PURPOSE:
  Calculate the predicted state error covariance matrix.
  
  [P_] = [S][P][S]' + [Q]

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion
ERROR    Operation failed

******************************************************************************/
int ias_math_compute_predicted_error_covar
(
    const double *S,    /* I: State transition matrix */
    const double *Pn,   /* I: Filtered error covariance matrix */
    double *Pn1,        /* O: Predicted error covariance matrix at k+1 */
    const double *Q,    /* I: Process noise matrix */
    int m               /* I: size in m direction */
)
{
    double *t1, *t2, *St;
    int status; 

    t1 = malloc( m * m * sizeof(double) );
    if (t1 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }
    t2 = malloc( m * m * sizeof(double) );
    if (t2 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        return ERROR;
    }
    St = malloc( m * m * sizeof(double) );
    if (St == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        free(t2);
        return ERROR;
    }

    ias_math_transpose_matrix( S, St, m, m );
    status = ias_math_multiply_matrix( S, Pn, t1, m, m, m, m );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(t2);
        free(St);
        return ERROR;
    }
    status = ias_math_multiply_matrix( t1, St, t2, m, m, m, m );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(t2);
        free(St);
        return ERROR;
    }
    ias_math_add_matrix( t2, Q, Pn1, m, m );

    free(t1);
    free(t2);
    free(St);
    return SUCCESS;
}
/******************************************************************************
NAME: ias_math_update_filter_state

PURPOSE:
Update estimate of state given new measurement.

  [X] = [X_] + [K]([Z] - [H][X_])

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion
ERROR    Operation failed

******************************************************************************/
int ias_math_update_filter_state
(
    const double *Xk,  /* I: State matrix at time K */
    double *Xk1,       /* O: State matrix at time K+1 */
    const double *K,   /* I: Kalman gain matrix */
    const double *z,   /* I: measure ment of gain matrix */
    const double *H,   /* I: size of state matrix */
    int m,             /* I: size in m direction */
    int n              /* I: size in n direction */
)
{
    double *t1, *t2, *t3;
    int status;

    t1 = malloc( n * sizeof(double) );
    if (t1 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }
    t2 = malloc( n * sizeof(double) );
    if (t2 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        return ERROR;
    }
    t3 = malloc( m * sizeof(double) );
    if (t3 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        free(t2);
        return ERROR;
    }

    status = ias_math_multiply_matrix( H, Xk, t1, n, m, m, 1 );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(t2);
        free(t3);
        return ERROR;
    }
    ias_math_subtract_matrix( z, t1, t2, n, 1);
    status = ias_math_multiply_matrix( K, t2, t3, m, n, n, 1 );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(t2);
        free(t3);
        return ERROR;
    }
    ias_math_add_matrix( Xk, t3, Xk1, m, 1);

    free(t1);
    free(t2);
    free(t3);
    return SUCCESS;
}
/******************************************************************************
NAME: ias_math_update_filter_error_covar

PURPOSE:
  Update error covariance matrix given update estimate
  of state.

  [P] = ([I] - [K][H])[P_]

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion
ERROR    Operation failed

******************************************************************************/
int ias_math_update_filter_error_covar
(
    const double *K,  /* I: Kalman gain matrix */
    const double *H,  /* I: size of state matrix */
    const double *Pn, /* I: Filtered error covariance matrix */
    double *Pn1,      /* O: predicted error covariance matrix */
    int m,            /* I: size in m direction */
    int n             /* I: size in n direction */
)
{
    double *t1, *t2;
    int status;

    t1 = malloc( m * m * sizeof(double) );
    if (t1 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }
    t2 = malloc( m * m * sizeof(double) );
    if (t2 == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(t1);
        return ERROR;
    }


    status = ias_math_multiply_matrix( K, H, t1, m, n, n, m );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(t2);
        return ERROR;
    }
    status = ias_math_multiply_matrix( t1, Pn, t2, m, m, m, m );
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error returned from ias_math_multiply_matrix");
        free(t1);
        free(t2);
        return ERROR;
    }
    ias_math_subtract_matrix( Pn, t2, Pn1, m, m );

    free(t1);
    free(t2);
    return SUCCESS;
}
