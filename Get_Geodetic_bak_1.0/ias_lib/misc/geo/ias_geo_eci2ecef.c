/****************************************************************************
NAME: ias_geo_eci2ecef

PURPOSE:
Transform Earth-centered inertial Cartesian coordinates (ECI/true-of-date)
to Earth-centered, Earth-fixed Cartesian coordinates (ECEF) at the
specified GMT (UTC) time.

RETURNS: SUCCESS or ERROR

*****************************************************************************/
#include "ias_const.h"
#include "ias_structures.h"
#include "ias_math.h"
#include "ias_logging.h"
#include "ias_geo.h"
#include "local_novas_wrapper.h"

int ias_geo_eci2ecef
(
    double xp, /* I: Earth's true pole offset from mean pole, in arc second */
    double yp, /* I: Earth's true pole offset from mean pole, in arc second */
    double ut1_utc, /* I: UT1-UTC, in seconds, due to variation of Earth's 
                          spin rate */
    const IAS_VECTOR *craft_pos, /* I: Satellite position in ECI */
    const IAS_VECTOR *craft_vel, /* I: Satellite velocity in ECI */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_VECTOR *fe_satpos, /* O: Satellite position in ECEF */
    IAS_VECTOR *fe_satvel  /* O: Satellite velocity in ECEF */
)
{
    double gast;            /* Greenwich apparent sidereal time, in rad */
    IAS_VECTOR eci_vec; /* vector in ECI system */
    IAS_VECTOR pre_vec; /* vector after precession transformation */
    IAS_VECTOR nut_vec; /* vector after nutation transformation */
    IAS_VECTOR mid_vec; /* vector in intermediate step */
    IAS_VECTOR ecr_vec; /* vector in ECEF system */
    double jd_tdb;      /* Barycentric (TDB) Julian date of ephem_time */
    double jd_tt;       /* Terrestrial Julian date of ephem_time */
    double jd_ut1;      /* UT1 Julian date of ephem_time */

    /* Convert the input time into the different time standards needed */
    if (ias_geo_convert_utc2times(ut1_utc, ephem_time, &jd_ut1, &jd_tdb,
        &jd_tt) != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to convert UTC time to other time standards");
        return ERROR;
    }

    /* Get the Greenwich apparent sidereal time */
    if (ias_geo_get_sidereal_time(jd_ut1, jd_tt, &gast) != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to get Greenwich sidereal time");
        return ERROR;
    }

    /* Convert the satellite position vector from ECI to ECEF */
    eci_vec.x = craft_pos->x; /* copy from the input vector */
    eci_vec.y = craft_pos->y;
    eci_vec.z = craft_pos->z;

    /* ACS ephemeris data is in the J2000.0 system, precession and nutation
       transformation should be added here before the rotation transformation
       around Earth's spin axis */
    if (ias_geo_transform_precession_j2k2mod(&eci_vec, jd_tdb, &pre_vec)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Failed performing the precession tranformation");
        return ERROR;
    }

    ias_geo_transform_nutation_mod2tod(&pre_vec, jd_tdb, &nut_vec);
    eci_vec.x = nut_vec.x;
    eci_vec.y = nut_vec.y;
    eci_vec.z = nut_vec.z;

    /* rotate around true pole for gast */
    ias_math_rotate_3dvec_around_z(&eci_vec, gast, &mid_vec);
    ias_geo_transform_polar_motion_true_pole_to_mean(&mid_vec, xp, yp, jd_tdb,
        &ecr_vec);
    fe_satpos->x = ecr_vec.x;
    fe_satpos->y = ecr_vec.y;
    fe_satpos->z = ecr_vec.z;

    /* Convert the satellite velocity vector from ECI to ECEF, no magnitude 
       change */
    eci_vec.x = craft_vel->x; /* copy vector from input */
    eci_vec.y = craft_vel->y;
    eci_vec.z = craft_vel->z;

    /* ACS ephemeris data is in the J2000.0 system, precession and nutation
       transformation should be added here before the rotation transformation
       around Earth's spin axis */
    if (ias_geo_transform_precession_j2k2mod(&eci_vec, jd_tdb, &pre_vec)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Failed performing the precession tranformation");
        return ERROR;
    }

    ias_geo_transform_nutation_mod2tod(&pre_vec, jd_tdb, &nut_vec);
    eci_vec.x = nut_vec.x;
    eci_vec.y = nut_vec.y;
    eci_vec.z = nut_vec.z;
   
    /* rotation around true pole   */
    ias_math_rotate_3dvec_around_z(&eci_vec, gast, &mid_vec);

    /* from true pole to mean pole */
    ias_geo_transform_polar_motion_true_pole_to_mean(&mid_vec, xp, yp, jd_tdb,
        &ecr_vec);
    fe_satvel->x = ecr_vec.x;
    fe_satvel->y = ecr_vec.y;
    fe_satvel->z = ecr_vec.z;

    return SUCCESS;
}
