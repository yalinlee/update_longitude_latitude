/*******************************************************************************
NAME: ias_geo_compute_forward_mappings

PURPOSE:
This function, given grid points in both input and output space, generates
the mapping polynomial coefficients needed to convert from a line/sample in
input space (satellite) to one in output space (projection).  It generates
these coefficients for every cell in the grid.

RETURN VALUE:
Type = int (SUCCESS or ERROR)

*******************************************************************************/
#include <math.h>
#include "ias_const.h"
#include "ias_math.h"
#include "ias_structures.h"
#include "ias_geo.h"

int ias_geo_compute_forward_mappings
(
    int nzplanes,       /* I: List of planes in input space */
    const int *il,      /* I: List of lines in input space */
    const int *is,      /* I: List of samples in input space */
    const double *ol,   /* I: List of lines in output space */
    const double *os,   /* I: List of samples in output space */
    int nrows,          /* I: Number of rows in the grid */
    int ncols,          /* I: Number of columns in the grid */
    struct IAS_COEFFICIENTS *coef /* O: Forward mapping coefficients */
)
{
    double A[36];    /* 9 x 4 matrix to populate and factorize */
    double b[9];     /* Vector for ias_math_matrix_QRsolve() to solve with */
    double v[9];     /* Vector used in least squares solution */
    int z, i, r, c;  /* Loop counters */
    int curcell = 0; /* Current grid cell being examined */
    int points_per_zplane; /* Point in one level of elevation */
    int zoffset;     /* Offset due to elevation level */

    /* Calculate forward mapping coefficients for every cell in the grid,
       storing the cell coefficients left-to-right, top-to-bottom in the
       array that was passed in by the caller.

       +-----------------------------+
       |         |         |         |
       | coef[0] | coef[1] | coef[2] |
       |         |         |         |
       +---------+---------+---------+
       |         |         |         |
       | coef[3] | coef[4] | coef[5] | etc ...
       |         |         |         |
       +-----------------------------+ */

    points_per_zplane = (nrows+1) * (ncols+1);

    for( z = 0; z < nzplanes; z++ )
    {
        for (r = 0; r < nrows ; r++) /* For each row .. */
        {
            for (c = 0; c < ncols; c++) /* For each column .. */
            {

                /* Populate the A matrix with values to represent the lines and
                   samples in input space, and then obtain the QR factorization.
                 */
                for( i = 0; i < 9; i++ )
                    A[i] = 1.0;
                A[9]  = is[c];
                A[10]  = is[c + 1];
                A[11]  = is[c];
                A[12]  = is[c + 1];
                A[13] = (A[9] + A[10] + A[11] + A[12]) / 4.0;
                A[14] = (A[9] + A[10]) / 2.0;
                A[15] = (A[10] + A[12]) / 2.0;
                A[16] = (A[11] + A[12]) / 2.0;
                A[17] = (A[9] + A[11]) / 2.0;

                A[18]  = il[r];
                A[19]  = il[r];
                A[20] = il[r + 1];
                A[21] = il[r + 1];
                A[22] = (A[18] + A[19] + A[20] + A[21]) / 4.0;
                A[23] = (A[18] + A[19]) / 2.0;
                A[24] = (A[19] + A[21]) / 2.0;
                A[25] = (A[20] + A[21]) / 2.0;
                A[26] = (A[18] + A[20]) / 2.0;

                A[27] = A[9] * A[18];
                A[28] = A[10] * A[19];
                A[29] = A[11] * A[20];
                A[30] = A[12] * A[21];
                A[31] = A[13] * A[22];
                A[32] = A[14] * A[23];
                A[33] = A[15] * A[24];
                A[34] = A[16] * A[25];
                A[35] = A[17] * A[26];
                ias_math_matrix_QRfactorization(A, 9, 4, v, 0);

                /* Now place the output samples in the b vector and call
                   ias_math_matrix_QRsolve() to solve the equation Ax = b for x.
                   The resulting vector is our satellite-to-projection
                   coefficients for samples in the input space.  Once this
                   has been done, copy the returned coefficients into the
                   coefficients array whose address was passed us. */
                zoffset = points_per_zplane * z;
                b[0] = os[(ncols + 1) * r + c + zoffset];
                b[1] = os[(ncols + 1) * r + c + 1 + zoffset];
                b[2] = os[(ncols + 1) * (r + 1) + c + zoffset];
                b[3] = os[(ncols + 1) * (r + 1) + c + 1 + zoffset];
                b[4] = (b[0] + b[1] + b[2] + b[3]) / 4.0;
                b[5] = ( b[0] + b[1] ) / 2.0;
                b[6] = ( b[1] + b[3] ) / 2.0;
                b[7] = ( b[2] + b[3] ) / 2.0;
                b[8] = ( b[0] + b[2] ) / 2.0;
                ias_math_matrix_QRsolve(A, 9, 4, v, b, 0);
                for (i = 0; i < 4; ++i)
                    coef[curcell].a[i] = b[i];


                /* Repeat this process for the output lines. */
                b[0] = ol[(ncols + 1) * r + c + zoffset];
                b[1] = ol[(ncols + 1) * r + c + 1 + zoffset];
                b[2] = ol[(ncols + 1) * (r + 1) + c + zoffset]; 
                b[3] = ol[(ncols + 1) * (r + 1) + c + 1 + zoffset];
                b[4] = (b[0] + b[1] + b[2] + b[3]) / 4.0;
                b[5] = ( b[0] + b[1] ) / 2.0;
                b[6] = ( b[1] + b[3] ) / 2.0;
                b[7] = ( b[2] + b[3] ) / 2.0;
                b[8] = ( b[0] + b[2] ) / 2.0;
                ias_math_matrix_QRsolve(A, 9, 4, v, b, 0);
                for (i = 0; i < 4; ++i)
                    coef[curcell].b[i] = b[i];

                /* Increment the cell count and go back to process
                   the next column (or start another row). */
                curcell++;
            }
        }
    }
    return SUCCESS;
} 
