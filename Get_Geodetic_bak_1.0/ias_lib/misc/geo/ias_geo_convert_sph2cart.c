/****************************************************************************
NAME: ias_geo_convert_sph2cart

PURPOSE: Convert spherical coordinates to Cartesian coordinates

RETURNS: void

*****************************************************************************/
#include <math.h>
#include "ias_structures.h"
#include "ias_geo.h"

void ias_geo_convert_sph2cart
(
    double latp,     /* I: Latitude in spherical coordinates */
    double longp,    /* I: Longitude in spherical coordinates */
    double radius,   /* I: Distance from origin to the point */
    IAS_VECTOR *vec  /* O: Vector containing Cartesian coords */
)
{
    double rcoslt; /* Radius times cosine of the latitude constant */

    /* Calculate a constant to speed up calculations */
    rcoslt = radius * cos(latp);

    /* Calculate the Cartesian coordinates for the vector */
    vec->x = rcoslt * cos(longp);
    vec->y = rcoslt * sin(longp);
    vec->z = radius * sin(latp);
}
