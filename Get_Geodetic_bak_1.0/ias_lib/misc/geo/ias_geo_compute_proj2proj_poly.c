/******************************************************************************
NAME: ias_geo_compute_proj2proj_poly

PURPOSE: Calculate the polynomials for a coordinate system transformation.

RETURN VALUE:   
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion
ERROR    Operation failed

******************************************************************************/
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_geo.h"

int ias_geo_compute_proj2proj_poly 
(
    const double source_proj_x[4], /*I: source projection x corner coordinates*/
    const double source_proj_y[4], /*I: source projection y corner coordinates*/
    const double target_proj_x[4], /*I: target projection x corner coordinates*/
    const double target_proj_y[4], /*I: target projection y corner coordinates*/
    int include_xy_term,           /*I: include x*y term if not zero */
    double poly_x[4],              /*O: calculated x polynomial coefficients */
    double poly_y[4]               /*O: calculated y polynomial coefficients */
)
{
    int i;                      /* Loop variable */
    int stat;                   /* Return status */
    double A[16];               /* Matrix used by QRsolve for LSQ fit */
    double v[4];                /* Vector used in above operation */
    int terms = 3;              /* number of terms in solution (set to 4 if the
                                   xy term is included in the solution */

    /* Calculate the first order polynomial coefficients which map output
     * line/samples to output projection coordinates.  The 4x3 matrix is
     * populated as shown below, QR-factorized and then Ax = b is solved
     * for two different vectors b; one for samples, the other for lines. */
    for (i = 0; i < 4; i++)
    {
        A[i] = 1.0;
    }
    A[4]  = source_proj_x[0];
    A[5]  = source_proj_x[1];    /*     +-                    -+ */
    A[6]  = source_proj_x[2];    /*     |  1  X0   Y0   X0*Y0  | */
    A[7]  = source_proj_x[3];    /* A = |  1  X1   Y1   X1*Y1  | */
    A[8]  = source_proj_y[0];    /*     |  1  X2   Y2   X2*Y2  | */
    A[9]  = source_proj_y[1];    /*     |  1  X3   Y3   X3*Y3  | */
    A[10] = source_proj_y[2];    /*     +-                    -+ */
    A[11] = source_proj_y[3];
    if (include_xy_term)
    {
        A[12] = A[4] * A[8];
        A[13] = A[5] * A[9];
        A[14] = A[6] * A[10];
        A[15] = A[7] * A[11];
        terms = 4;
    }

    stat = ias_math_matrix_QRfactorization(A, 4, terms, v, 0); 
                                            /* Obtain QR factorization */
    if (stat != SUCCESS)
    {
        IAS_LOG_ERROR ("Error performing QR factorization");
        return ERROR;
    }

    /* Vector one: Find the coefficients to transform output coordinates
       in projection space to output samples.  Note that the call
       to ias_math_matrix_QRsolve() modifies the values at poly_x for part of
       the output (this vector becomes the before-mentioned coefficients.) */
    poly_x[0] = target_proj_x[0];
    poly_x[1] = target_proj_x[1];
    poly_x[2] = target_proj_x[2];
    poly_x[3] = target_proj_x[3];
    stat = ias_math_matrix_QRsolve(A, 4, terms, v, poly_x, 0);
    if (stat != SUCCESS)
    {
        IAS_LOG_ERROR ("Error solving for X polynomial");
        return ERROR;
    }

    /* Vector two: Find the coefficients to transform output
     * coordinates in projection space to output lines. */
    poly_y[0] = target_proj_y[0];
    poly_y[1] = target_proj_y[1];
    poly_y[2] = target_proj_y[2];
    poly_y[3] = target_proj_y[3];
    stat = ias_math_matrix_QRsolve(A, 4, terms, v, poly_y, 0);
    if (stat != SUCCESS)
    {
        IAS_LOG_ERROR ("Error solving for Y polynomial");
        return ERROR;
    }

    return SUCCESS;
}
