/****************************************************************************
Name: ias_math_eval_poly

Purpose: Evaluates a polynomial at a given point without including the x*y
    terms

Returns:
    nothing

Notes:
 - This is an evaluation of a bidimensional polynomial (x and y terms) of a
   given degree, where the fit was done not using the x*y terms.  (The 
   ias_math_eval_poly_xy routine uses the x*y terms.)
 - This routine is the equivalent xxx_eval in the alias code.

******************************************************************************/
#include <stdlib.h>
#include "ias_logging.h"
#include "ias_math.h"

void ias_math_eval_poly
(
    int degree,      /* I: Degree of polynomial                      */
    const double *a, /* I: Array of polynomial coefficients          */
    double x,        /* I: X coordinates to be evaluated             */
    double y,        /* I: Y coordinates to be evaluated             */
    double *val      /* O: Value of the polynomial at the point      */
)
{
    switch (degree)
    {
        case 4:    
             *val = a[0] + x * (a[1] + x * (a[3]
                  + x * (a[6] + x * a[10] + y * a[11]) + y * a[7]) + y * a[4])
                  + y * (a[2] + y * (a[5] + y * (a[9] + y * a[14] + x * a[13])
                  + x * a[8] + x * x * a[12]));
             break;
        case 3:
            *val = a[0] + x * (a[1] + x * (a[3] + a[6] * x + a[7] * y)
                 + a[4] * y) + y * (a[2] + y * (a[5] + a[9] * y + a[8] * x));
             break;
        case 2:
            *val = a[0] + x * (a[1] + a[3] * x + a[4] * y)
                 + y * (a[2] + a[5] * y);
             break;
        case 1:
            *val = a[0] + a[1] * x + a[2] * y;
             break;
        default:
            /* should never be called with this, so catastrophic error */
            IAS_LOG_ERROR("Unsupported degree number: %d", degree);
            exit(EXIT_FAILURE);
    } 
}
