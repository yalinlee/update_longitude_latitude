/****************************************************************************
FILE:           ias_math_fit_registration

PURPOSE:
Fits a quadratic surface to the neighborhood of the correlation peak and from it
determine the best-fit registration offsets and their estimated errors.  This 
file contains the following functions:
        ias_math_fit_registration
        sums
        invert
        lu_fact
        back_solve
        init_pivot
        find_pivot
        esterr

*****************************************************************************/
#include <math.h>
#include "ias_math.h"
#include "ias_logging.h"
#include "local_defines.h"

#define GMAX(A, B)      ((A) > (B) ? (A) : (B)) /* assign maximum of a and b */

/****************************************************************************
NAME:                                IAS_SUMS

PURPOSE:  Compute the sums of terms for the matrix elements and column vector 
          used for determining the best-fit surface to the cross-correlation 
          values in the neighborhood of the peak

RETURN VALUE:
No return value

*****************************************************************************/
static void ias_sums
(
    const double *cpval, /* I: 3 by 3 array of xcorr values, in std dev      */
    IAS_CORRELATION_FIT_TYPE fit_method, /* I: Method of fitting surface */
    double *z,           /* O: Function of xcorr to which quadratic is fit   */
    double *wghts,       /* O: Weights to be assigned to points in 3 by 3 
                               array                                         */
    double b[MAX_DIM][MAX_DIM],/* O: Matrix of sums of polynomial terms       */
    double *vector       /* O: Col vector obtained by summing products of poly 
                            terms and an approp. function of xcorr value     */
)
{
    int i,ic,ir,j;       /* Loop counters                                    */
    int ivalpt;          /* Index of xcorr vals and weights in peak area     */
    double term[6];      /* Terms in approximating quadratic polynomial      */
    double val;          /* Value of one element in CPVAL array              */
 
    /* Initialize arrays and constants */
    for (i = 0; i < MAX_DIM; i++)
    {
        for (j = 0; j < MAX_DIM; j++)
            b[i][j] = 0.0;
        vector[i] = 0.0;
    }
 
    /* Compute function of correlation coefficient and assign weight for each 
       location in neighborhood of peak */
    term[0] = 1.0;
    for (ivalpt = 0,ir = 0; ir < 3; ir++)
    {
        for (ic = 0; ic < 3; ic++, ivalpt++)
        {
            val = GMAX(cpval[ivalpt], 1.0);
            if (fit_method == IAS_FIT_ELLIP_PARA)
            {
                z[ivalpt] = val;
                wghts[ivalpt] = 1.0;
            }
            else if (fit_method == IAS_FIT_ELLIP_GAUSS)
            {
                z[ivalpt] = -(log(val));
                wghts[ivalpt] = val * val;
            }
            else /* fit_method == IAS_FIT_RECIP_PARA */
            {
                z[ivalpt] = 1.0/val;
                wghts[ivalpt] = val * val * val * val;
            }
 
            /* Compute matrix and vector elements in linear equations for
               best-fit*/
            term[1] = (double)(ic - 1);
            term[2] = (double)(ir - 1);
            term[3] = term[1] * term[1];
            term[4] = term[1] * term[2];
            term[5] = term[2] * term[2];
            for (i = 0; i < MAX_DIM; i++)
            {
                vector[i] += wghts[ivalpt] * term[i] * z[ivalpt];
                for (j = 0; j < MAX_DIM; j++)
                    b[i][j] += wghts[ivalpt] * term[i] * term[j];
            }
        }
    }
}

/******************************************************************************
NAME                                FIND_PIVOT

RETURN VALUE:
Type = int
Value           Description
-----           -----------
max_row         row with the maximum value

******************************************************************************/
static int find_pivot
(
    int  n,                    /* I: dimension of matrix */
    double (*matrix)[MAX_DIM], /* I: matrix A */
    int  k,                    /* I: Current row (zero-relative) */
    const double *s            /* I: S array */
)
{
    double max;                /* Maximum value found                    */
    int  j;                    /* Loop counter                           */
    double test;               /* Temporary value used in max value test */
    int max_row = 0;           /* Row containing the maximum value       */

    /* Find the row (zero relative) which have the largest value after being
       scaled with the corresponding element of the s array. */
    max = 0.0;
    for (j = k; j < n; j++)
    {
        test = matrix[j][k] / s[j];
        test = fabs(test);
        if (test > max)
        {
            max = test;
            max_row = j;
        }
    }

    return max_row;
}

/******************************************************************************
NAME: INIT_PIVOT

PURPOSE:  Scaled column pivoting.  The first routine (init_pivot) sets up the 
          s array.  The second routine (find_pivot) finds the pivot element and
          returns it to the caller.  These routines are isolated so that
          different pivoting methods can be used with little re-coding
          (although the s array may be unused).

RETURN VALUE:
Type = int
Value           Description
-----           -----------
ERROR           Can't pivot
SUCCESS         s array set up

******************************************************************************/
static int init_pivot
(
    int  n,                        /* I: dimension of matrix     */
    double (*matrix)[MAX_DIM],     /* I: matrix A                */
    double *s                      /* O: S array                 */
)
{
    int  i,j;                      /* Loop counters */
    double test;                   /* Temporary variable for maximum test */

    /* Find the value of the abs(largest) element in a row. */
    for (i = 0; i < n; i++)
    {
        for (s[i] = 0.0, j = 0; j < n; j++)
        {
            test = fabs(matrix[i][j]);
            if (test > s[i])
                s[i] = test;
        }

        /* If s[i] is zero, all elements in the row are zero--something this
           routine won`t deal with... */
        if (s[i] == 0.0)
        {
            IAS_LOG_ERROR("Matrix row contains all zeroes");
            return ERROR;
        }
    }
    return SUCCESS;
}

/****************************************************************************
NAME:                                LU_FACT

PURPOSE:  Routine to perform the LU factorization of a matrix.  Upon input,
          "matrix" contains matrix A and upon output "matrix" contains A's
          LU factorization.  The pivoting method used is scaled column
          pivoting.

RETURN VALUE:
Type = int
Value           Description
-----           -----------
ERROR           LU factorization not calculated
SUCCESS         Success

******************************************************************************/
static int lu_fact
(
    int n,                        /* I: Dimension of matrix                 */
    double (*matrix)[MAX_DIM],    /* I/O: Matrix A                          */
    int *ipvt                     /* O: Record of pivots performed          */
)
{
    double s[MAX_DIM];   /* Vector containing abs() of max val for each row */
    double factor;       /* Factor used in zeroing entries (stored) */
    int n_1;             /* Parameter n minus 1 */
    int i, j, k;         /* Loop counters */
    double temp;         /* Temporary variable for row swapping */

    if ((n <= 1) && (matrix[0][0] == 0.0))
    {
        IAS_LOG_ERROR("Cannot calculate LU factorization of matrix");
        return ERROR;
    }
    n_1 = n - 1;

    /* Initialize scaled column pivoting (calculate the s vector). */
    if (init_pivot(n, matrix, s) == ERROR)
    {
        IAS_LOG_ERROR("Cannot set up matrix pivot");
        return ERROR;
    }

    /* Factor the matrix */
    for (k = 0; k < n_1; k++)
    {
        ipvt[k] = find_pivot(n, matrix, k, s);

        /* Perform row interchange, if necessary */
        if (ipvt[k] != k)
        {
            for (i = k; i < n; i++)
            {
                temp = matrix[k][i];
                matrix[k][i] = matrix[ipvt[k]][i];
                matrix[ipvt[k]][i] = temp;
            }
        }
        if (matrix[k][k] == 0.0)
        {
            IAS_LOG_ERROR("Cannot calculate LU factorization of matrix");
            return ERROR;
        }

        /* Factor the row */
        for (i = k + 1; i < n; i++)
        {
            factor = -matrix[i][k] / matrix[k][k];
            matrix[i][k] = factor;
            for (j = k + 1; j < n; j++)
                matrix[i][j] += factor * matrix[k][j];
        }
    }
    if (matrix[n_1][n_1] == 0.0)
    {
        IAS_LOG_ERROR("Cannot calculate LU factorization of matrix");
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
NAME:                                        BACK_SOLVE

PURPOSE:  Routine to solve the LU factorized matrix A, given a vector b.
          Upon output, vector b is really vector x.

RETURN VALUE:
No return value

******************************************************************************/
static void back_solve
(
    int  n,                        /* I: dimension of matrix                 */
    double (*matrix)[MAX_DIM],     /* I: LU factorization of matrix A        */
    int  *ipvt,                    /* I: Record of pivots                    */
    double *b                      /* I/O: b (in) & x (out) vector           */
)
{
    int  k, j, i;                  /* Loop counters                          */
    int  n_1;                      /* Dimension of matrix - 1                */
    int  kp1;                      /* Var to reduce computations             */
    double temp;                   /* Temporary variable for interchanges    */

    n_1 = n - 1;

    /* Align vector b so that it corresponds to the row interchanges made 
       during the factorization process. */
    for (k = 0; k < n_1; k++)
    {
        if (k != ipvt[k])
        {
            temp = b[k];
            b[k] = b[ipvt[k]];
            b[ipvt[k]] = temp;
        }

        /* Solve Ly = b */
        for (i = k + 1; i < n; i++)
            b[i] = b[i] + matrix[i][k] * b[k];
    }

    /* Backsolve--Solve Ux = y */
    b[n_1] = b[n_1] / matrix[n_1][n_1];
    for (k = n_1; k--;)
    {
        kp1 = k + 1;
        for (j = 0; j < kp1; j++)
            b[j] = b[j] - matrix[j][kp1] * b[kp1];
        b[k] = b[k] / matrix[k][k];
    }
}

/****************************************************************************

NAME: INVERT                        

PURPOSE:  Driver program for Gaussian Elimination (LU factorization) using 
          scaled column pivoting.  All arrays are zero relative--the first 
          element of matrix A is A[0][0].  Solves Ax = b where A is an n x n 
          matrix and x and b are n dimensional vectors.

RETURN VALUE:
Type = int
Value           Description
-----           -----------
ERROR           Matrix not invertible
SUCCESS         Matrix inverted

*****************************************************************************/
static int invert
(
    double (*matrix)[MAX_DIM],    /* I: Matrix A                         */
    double (*outA)[MAX_DIM],      /* O: Matrix A                         */
    int n                         /* I: Dimension of matrix              */
)
{
    int ipvt[MAX_DIM];            /* Array of pivots                     */
    double b[MAX_DIM];            /* b & resulting x vectors             */
    int i,j;                      /* Loop counter                        */
    int status;                   /* Return status flag                  */

    /* Factor matrix */
    status = lu_fact(n, matrix, ipvt);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("No solution--matrix not invertible");
        return ERROR;
    }

    /* If factored successfully, solve */
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
            b[j] = 0.0;
        b[i] = 1.0;
        back_solve(n, matrix, ipvt, b);
        for (j = 0; j < n; j++)
            outA[j][i] = b[j];
    }
    return SUCCESS;
}

/****************************************************************************
NAME:                                ESTERR

PURPOSE:  Estimate the probable errors in the best-fit location of the 
          correlation peak

RETURN VALUE:
No return value

*****************************************************************************/
static void esterr
(
    double *z,       /* I: Function of xcorr val to which quadratic is fit    */
    double *wghts,   /* I: Weights for each point in 3x3 array of values      */
    double bnvrs[MAX_DIM][MAX_DIM],/* I: Inverse of matrix B                 */
    double *coeffs,  /* I: Coefs of best-fit quad surface to values of Z   */
    double *pkoffs,  /* I: Best-fit offset of peak to center of 3x3 array  */
    double *est_err  /* O: Est horiz & vert error & h-v cross term         */
)
{
    int  i, j, x, y;  /* Loop counters                                      */
    int  ivalpt;      /* Index to values det from measured corr coeffs      */
    double du[MAX_DIM];/* Partial derivatives of horiz best-fit peak offset 
                         with respect to coefs of polynomial surface        */
    double dv[MAX_DIM];/* Partial derivatives of vertical offset             */
    double c;         /* Constant for computing weights or errors           */
    double denom;     /* Denominator used for partial derivatives           */
    double f;         /* Value of function of corr coefficient computed 
                         using best-fit coefficients                        */
    double usum;      /* Accum. for sum of terms to determine est errors in 
                         horizontal offset                                  */
    double var;       /* Residual variance of fit                           */
    double vsum;      /* Sum of terms to estimate vertical offset error     */
    double xsum;      /* Accum. for sum of cross terms to est errors when 
                           search chip was rotated                          */

    /* Compute residual variance for elements of peak array */
    ivalpt = 0;
    var = 0.0;
    for (y= -1; y <= 1; y++)
        for (x= -1; x <= 1; x++)
        {
            f = coeffs[0] + coeffs[1] * (double)x + coeffs[2] * (double)y + 
                coeffs[3] * (double)x * (double)x + coeffs[4] * (double)x * 
                (double)y + coeffs[5] * (double)y * (double)y;
            var += wghts[ivalpt] * (f - z[ivalpt]) * (f - z[ivalpt]);
            ivalpt++;
        }

    /* Compute constant to use with weights */
    c = var/19.0;

    /* Compute partial derivatives of peak offsets with respect to poly coefs
       i.e., pkoffs[0] = (c2 * c4 - 2.0 * c1 * c5) / (4.0 * c3 * c5 - c4 * c4)
       so dp0/dc1 = -2.0 * c5 / (4.0 * c3 * c5 - c4 * c4)
        pkoffs[1] = (c1 * c4 - 2.0 * c2 * c3) / (4.0 * c3 * c5 - c4 * c4)
       so dp1/dc1 = c4 / (4.0 * c3 * c5 - c4 * c4) */
    denom = 4.0 * coeffs[3] * coeffs[5] - coeffs[4] * coeffs[4];
    du[0] = 0.0;
    du[1] = -2.0 * coeffs[5]/denom;
    du[2] = coeffs[4]/denom;
    du[3] = -4.0 * coeffs[5] * pkoffs[0]/denom;
    du[4] = (coeffs[2] + 2.0 * coeffs[4] * pkoffs[0])/denom;
    du[5] = (-2.0 * coeffs[1] - 4.0 * coeffs[3] * pkoffs[0]) / denom;

    dv[0] = 0.0;
    dv[1] = du[2];
    dv[2] = -2.0 * coeffs[3] / denom;
    dv[3] = (-2.0*coeffs[2] - 4.0 * coeffs[5] * pkoffs[1])/denom;
    dv[4] = (coeffs[1] + 2.0 * coeffs[4] * pkoffs[1])/denom;
    dv[5] = -4.0 * coeffs[3] * pkoffs[1]/denom;

    /* Compute estimated errors in offsets */
    usum = vsum = xsum = 0.0;
    for (i = 0; i < MAX_DIM; i++)
    {
        for (j = 0; j < MAX_DIM; j++)
        {
            usum += du[i] * du[j] * bnvrs[i][j];
            vsum += dv[i] * dv[j] * bnvrs[i][j];
            xsum += du[i] * dv[j] * bnvrs[i][j];
        }
    }
    est_err[0] = sqrt(fabs(c*usum));
    est_err[1] = sqrt(fabs(c*vsum));
    est_err[2] = c * xsum;
}

/*******************************************************************************
NAME:        ias_math_fit_registration

PURPOSE:  Fits a quadratic surface to the neighborhood of the correlation peak 
          and from it determine the best-fit registration offsets and their 
          estimated errors

RETURN VALUE:
Type = int
Value           Description
-----           -----------
ERROR           Error fitting (couldn't invert matrix)
SUCCESS         Success

ALGORITHM REFERENCES:
1.0  LAS 4.0 GREYCORR & EDGECORR by R. White 6/83

*****************************************************************************/
int ias_math_fit_registration
(
    const double *cpval,/* I: 3 by 3 array of xcorr vals, in units of standard 
                              dev above background, centered on corr peak */
    IAS_CORRELATION_FIT_TYPE fit_method, /* I: Method of surface fit: 
                                 FIT_ELLIP_PARA - Elliptical paraboloid
                                 FIT_ELLIP_GAUSS - Elliptical Gaussian
                                 FIT_RECIP_PARA - Reciprocal Paraboloid */
    double *pkoffs,   /* O: Best-fit horiz and vertical offsets of correlation
                            peak relative to center of 3 by 3 array */
    double *est_err   /* O: Estimated horizontal error [0], vertical error [1]
                            and h-v cross term [2] in best-fit offsets */
)
{
    int i, j;                  /* Loop counters */
    double B[MAX_DIM][MAX_DIM];/* Matrix of sums of polynomial terms */
    double A[MAX_DIM][MAX_DIM];/* Inverse of Matrix B */
    double coeffs[MAX_DIM];    /* Coefs of best-fit quadratic surface */
    double vector[MAX_DIM];    /* Right side Col vector to solve for coefs */
    double wghts[9];           /* Weight assigned to each value in 3x3 area 
                                  of peak */
    double z[9];               /* Xcorr coeffs to which 3x3 surface is to be 
                                  fit */
    double denom;              /* Denominator of equation for best-fit offsets*/

    /* Compute elements of matrix and column vector */
    ias_sums(cpval, fit_method, z, wghts, B, vector);

    /* Invert matrix to get best-fit peak offsets */
    if (invert(B, A, MAX_DIM) == ERROR)
    {
        IAS_LOG_ERROR("Error inverting matrix of polynomial terms");
        return ERROR;
    }
    for (i = 0; i < MAX_DIM; i++) 
    {
        for (coeffs[i] = 0.0, j = 0; j < MAX_DIM; j++)
             coeffs[i] += A[i][j] * vector[j];
    }
    denom = 4.0 * coeffs[3] * coeffs[5] - coeffs[4] * coeffs[4];
    if (denom == 0.0)
    {
        pkoffs[0] = 0.0;
        pkoffs[1] = 0.0;
        est_err[0] = 0.0;
        est_err[1] = 0.0;
        est_err[2] = 0.0;
    }
    else
    {
        pkoffs[0] = 
                 (coeffs[2] * coeffs[4] - 2.0 * coeffs[1] * coeffs[5]) / denom;
        pkoffs[1] = 
                 (coeffs[1] * coeffs[4] - 2.0 * coeffs[2] * coeffs[3]) / denom;

        /* Estimate errors in best-fit offsets */
        esterr(z, wghts, A, coeffs, pkoffs, est_err);
    }

    return SUCCESS;
}

