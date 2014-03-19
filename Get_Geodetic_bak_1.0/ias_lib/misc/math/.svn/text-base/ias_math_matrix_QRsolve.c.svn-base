/*****************************************************************************
NAME:             ias_math_matrix_QRsolve

PURPOSE:
Find the least squares solution of Ax = b where A is a full rank m by n
matrix with m >= n.  On entry, matrix A should contain the results of the
QR routine.  If iflag == 0, then v is a vector of length m containing the
results of the QR routine.  If iflag != 0, the entries in v are
reconstructed--the vector v is not changed.  The solution will be stored
in the first n entries of vector b.

RETURN VALUE:
Type = int SUCCESS

ALGORITHM REFERENCES:
"Matrix Computations (2nd ed.)", Gene H. Golub and Charles F. Van Loan,
John Hopkins University Press, Baltimore 1989 ISBN 0-8018-3739-1

******************************************************************************/
#include "ias_math.h"

int ias_math_matrix_QRsolve
(
    const double *A, /* I: Matrix A (stored by columns)*/
    int  m,          /* I: Number of rows in matrix A */
    int  n,          /* I: Number of columns in matrix A */
    const double *v, /* I: Vector v */
    double *b,       /* O: Vector b */
    int  iflag       /* I: flag (see comments above) */
)
{  
    int  N;                 /* Maximum loop iteration                       */
    int  k,j;               /* Loop counters                                */
    const double *cur_col_A;/* Pointer to current column of matrix A        */
    double sum;             /* Accumulator                                  */
    double vtv;             /* Temporary variable                           */
    double alpha;           /* Temporary variable                           */

    /* Determine the maximum looping */
    N = ((m - 1) > n) ? n : (m - 1);

    for (k = 0; k < N; k++)
    {
        /* Set a pointer to the current column in vector A */
        cur_col_A = A + (k * m);

        sum = b[k];
        for (j = k + 1; j < m; j++)
            sum += (b[j] * cur_col_A[j]);

        if (iflag == 0)
            vtv = v[k];
        else
        {
            vtv = 1.0;
            for (j = k + 1; j < m; j++)
                vtv += (cur_col_A[j] * cur_col_A[j]);
        }

        alpha = -2.0 / vtv * sum;
        b[k] += alpha;

        for (j= k + 1; j < m; j++)
            b[j] += (alpha * cur_col_A[j]);
    }

    /* Backsolve. */
    b[n - 1] /= A[(n - 1) * m + n - 1];
    for (k = n - 1; k--;)
    {
        /* Set the current column in vector A */
        cur_col_A = A + ((k + 1) * m);

        for (j = 0; j < k + 1; j++)
            b[j] -= (b[k + 1] * cur_col_A[j]);

        b[k] /= A[k * m + k];
    }

    return SUCCESS;
}
