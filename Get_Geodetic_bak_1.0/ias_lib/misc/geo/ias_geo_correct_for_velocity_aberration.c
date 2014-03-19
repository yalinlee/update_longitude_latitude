/******************************************************************************
NAME: ias_geo_correct_for_velocity_aberration

PURPOSE:
Correct LOS for relativistic velocity aberration.  LOS is adjusted for 
apparent deflection caused by relative velocity of platform (spacecraft)
and target (Earth or Moon).

RETURNS:
    SUCCESS or ERROR
******************************************************************************/
#include <math.h>
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_geo.h"

int ias_geo_correct_for_velocity_aberration
(
   const IAS_VECTOR *satpos,    /* I: Satellite position (meters) */
   const IAS_VECTOR *satvel,    /* I: Satellite velocity (meters/sec) */
   IAS_ACQUISITION_TYPE acq_type, /* I: Image acquisition type */
   const IAS_EARTH_CHARACTERISTICS *earth, /* I: Earth parameters */
   const IAS_VECTOR *clos,      /* I: Input LOS vector */ 
   IAS_VECTOR *vlos             /* O: New LOS vector adjusted for aberration */
)
{
    IAS_VECTOR groundv;      /* Ground velocity vector */
    IAS_VECTOR groundpt;     /* Ground location vector */
    IAS_VECTOR earthv;       /* Earth rotation vector */
    double tarelev = 0.0;    /* Target elevation */
    double tarlatc;          /* Target geocentric latitude (radians)*/
    double tarlong;          /* Target longitude (radians (radians)*/
    double tarrad;           /* Radius of the earth at the target  */
    IAS_VECTOR nlos;         /* New corrected LOS */            
    double m;                /* Magnitude of new LOS vector */

    /* Find the position where the line of sight vector intersects the Earth's
     * surface */
    if (acq_type == IAS_EARTH)
    {
        if (ias_geo_find_target_position(satpos, clos, earth, tarelev, 
                    &groundpt, &tarlatc, &tarlong, &tarrad) != SUCCESS)
        {
            IAS_LOG_ERROR("Failed calculating target position");
            return ERROR;
        }

        /* Calculate ground velocity from Earths rotational velocity
           and ground point location. */
        earthv.x = 0.0;
        earthv.y = 0.0;
        earthv.z = earth->earth_angular_velocity;
        ias_math_compute_3dvec_cross(&earthv,&groundpt,&groundv);

        /* Calculate new LOS vector */
        nlos.x = clos->x - ((satvel->x - groundv.x) / earth->speed_of_light);
        nlos.y = clos->y - ((satvel->y - groundv.y) / earth->speed_of_light);
        nlos.z = clos->z - ((satvel->z - groundv.z) / earth->speed_of_light);

        m = ias_math_compute_3dvec_dot(&nlos,&nlos);
        m = sqrt(m);
        vlos->x = nlos.x / m; 
        vlos->y = nlos.y / m; 
        vlos->z = nlos.z / m; 
    }
    else /* LOS is stellar or Lunar */
    {
        /* Calculate new LOS vector */
        nlos.x = clos->x - (satvel->x / earth->speed_of_light);
        nlos.y = clos->y - (satvel->y / earth->speed_of_light);
        nlos.z = clos->z - (satvel->z / earth->speed_of_light);

        m = ias_math_compute_3dvec_dot(&nlos,&nlos);
        vlos->x = nlos.x / m;
        vlos->y = nlos.y / m;
        vlos->z = nlos.z / m;
    }

    return SUCCESS;
}
