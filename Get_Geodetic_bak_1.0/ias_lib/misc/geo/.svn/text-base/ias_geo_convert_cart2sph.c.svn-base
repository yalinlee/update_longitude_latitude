/*******************************************************************************
NAME: ias_geo_convert_cart2sph

PURPOSE: 
Convert Cartesian coordinates to spherical coordinates

RETURN VALUE:
Type = int (SUCCESS or ERROR)

*******************************************************************************/
#include <math.h>
#include "ias_structures.h"
#include "ias_logging.h"
#include "ias_geo.h"

int ias_geo_convert_cart2sph
(
    const IAS_VECTOR *vec, /* I: Vector containing Cartesian coords */
    double *lat,           /* O: Latitude in spherical coordinates */
    double *longs,         /* O: Longitude in spherical coordinates */
    double *radius         /* O: Distance from origin to the point */
)
{
    /* Calculate the radius (length of the vector from the origin) */
    *radius = ias_math_compute_vector_length(vec);

    /* Make sure we aren't going to divide by zero (i.e. were given a bad
       input vector) */
    if (*radius == 0.0)
    {
        IAS_LOG_ERROR("Invalid zero length vector provided");
        return ERROR;
    }

    /* Calculate the latitude spherical coordinate */
    *lat = asin(vec->z / *radius);

    /* Calculate the longitude spherical coordinate.  The longitude depends on
       the quadrant that the vector lies in.  */
    *longs = atan2(vec->y, vec->x);

    return SUCCESS;
}
