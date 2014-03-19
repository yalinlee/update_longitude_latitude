/******************************************************************************
NAME: ias_geo_compute_poly

PURPOSE: Calculate the polynomial coefficients for converting from a projection
to line/sample.  Typically used with ias_math_eval_poly.

RETURN VALUE:   SUCCESS or ERROR

******************************************************************************/
#include "ias_logging.h"
#include "ias_const.h"
#include "ias_math.h"
#include "ias_geo.h"

int ias_geo_compute_proj2ls_poly 
(
    int number_samples,   /* I: Number of samples in the scene */ 
    int number_lines,     /* I: Number of lines in the scene */ 
    double corners[][4],  /* I: Lat and long for the four corners. Index 0 is
                                Y (lat) and 1 is X (long) with corners in 
                                UL, UR, LR, LL order. */
    double poly_x[4],     /* O: Projection-to-sample coefficients */
    double poly_y[4]      /* O: Projection-to-line coefficients */
)
{
    int i;                      /* Loop variable */
    int stat;                   /* Return status */
    double A[12];               /* Matrix used by QRsolve for LSQ fit */
    double v[4];                /* Vector used in above operation */

    /* Calculate the first order polynomial coefficients which map output
     * line/samples to output projection coordinates.  The 4x3 matrix is
     * populated as shown below, QR-factorized and then Ax = b is solved
     * for two different vectors b; one for samples, the other for lines.*/
    for (i = 0; i < 4; i++)
    {       
        A[i] = 1.0;
    }
    A[4]  = corners[1][0];
    A[5]  = corners[1][1];  /*     +-                                 -+ */
    A[6]  = corners[1][2];  /*     |  1  upper-left  X  upper-left  Y  | */
    A[7]  = corners[1][3];  /* A = |  1  upper-right X  upper-right Y  | */
    A[8]  = corners[0][0];  /*     |  1  lower-right X  lower-right Y  | */
    A[9]  = corners[0][1];  /*     |  1  lower-left  X  lower-left  Y  | */
    A[10] = corners[0][2];  /*     +-                                 -+ */
    A[11] = corners[0][3];

    /* Obtain QR factorization */
    stat = ias_math_matrix_QRfactorization(A, 4, 3, v, 0); 
    if (stat != SUCCESS)
    {
        IAS_LOG_ERROR ("Error performing QR factorization");
        return ERROR;
    }

    /* Vector one: Find the coefficients to transform output coordinates
     * in projection space to output samples.  Note that the call
     * to gxx_QRsolve() modifies the values at poly_x for part of the
     * output (this vector becomes the before-mentioned coefficients.) */
    poly_x[0] = 0.0;
    poly_x[1] = number_samples - 1.0;
    poly_x[2] = number_samples - 1.0;
    poly_x[3] = 0.0;
    stat = ias_math_matrix_QRsolve(A, 4, 3, v, poly_x, 0);
    if (stat != SUCCESS)
    {
        IAS_LOG_ERROR ("Error solving for X polynomial");
        return ERROR;
    }

    /* Vector two: Find the coefficients to transform output
     * coordinates in projection space to output lines. */
    poly_y[0] = 0.0;
    poly_y[1] = 0.0;
    poly_y[2] = number_lines - 1.0;
    poly_y[3] = number_lines - 1.0;
    stat = ias_math_matrix_QRsolve(A, 4, 3, v, poly_y, 0);
    if (stat != SUCCESS)
    {
        IAS_LOG_ERROR ("Error solving for Y polynomial");
        return ERROR;
    }

    return SUCCESS;
}
