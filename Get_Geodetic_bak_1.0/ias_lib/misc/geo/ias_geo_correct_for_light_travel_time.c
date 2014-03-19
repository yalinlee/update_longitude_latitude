/******************************************************************************
NAME: ias_geo_correct_for_light_travel_time

PURPOSE:
Correct target vector for light travel time.

RETURNS
    SUCCESS or ERROR
******************************************************************************/
#include <math.h>
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_geo.h"

int ias_geo_correct_for_light_travel_time
(
    const IAS_VECTOR *satpos,    /* I: Satellite position (meters) */
    const IAS_EARTH_CHARACTERISTICS *earth, /* I: Earth parameters */
    const IAS_VECTOR *itarvec,   /* I: Target vector to get adjusted */
    IAS_VECTOR *ltarvec,         /* O: Target vector adjusted for LLT */
    double *tarlat,              /* O: Target latitude */
    double *tarlong,             /* O: Target longitude */
    double *tarrad               /* O: Radius of the target */
)
{
    IAS_VECTOR d;    /* Distance between satellite and target */
    double m;        /* Magnitude of d vector */
    double llt;      /* Light travel time */
    double da;       /* Angle associated with llt */
    double cda;      /* cosine of da */
    double sda;      /* sine of da */

    /* Find vector between satellite position and target */
    d.x = satpos->x - itarvec->x;
    d.y = satpos->y - itarvec->y;
    d.z = satpos->z - itarvec->z;

    /* Find distance between satellite position and target */
    m = ias_math_compute_vector_length(&d);

    /* Calculate light travel time */
    llt = m / earth->speed_of_light;

    /* Calculate Earth rotation in llt */
    da = llt * earth->earth_angular_velocity;

    /* Calculate new target vector */
    cda = cos(da);
    sda = sin(da);

    ltarvec->x = cda * itarvec->x - sda * itarvec->y;
    ltarvec->y = sda * itarvec->x + cda * itarvec->y;
    ltarvec->z = itarvec->z;

    return ias_geo_convert_cart2sph(ltarvec, tarlat, tarlong, tarrad);
}
