/****************************************************************************
PURPOSE:
matrix routines

ROUTINES:
ias_math_transpose_matrix
ias_math_multiply_matrix
ias_math_invert_matrix
ias_math_add_matrix
ias_math_subtract_matrix
ias_math_decompose_lu_matrix
ias_math_back_substitute_lu_matrix

*******************************************************************************/

/****************************************************************************
NAME: ias_math_transpose_matrix

PURPOSE:
transposes a matrix

RETURN VALUE:
NONE
Type = void

*******************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_math.h"

#define TINY        1.0e-20        /* to avoid dividing by zero and for
                                      double value conditional check */

void ias_math_transpose_matrix
(
    const double *a,           /* I: matrix to be transposed */
    double *b,                 /* O: transpose of matrix */
    int row,                   /* I: number of rows in matrix */
    int col                    /* I: number of columns in matrix */
)
{
    int i,j; /* loop variables */
 
    /* transpose the matrix */
    for (i = 0 ; i < row ; i++)
    {
        for (j = 0 ; j < col ; j++)
            b[j*row+i] = a[i*col+j];
    }
}


/****************************************************************************
NAME: ias_math_multiply_matrix

PURPOSE:
multiplies two matrices

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion 
ERROR    Operation failed

NOTE: ias_math_multiply_3x3_matrix should be used for 3x3 multiplication

*******************************************************************************/
int ias_math_multiply_matrix
(
    const double *a,        /* I: ptr to matrix */
    const double *b,        /* I: ptr to matrix */
    double *m,              /* O: ptr to output matrix */
    int arow,               /* I: number of rows in a */
    int acol,               /* I: number of cols in a */
    int brow,               /* I: number of rows in b */
    int bcol                /* I: number of cols in b */
)
{
    int j, rr, cc;          /* loop vars */
    double sum;             /* sum of matrix elements */

    /* see if the matrices are compatible to multiply */
    if (acol != brow)
    {
        IAS_LOG_ERROR("Matrices are incompatible, unable to multiply");
        return ERROR; 
    }

    /* the matrix multiplication */
    for (rr = 0 ; rr < arow ; rr++)
    {
        for (cc = 0 ; cc < bcol ; cc++)
        {
            sum = 0.0;
            for (j = 0 ; j < acol ; j++)
                sum = sum + a[rr*acol+j]*b[j*bcol+cc];
            m[rr*bcol+cc] = sum;
        }
    }

    return SUCCESS;
}

/****************************************************************************
NAME: ias_math_invert_matrix

PURPOSE:
inverts a matrix

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion 
ERROR    Operation failed

*******************************************************************************/
int ias_math_invert_matrix
(
    const double *a,    /* I: input array */
    double *y,          /* O: ouput array */
    int n               /* I: dimension of a (n x n) */
)
{
    double d,           /* flag from ludcmp */
        *col,           /* array of columns */
        *mat;           /* temporary matrix */
    int i,j,*indx, status;

    col = (double *)malloc(n*sizeof(double));
    if (col == NULL)
    {
        IAS_LOG_ERROR("Error allocating array");
        return ERROR; 
    }
    indx = malloc(n*sizeof(int));
    if (indx == NULL)
    {
        IAS_LOG_ERROR("Error allocating array");
        free(col);
        return ERROR;
    }
    mat = (double *)malloc(n*n*sizeof(double));
    if (mat == NULL)
    {
        IAS_LOG_ERROR("Error allocating array");
        free(col);
        free(indx);
        return ERROR;
    }

    /* making a copy of the input array */
    for (i = 0 ; i < n*n ; i++)
        mat[i] = a[i];
 
    status = ias_math_decompose_lu_matrix(mat,n,indx,&d);
    if (status != SUCCESS)
    {
        free(col);
        free(indx);
        free(mat);
        IAS_LOG_ERROR("Error returned from ias_math_decompose_lu_matrix");
        return ERROR; 
    }
    for (j = 0 ;j < n ; j++)
    {
        for (i = 0 ; i < n ; i++)
             col[i] = 0.0;
        col[j] = 1.0;
        ias_math_back_substitute_lu_matrix(mat,n,indx,col);
        for (i = 0 ; i < n ; i++)
        {
            y[i*n+j] = col[i];
        }
    }

    free(col);
    free(indx);
    free(mat);
    return SUCCESS;
} 

/****************************************************************************
NAME: ias_math_add_matrix

PURPOSE:
adds two matrices

RETURN VALUE:
Type = none

*******************************************************************************/
void ias_math_add_matrix
(
    const double *a,        /* I: input matrix 1 */
    const double *b,        /* I: input matrix 2 */
    double *m,              /* O: output matrix */
    int row,                /* I: number of rows in matrices */
    int col                 /* I: number of cols in matrices */
)
{ 
    int rr, cc; /* loop vars */
 
    for (rr = 0 ; rr < row ; rr++)
    {
        for (cc = 0 ; cc < col ; cc++)
            m[rr*col+cc] = a[rr*col+cc] + b[rr*col+cc];
    }
}

/****************************************************************************
NAME: ias_math_subtract_matrix

PURPOSE:
subtracts two matrices

RETURN VALUE:
Type = void

*******************************************************************************/
void ias_math_subtract_matrix
(
    const double *a,         /* I: input matrix 1 */
    const double *b,         /* I: input matrix 2 */
    double *m,               /* O: output matrix */
    int row,                 /* I: number of rows in matrix */
    int col                  /* I: number of cols in matrix */
)
{ 
    int rr, cc;
 
    for (rr = 0 ; rr < row ; rr++)
    {
        for (cc = 0 ; cc < col ; cc++)
            m[rr*col+cc] = a[rr*col+cc] - b[rr*col+cc];
    }
}

/****************************************************************************
NAME: ias_math_decompose_lu_matrix

PURPOSE:
performs lu decomposition on a matrix

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion 
ERROR    Operation failed

*******************************************************************************/
int ias_math_decompose_lu_matrix
(
    double *a,                 /* I/O: input matrix, output LU decomp matrix */
    int n,                     /* I: size of matrix */
    int *indx,                 /* O: index */
    double *d                  /* O: flag */
)
{
    int i,imax = 0,j,k;
    double big,dum,sum,temp;
    double *vv;

    vv = (double *)malloc(sizeof(double) * n);
    if (vv == NULL)
    {
        IAS_LOG_ERROR("Error allocating array");
        return ERROR;
    }

    *d = 1.0;
    for (i = 0 ; i < n ; i++)
    {
        big = 0.0;
        for (j = 0 ; j < n ; j++)
        {
            if ((temp = fabs(a[i*n+j])) > big)    
                big = temp;
        }
        if (fabs(big) <= TINY)
        {
            IAS_LOG_ERROR("Singular matrix");
            free(vv);
            return ERROR; 
        }
        vv[i] = 1.0/big;
    }
    for (j = 0 ; j < n ; j++)
    {
        for (i = 0 ; i < j ; i++)
        {
            sum = a[i*n+j];
            for (k = 0 ; k < i ; k++)
                sum -= a[i*n+k]*a[k*n+j];
            a[i*n+j] = sum;
        }
        big = 0.0;
        for (i = j ; i < n ; i++)
        {
            sum = a[i*n+j];
            for (k = 0 ; k < j ; k++)
                sum -= a[i*n+k]*a[k*n+j];
            a[i*n+j] = sum;
            if ((dum = vv[i]*fabs(sum)) >= big)
            {
                big = dum;
                imax = i;
            }
        }
        if (j!=imax)
        {
            for (k = 0 ; k < n ; k++)
            {
                dum = a[imax*n+k];
                a[imax*n+k] = a[j*n+k];
                a[j*n+k] = dum; 
            }
            *d = - (*d);
            vv[imax] = vv[j];
        }
        indx[j] = imax;
        if (fabs(a[j*n+j]) <= TINY)
            a[j*n+j] = TINY;
        if (j!=(n-1))
        {
             dum = 1.0/a[j*n+j];
             for (i = j+1 ; i < n ; i++)
                 a[i*n+j] *= dum;
        }
    }
    free(vv);
    return SUCCESS;
}

/****************************************************************************
NAME: ias_math_back_substitute_lu_matrix

PURPOSE:
performs back substitution on a lu matrix

RETURN VALUE:
Type = none

*******************************************************************************/
void ias_math_back_substitute_lu_matrix
(
    double *a,              /* I/O: lu decomp matrix */
    int n,                  /* I: size of matrix */
    const int *indx,        /* I: flag */
    double b[]              /* O: solution vector */
)
{
    int i,ii = 0,ip,j;
    double sum;

    for (i = 1 ; i <= n ; i++)
    {
        ip = indx[i-1];
        sum = b[ip];
        b[ip] = b[i-1];
        if (ii)
        {
            for (j = (ii-1) ; j < i-1 ; j++)
                sum -= a[(i-1)*n+j]*b[j];
        }
        else if (sum) 
            ii = i;
        b[i-1] = sum;
    }
    for (i = (n-1) ; i>=0 ; i--)
    {
        sum = b[i];
        for (j = i+1 ; j < n ; j++)
            sum -= a[i*n+j]*b[j];
        if (a[i*n+i] == 0.0)
            a[i*n+i] = TINY;        
        b[i] = sum/a[i*n+i];
    }
}
