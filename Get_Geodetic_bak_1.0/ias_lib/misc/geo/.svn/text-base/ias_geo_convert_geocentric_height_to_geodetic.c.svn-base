/*******************************************************************************
NAME: ias_geo_convert_geocentric_height_to_geodetic

PURPOSE: Calculate the geodetic latitude and height of a point above the 
         ellipse, given the geocentric latitude and the radius  

RETURN VALUE:
Type = int 
Value           Description
-----           -----------
SUCCESS         Latitude and height calculated
ERROR           Error calculating satellite height

ALGORITHM REFERENCES:
        ESCOBAL, METHODS OF ORBIT DETERMINATION (page 398)

*******************************************************************************/
#include <math.h>
#include "ias_logging.h"
#include "ias_geo.h"
#include "ias_cpf.h"

#define EPSILON 0.0001          /* tolerance to terminate height calculation */

int ias_geo_convert_geocentric_height_to_geodetic
(
    double latc,          /* I: Geocentric latitude (radians) */
    double radius,        /* I: Radius of the point (meters) */
    const IAS_EARTH_CHARACTERISTICS *earth, /* I: earth constants */
    double *latd,         /* O: Geodetic latitude (radians) */
    double *height        /* O: Height of the point (meters) */
)
{
    double rsq;         /* Radius of the point squared */
    double radius1;     /* Temporary radius */
    double radius2;     /* Calculated radius of the point */
    double r2sq;        /* Calculated radius of the point squared */
    double height2;     /* Height of the satellite */
    double phis;        /* Latitude of the satellite */
    double phip;        /* Latitude of the point */
    double dphip;       /* Delta latitude of the point */
    double dphis;       /* Delta latitude of the satellite */

    double last_height = 0.0; /* Previous height of the satellite */
    int i;              /* Looping variable */

    /* Check for invalid radius */
    if (radius <= 0.0)
    {
        IAS_LOG_ERROR("Invalid point radius %f is not greater than zero",
            radius);
        return ERROR;
    }

    /* Save values for calculations */
    phip = latc;
    rsq  = radius * radius;

    /* Iterate for a solution a maximum of 20 times.  If it doesn't find a
       solution with the wanted tolerance, it will use the last result. */
    for (i = 0; i < 20; i++)
    {
        /* Calculate the radius at the target */
        if (ias_geo_find_earth_radius(phip, earth, &radius2) == ERROR)
        {
            IAS_LOG_ERROR("Failed finding the Earth radius");
            return ERROR;
        }
        r2sq = radius2 * radius2;

        /* Calculate the latitude of the satellite */
        phis = atan2(tan(phip), (1.0 - earth->eccentricity));
        dphis = phis - phip;

        /* Calculate the height of the satellite */
        radius1 = rsq - r2sq * sin(dphis) * sin(dphis);
        if (radius1 < 0.0)
        {
            IAS_LOG_ERROR("Calculated invalid radius of %f (should not be "
                          "negative)", radius1);
            return ERROR;
        }
        else
            height2 = sqrt(radius1) - radius2 * cos(dphis);

        /* Check the tolerance */
        if (fabs(height2 - last_height) <= EPSILON)
            break;

        /* Save the current height */
        last_height = height2;

        /* Calculate the delta latitude correction for the new height */
        dphip = asin(height2 / radius * sin(dphis));
        phip = latc - dphip;
    }

    /* Return the new latitude and height */
    *latd = phis;
    *height = height2;

    return SUCCESS;
}
