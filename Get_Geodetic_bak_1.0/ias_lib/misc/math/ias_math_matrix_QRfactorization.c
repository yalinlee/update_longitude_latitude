/*****************************************************************************
NAME: ias_math_matrix_QRfactorization

PURPOSE: Factorize the A matrix to prepare for calling the QRsolve routine.

RETURN VALUE:
Type = int (SUCCESS)

ALGORITHM REFERENCES:
"Matrix Computations (2nd ed.)", Gene H. Golub and Charles F. Van Loan,
John Hopkins University Press, Baltimore 1989 ISBN 0-8018-3739-1

NOTES:
This source file also contains some internal, associated functions used in
the calculation.

******************************************************************************/
#include <math.h>
#include "ias_math.h"

/* Prototypes for forthcoming processing functions. */
static void house(int, double *, double *, double *, double *);
static void row_house(double *, int, int, int, double *, double);
static int sign(double); 
static double norm_2(double *, int);

#define LIMIT 1e-9

int ias_math_matrix_QRfactorization
(
    double *A,     /* I/O: Matrix A (stored by columns)     */
    int m,         /* I: Number of rows in matrix A         */
    int n,         /* I: Number of columns in matrix A      */
    double *v,     /* O: Vector v (work vector)             */
    int flag       /* I: If true, order matrix A by columns */
)
{
    int  N;                 /* Maximum loop iteration                        */
    int  i, j, k;           /* Loop counters                                 */
    double *cur_col_A;      /* Pointer to current column of matrix A         */
    double vtv, temp;       /* Temporary variables                           */

    /* By default, matrix A is assumed to be ordered by columns (that is, in
       column major form.)  If "flag" is set, the caller has passed the matrix
       in row major, so reorder the matrix so that we can work with it. */
    if (flag)
    {
        for (i = 0; i < m; i++)
        {
            for (j = i + 1; j < n; j++)
            {      
                k = (i * n) + j;
                temp = A[k];   
                A[k] = A[(j * n) + i];
                k = (j * n) + i;
                A[k] = temp;
            }
        }
    }

    /* Determine the maximum looping factor */
    N = ((m - 1) > n) ? n : (m - 1);

    for (k = 0; k < N; ++k)
    {
        cur_col_A = A + (k * m);
        /* Determine the Householder Transformation in vector v for a row in 
           the input vector A */
        house((m - k), &(cur_col_A[k]), &(v[k]), &vtv, &(cur_col_A[k]));

        for (j = k + 1; j < m; j++)
            cur_col_A[j] = v[j];

        /* Replace the m by n matrix A by the product vA. */
        row_house(&(A[(k + 1) * m + k]), (m - k), (n - k - 1), m, &(v[k]), vtv);
        v[k] = vtv;  
    } 

    return SUCCESS;
}

/******************************************************************************
NAME:                SIGN

PURPOSE:        Determine the sign of a quantity

RETURN VALUE:
Type =  int
Value    Description
-----    -----------
-1         Negative one if input value is less than 0.0
 1         Positive one if input value is greater than or equal to 0.0

******************************************************************************/
static int sign
(
    double val          /* I: Input value      */
)
{
    /* Return the sign of the input value */
    return (val < 0.0 ? -1 : 1);
}

/******************************************************************************
NAME:                NORM_2

PURPOSE:        Calculate the 2 - norm of a vector

RETURN VALUE:
Type = double
Value    Description
-----    -----------
sum         Square root of the total of squares of vector members

******************************************************************************/
static double norm_2
(
    double *x,     /* I: Input vector x                           */
    int n          /* I: Dimension of input vector                */
)
{
    int  i;             /* Loop counter                                */
    double sum = 0.0;   /* Running total of squares of vector members  */

    /* Sum the square of the input vector */
    for (i = 0; i < n; i++)
        sum += x[i] * x[i];

    /* Return the square root of the total of the squares */
    return (sqrt(sum)); 
}

/******************************************************************************
NAME:                HOUSE

PURPOSE:        Given vector x of length n, find a vector v of length n such
                that v[0] = 1, and for the Householder matrix P defined by
                v, Px = (Px, 0, ..., 0) transpose.

RETURN VALUE:
Type = void

******************************************************************************/
static void house
(
    int  n,      /* I: Dimension of vectors x and v             */
    double *x,   /* I: Input vector x                           */
    double *v,   /* O: Vector v                                 */
    double *vtv, /* O: Product of vector v transpose and v      */
    double *Px   /* O: Product of Householder matrix P and x    */
)
{ 
    int  i;                 /* Loop counter                     */
    double u;               /* Norm variable                    */
    int  sign_x0 = 1;       /* Sign variable (contains 1 or -1) */
    double beta;            /* Temporary variable               */

    /* Copy the x vector into the v vector */
    for (i = 0; i < n; i++)
        v[i] = x[i];

    /* Calculate the norm (unit) of the x vector */
    u = norm_2(x, n);
    if (u >= LIMIT)
    {
        sign_x0 = sign(x[0]);

        /* Calculate the Householder transformation vector.  A Householder
           transformation depends only on the ratios of the elements. */
        beta = x[0] + sign_x0 * u;
        for (i = 1; i < n; i++)
            v[i] /= beta;

        /* Calculate the product of vector v transpose and v */
        *vtv = 2.0 * ((u * u) + (u * fabs(x[0]))) / (beta * beta);
    }
    else *vtv = 1.0;

    v[0] = 1.0;
    *Px = -sign_x0 * u;
} 

/******************************************************************************
NAME:                ROW_HOUSE

PURPOSE:        Replace the m by n matrix A by the product PA, where P is
                the Householder Transformation defined by vector v.  vtv
                should contain the product of v transpose v when this routine
                is called.
******************************************************************************/
static void row_house
(
    double *A,      /* I/O: Matrix A (stored by columns)      */
    int  m,         /* I: Number of rows in matrix A          */
    int  n,         /* I: Number of columns in matrix A       */
    int  mdim,      /* I: Row dimension                       */
    double *v,      /* I: Vector v                            */
    double vtv      /* I: Product of vector v transpose and v */
)
{
    double *cur_col_A;      /* Pointer to current column of matrix A        */
    double alpha;           /* Temporary variable                           */
    int  i,k;               /* Loop counters                                */

    for (k = 0; k < n; k++)
    {
        /* Set the current column of matrix A */
        cur_col_A = A + (k * mdim);

        /* Sum the elements of the current column by multiplying the Householder
           transformation time each element (the row). */
        for (alpha = 0.0, i = 0; i < m; i++)
            alpha += v[i] * cur_col_A[i];

        alpha *= -(2.0 / vtv);

        /* Transpose the elements of A */
        for (i = 0; i < m; i++)
            cur_col_A[i] += (alpha * v[i]);
    }
} 
