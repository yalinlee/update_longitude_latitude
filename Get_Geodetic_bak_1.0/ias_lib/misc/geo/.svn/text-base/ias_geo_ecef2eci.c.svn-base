/****************************************************************************
NAME: ias_geo_ecef2eci

PURPOSE:
Transform Earth-centered, Earth-fixed Cartesian coordinates (ECEF) to
Earth-centered inertial Cartesian coordinates (ECI/true-of-date) at the
specified GMT (UTC) time.

RETURNS: SUCCESS or ERROR

*****************************************************************************/
#include "ias_structures.h"
#include "ias_math.h"
#include "ias_logging.h"
#include "ias_geo.h"

int ias_geo_ecef2eci
(
    double xp, /* I: Earth's true pole offset from mean pole, in arc second */
    double yp, /* I: Earth's true pole offset from mean pole, in arc second */
    double ut1_utc, /* I: UT1-UTC, in seconds, due to variation of Earth's 
                          spin rate */
    const IAS_VECTOR *craft_pos, /* I: Satellite position in ECEF */
    const IAS_VECTOR *craft_vel, /* I: Satellite velocity in ECEF */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_VECTOR *ic_satpos, /* O: Satellite position in ECI */
    IAS_VECTOR *ic_satvel  /* O: Satellite velocity in ECI */
)
{
    IAS_VECTOR ecr_x;   /* ECI X basis vector in ECEF system */
    IAS_VECTOR ecr_y;   /* ECI Y basis vector in ECEF system */
    IAS_VECTOR ecr_z;   /* ECI Z basis vector in ECEF system */
    IAS_VECTOR basis_x; /* ECI X basis vector */
    IAS_VECTOR basis_y; /* ECI Y basis vector */

    /* Construct the ECI basis vectors */
    basis_x.x = 1.0;
    basis_x.y = 0.0;
    basis_x.z = 0.0;
    basis_y.x = 0.0;
    basis_y.y = 1.0;
    basis_y.z = 0.0;

    /* Construct the ECEF basis vectors */
    if (ias_geo_eci2ecef(xp, yp, ut1_utc, &basis_x, &basis_y, ephem_time, 
        &ecr_x, &ecr_y) != SUCCESS)
    {
        IAS_LOG_ERROR("Failed to construct the ECEF basis vectors");
        return ERROR;
    }

    /* We can get the third axis from the cross product */
    ias_math_compute_3dvec_cross(&ecr_x, &ecr_y, &ecr_z);

    /* Convert the satellite position vector from ECEF to ECI
       using the basis vectors */
    ic_satpos->x = ias_math_compute_3dvec_dot(craft_pos, &ecr_x);
    ic_satpos->y = ias_math_compute_3dvec_dot(craft_pos, &ecr_y);
    ic_satpos->z = ias_math_compute_3dvec_dot(craft_pos, &ecr_z);

    /* Convert the satellite velocity vector from ECI to ECEF, no magnitude 
       change */
    ic_satvel->x = ias_math_compute_3dvec_dot(craft_vel, &ecr_x);
    ic_satvel->y = ias_math_compute_3dvec_dot(craft_vel, &ecr_y);
    ic_satvel->z = ias_math_compute_3dvec_dot(craft_vel, &ecr_z);

    return SUCCESS;
}
