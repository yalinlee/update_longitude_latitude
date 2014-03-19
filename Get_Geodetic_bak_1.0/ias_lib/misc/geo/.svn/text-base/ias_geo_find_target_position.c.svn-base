/******************************************************************************
Name: ias_geo_find_target_position

Purpose: Find the position where the line of sight vector intersects the Earth's
    surface.

Returns:
SUCCESS or ERROR

******************************************************************************/
#include <math.h>
#include "ias_math.h"
#include "ias_logging.h"
#include "ias_geo.h"

#define NUM_ITERATIONS 10   /* number of iterations for converging
                               LOS intersection */
#define CONV_TOLERANCE 0.01 /* convergence tolerance */

int ias_geo_find_target_position
(
    const IAS_VECTOR *satpos, /* I: Satellite location vector */
    const IAS_VECTOR *losv,   /* I: Line of sight to target vector */
    const IAS_EARTH_CHARACTERISTICS *earth, /* I: Earth parameters */
    double tarelev,         /* I: Elevation of target above the ellipsoid */
    IAS_VECTOR *tarvec,     /* O: Target vector */
    double *tarlat,         /* O: Target latitude */
    double *tarlong,        /* O: Target longitude */
    double *tarrad          /* O: Radius of the target */
)
{
    IAS_VECTOR u;          /* Scaled LOS vector */
    IAS_VECTOR p;          /* Scaled satellite position vector */
    IAS_VECTOR n;          /* Unit vector normal to surface */
    double up;             /* u dot p vector */
    double pp;             /* p dot p vector (satpos vector length squared) */
    double uu;             /* u dot u vector (LOS vector length squared) */   
    double d;              /* LOS adjustment */
    double q;              /* u dot n vector */
    double dh;             /* Delta height */
    double height;         /* Height of target */
    double tarlatd;        /* Target geodetic latitude */
    double dist_x,dist_y,dist_z;
    double temp;
    int iter;              /* Iteration */

    u.x = losv->x / earth->semi_major_axis;
    u.y = losv->y / earth->semi_major_axis;
    u.z = losv->z / earth->semi_minor_axis;

    p.x = satpos->x / earth->semi_major_axis;
    p.y = satpos->y / earth->semi_major_axis;
    p.z = satpos->z / earth->semi_minor_axis;

    up = ias_math_compute_3dvec_dot(&u,&p);

    /* Get the squared length of the LOS and satellite position vectors. */
    uu = ias_math_compute_3dvec_dot(&u,&u);
    pp = ias_math_compute_3dvec_dot(&p,&p);

    /* if the sensor is not viewing the earth, return an error.  The sensor is
       not viewing the earth if up is > 0 (viewing within +/- 90 degrees of
       directly away from the earth) or temp < 0 (in the direction of the earth
       but off to one side or the other for a stellar collect) */
    temp = up * up - (uu * (pp - 1.0));
    if ((up > 0.0) || (temp < 0.0))
    {
        IAS_LOG_ERROR("LOS does not target Earth");
        return ERROR;
    }

    /* Solve for distance of LOS vector to target */
    d = (-up - sqrt(up * up - (uu * (pp - 1.0)))) / uu;

    /* Find target vector for current estimate */
    tarvec->x = (p.x + d * u.x) * earth->semi_major_axis;
    tarvec->y = (p.y + d * u.y) * earth->semi_major_axis;
    tarvec->z = (p.z + d * u.z) * earth->semi_minor_axis;

    if (ias_geo_convert_cart2sph(tarvec, tarlat, tarlong, tarrad) != SUCCESS)
    {
        IAS_LOG_ERROR("Failed to convert cartesian coordinates to spherical");
        return ERROR;
    }

    /* If not looking for Earth ellipsoid answer */
    if (tarelev != 0.0)
    {
        p.x = satpos->x;
        p.y = satpos->y;
        p.z = satpos->z;

        u.x = losv->x;
        u.y = losv->y;
        u.z = losv->z;

        iter = 0;

        if (ias_geo_convert_geocentric_height_to_geodetic(*tarlat, *tarrad, 
                    earth, &tarlatd, &height) != SUCCESS)
        {
            IAS_LOG_ERROR("In targeting");
            return ERROR;
        }

        for(;;)
        {
            dh = height - tarelev;
            if (fabs(dh) < CONV_TOLERANCE)
                break;

            dist_x = tarvec->x - p.x;
            dist_y = tarvec->y - p.y;
            dist_z = tarvec->z - p.z;
            d = sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);

            /* Calculate normal to surface */
            n.x = cos(tarlatd) * cos(*tarlong);
            n.y = cos(tarlatd) * sin(*tarlong);
            n.z = sin(tarlatd);

            q = -ias_math_compute_3dvec_dot(&u,&n);

            /* Update length of LOS vector */
            d = d + dh / q;

            /* Re-adjust target vector based on new distance */
            tarvec->x = p.x + d * u.x;
            tarvec->y = p.y + d * u.y;
            tarvec->z = p.z + d * u.z;

            if (ias_geo_convert_cart2sph(tarvec, tarlat, tarlong, tarrad)
                != SUCCESS)
            {
                IAS_LOG_ERROR("Failed to convert cartesian coordinates to "
                    "spherical");
                return ERROR;
            }

            if (ias_geo_convert_geocentric_height_to_geodetic(*tarlat, *tarrad,
                    earth, &tarlatd, &height) != SUCCESS)
            {
                IAS_LOG_ERROR("Failed to convert geocentric height to "
                              "geodetic");
                return ERROR;
            }
            if (iter > NUM_ITERATIONS)
            {
                IAS_LOG_ERROR("Exceeded %d iterations without converging to a "
                              "solution", NUM_ITERATIONS);
                IAS_LOG_DEBUG("Height %f tarelev %f", height, tarelev);
                return ERROR;
            }
            iter++;
        }
    }

    return SUCCESS;
}
