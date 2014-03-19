/****************************************************************************
Name: ias_math_eval_poly_xy        

Purpose: Evaluates a polynomial at a given point

Returns:
    Result of polynomial evaluation (double)

NOTES:
 - Takes two points in an x/y coordinate system and returns a new values
   according to the polynomial defined by the coefficients a.  It is the
   evaluation of a bidimensional polynomial (x and y terms) of a given degree
   where the fit was done using x*y terms.  (The ias_math_eval_poly routine
   does not use the x*y terms.)
 - This routine is the equivalent of xxx_eval_poly in the alias code.

*****************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "ias_logging.h"
#include "ias_math.h"

double ias_math_eval_poly_xy
(
    int degree,     /* I: Degree of polynomial */
    const double *a,/* I: Array of polynomial coefficients */
    double x,       /* I: X coordinates to be evaluated */
    double y        /* I: Y coordinates to be evaluated */
)
{
    double val = 0.0;
    int i,m;             /* loop variables */ 
    double xp,yp;        /* y coordinate raised to (degree) power */
    double x2,x3,y2,y3;  /* x and y values squared and cubed */
    const double *coeff; /* pointer to polynomial coefficients */

    switch (degree)
    {
        case 1:
            val = a[0] + a[1] * x + a[2] * y + a[3] * x * y;
            break;
        case 2:
            x2 = x * x;
            y2 = y * y;
            val = a[0] + a[1] * x + a[2] * x2 + a[3] * y + a[4] * x * y +
                a[5] * x2 * y + a[6] * y2 + a[7] * x * y2 + a[8] * x2 * y2;
            break;
        case 3:
            x2 = x * x;
            y2 = y * y;
            x3 = x2 * x;
            y3 = y2 * y;
            val = a[0] + a[1] * x + a[2] * x2 + a[3] * x3 + a[4] * y + 
                a[5] * x * y + a[6] * x2 * y + a[7] * x3 * y + a[8] * y2 +
                a[9] * x * y2 + a[10] * x2 * y2 + a[11] * x3 * y2 + 
                a[12] * y3 + a[13] * x * y3 + a[14] * x2 * y3 + a[15] * x3 * y3;
            break;
        case 4:
            coeff = a;
            for (i = 0; i <= degree; i++)
            {
                yp = pow(y,(double)i);
                for (m = 0; m <= degree; m++, coeff++)
                {
                    xp = pow(x,(double)m);
                    val += xp*yp*(*coeff);
                }
            }
            break;
        default:
            /* This should never happen, so make it a catastrophic error */
            IAS_LOG_ERROR("Unsupported degree number: %d", degree);
            exit(EXIT_FAILURE);
    }
    return val;
}
