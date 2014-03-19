/******************************************************************************
NAME: ias_math_cubic_convolution

PURPOSE:        
The cubic_convolution function calculates the cubic convolution weight for
a point.  This is done with cubic convolution interpolation using a set of 
cubic spline interpolating polynomials.

RETURN VALUE:
The cubic convolution weight is returned as a double.

ALGORITHM REFERENCES:
Park, S.K., R.A. Schowengerdt, Image Reconstruction by Parametric Cubic
Convolution, Computer Vision, Graphics and Image Processing, v.23, no.3,
September 1983.

******************************************************************************/

#include <math.h>       /* fabs function */
#include "ias_math.h"

double ias_math_cubic_convolution
(
    double alpha,   /* I: cubic convolution alpha parameter */
    double x        /* I: value to perform cubic convolution on */
)
{
    double fx;  /* absolute value of x */
    double fx2; /* x squared */
    double fx3; /* absolute value of x cubed */
    double ccw; /* cubic convolution weight */

    fx = fabs(x);

    /* calculate weight for piecewise defined function */
    if (fx < 1.0)
    {
        /* calculate weight when x is less than 1 */
        fx2  = fx * fx;
        fx3  = fx2 * fx;
        ccw = (alpha + 2.0) * fx3 - (alpha + 3.0) * fx2 + 1.0;
    }
    else if (fx < 2.0)
    {
        /* calculate weight when x is less than 2, but greater than or equal
           to 1 */
        fx2 = fx * fx;
        fx3 = fx2 * fx;
        ccw = alpha * fx3 - 5.0 * alpha * fx2 + 8.0 * alpha * fx - 4.0 * alpha;
    }
    else
    {
        /* weight is zero when x is two or more */
        ccw = 0.0;
    }

    return ccw;
}

