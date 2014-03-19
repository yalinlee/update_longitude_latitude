/******************************************************************************
Name: ias_math_constants

Purpose: Provides routines to calculate and return mathematical constants 
    such as Pi.

******************************************************************************/
#include <math.h>
#include "ias_math.h"


/******************************************************************************
Name: ias_math_get_pi

Purpose: Returns the calculated value of Pi.

Returns: The value of Pi

******************************************************************************/
double ias_math_get_pi()
{
    static double pi;
    static int initialized = 0;

    if (!initialized)
    {
        initialized = 1;
        pi = 4.0 * atan(1.0);
    }

    return pi;
}

/******************************************************************************
Name: ias_math_get_arcsec_to_radian_conversion

Purpose: Returns the calculated value for converting arc seconds to radians.

Returns: The value of arc2rad

******************************************************************************/
double ias_math_get_arcsec_to_radian_conversion()
{
    static int initialized = 0;
    static double arc2rad;

    if (!initialized)
    {
        arc2rad = ias_math_get_pi() / (180.0 * 3600.0);

        initialized = 1;
    }

    return arc2rad;
}

/******************************************************************************
Name: ias_math_get_radians_per_degree

Purpose: Returns the calculated value of radians per degree.

Returns: The value of radians per degree

******************************************************************************/
double ias_math_get_radians_per_degree()
{
    static double radians_per_degree;
    static int initialized = 0;

    if (!initialized)
    {
        initialized = 1;
        radians_per_degree = ias_math_get_pi() / 180.0;
    }

    return radians_per_degree;
}

/******************************************************************************
Name: ias_math_get_degrees_per_radian

Purpose: Returns the calculated value of degrees per radian.

Returns: The value of degrees per radian

******************************************************************************/
double ias_math_get_degrees_per_radian()
{
    static double degrees_per_radian;
    static int initialized = 0;

    if (!initialized)
    {
        initialized = 1;
        degrees_per_radian = 180.0 / ias_math_get_pi();
    }

    return degrees_per_radian;
}
