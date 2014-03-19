/******************************************************************************
NAME: ias_math_solve_linear_equation

PURPOSE: Solve a symmetric positive-definite linear equation using 
         Cholesky decomposition

RETURNS: SUCCESS or ERROR

ALGORITHM REFERENCES:
D. Kuang modified from "Numerical Recipe in C", Dec. 1995.

******************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_math.h" /* prototypes for this module */

int ias_math_solve_linear_equation
(
    double **A, /* I/O: double pointer to the n x n coefficient matrix which
                        will be inverted */
    int n,      /* I: the dimension of the matrix */
    double b[]  /* I/O: The n x 1 constant vector, with first m to be
                        solved, where m is the rank of A.  The elements of
                        this vector will be replaced by the solution vector */
)
{
    double *p, *x, sum;      /* intermediate variables         */
    int i, j, k;             /* looping variables                 */

    /* Allocate memory for arrays for intermediate use. */
    p = malloc(n * sizeof(*p));        
    if (p == NULL)
    {
        IAS_LOG_ERROR("Allocating memory");
        return ERROR;
    }

    x = malloc(n * sizeof(*x));        
    if (x == NULL)
    {
        IAS_LOG_ERROR("Allocating memory");
        free(p);
        return ERROR;
    }

    /* Cholesky decomposition, only the upper triangle part of the input matrix
       A is required, the resulting decomposition is placed in the lower 
       triangle. */
    for (i = 0; i < n; i++)
    {
        for (j = i; j < n; j++)
        {
            sum = A[i][j];
            for (k = 0; k <= i - 1; k++)
                sum -= A[i][k] * A[j][k];
            if (i == j)
            { 
                if (sum <= 0.0)
                {
                    IAS_LOG_ERROR("Invalid sum obtained from decomposition");
                    free(p);
                    free(x);
                    return ERROR;
                }
                p[i] = sqrt(sum);
            }
            else
                A[j][i] = sum / p[i];
        }
    }
  
    /* Forward-substitute down the lower triangle of the decomposition. */
    for (i = 0; i < n; i++)
    {
        sum = b[i];
        for (k = 0; k <= i - 1; k++)
            sum -= A[i][k] * x[k];
        x[i] = sum / p[i];
    }

    /* Back-substitute to solve x and put back in b[]. */
    for (i = n - 1; i >= 0; --i)
    {
        sum = x[i];
        for (k = i + 1; k < n; k++)
            sum -= A[k][i] * x[k];
        x[i] = sum/p[i];
    }
    for (i = 0; i < n; i++)
        b[i] = x[i];

    /* Find the inverse of the lower triangle. */
    for (i = 0; i < n; i++)
    {
        A[i][i] = 1.0 / p[i];
        for (j = i + 1; j < n; j++)
        {
            sum = 0.0;
            for (k = i; k < j; k++)
                sum -= A[j][k] * A[k][i];
            A[j][i] = sum / p[j];
        }
    }

    free(p);
    p = NULL;
    free(x);
    x = NULL;

    /* Compute the upper triangle of the inverse of the whole A matrix. */
    for (i = 0; i < n; i++)
    {
        for (j = i; j < n; j++)
        {
            sum = 0.0;
            for (k = j; k < n; k++)
                sum += (A[k][i] * A[k][j]);
            A[i][j] = sum;
        }
    }

    /* Fill up the lower triangle part of the inverse matrix. */
    for (i = 0; i < n; i++)
    {
        for (j = i + 1; j < n; j++)
            A[j][i] = A[i][j];
    }

    return SUCCESS;
}
