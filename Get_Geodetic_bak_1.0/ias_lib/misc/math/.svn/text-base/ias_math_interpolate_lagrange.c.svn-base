/******************************************************************************
NAME: ias_math_interpolate_lagrange 

PURPOSE: Calculates the interpolation of the requested time

RETURN VALUE:
    Type = double
******************************************************************************/
#include "ias_math.h"

double ias_math_interpolate_lagrange
( 
    const double *p_YY, /* I: Pointer to the array of input values */
    const double *p_XX, /* I: Pointer to the array of times closest to the
                              requested time */
    int n_pts,          /* I: Number of points for the interpolation */
    double in_time      /* I: Requested time for interpolation */
)
{
    double term;
    double sum;
    int i, j;

    sum = 0.0;
    for (i = 0; i < n_pts; i++)
    {
        term = p_YY[i];
        for (j = 0; j < n_pts; j++)
        {
            if (j != i)
                term = term*(in_time - p_XX[j])/(p_XX[i] - p_XX[j]);
        }
        sum = sum + term;
    }

    return sum;
}
