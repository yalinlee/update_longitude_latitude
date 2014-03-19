/****************************************************************************
NAME: ias_geo_transform_sidereal

PURPOSE: Transform inertial cartesian coordinates between ECI and ECEF
    true-of-date by applying Earth rotation effects.

*****************************************************************************/
#include <math.h>
#include "ias_logging.h"
#include "ias_geo.h"
#include "local_novas_wrapper.h"

/****************************************************************************
NAME: ias_geo_get_sidereal_time

PURPOSE: Calculate the Greenwich apparent sidereal time (GAST) for given UT1
    and Terrestrial (TT) times.

RETURNS: SUCCESS or ERROR

*****************************************************************************/
int ias_geo_get_sidereal_time
(
    double jd_ut1,        /* I: UT1 Julian date of ephemeris time */
    double jd_tt,         /* I: TT Julian date of ephemeris time */
    double *gast          /* O: Greenwich apparent sidereal time, in rad */
)
{
    int status;
    double gast_hrs;   /* Greenwich apparent sidereal time (GAST), in hours */ 
    double delta_t;    /* Seconds difference between TT and UT1 times */
    double hours2radians = 15.0 * ias_math_get_radians_per_degree();
        /* The conversion factor to convert the gast from hours to radians.
           There are 15 degrees per hour. */

    /* Calculate the GAST at the ephemeris time and convert it to radians */
    delta_t = (jd_tt - jd_ut1) * IAS_SEC_PER_DAY;
    status = NOVAS_SIDEREAL_TIME(jd_ut1, 0.0, delta_t, NOVAS_APPARENT_GAST,
        NOVAS_EQUINOX_METHOD, NOVAS_FULL_ACCURACY, &gast_hrs);
    if (status != NOVAS_SUCCESS)
    {
        IAS_LOG_ERROR("NOVAS sidereal_time routine returned error code of %d",
            status);
        return ERROR;
    }
    *gast = hours2radians * gast_hrs;

    return SUCCESS;
}

/****************************************************************************
NAME: get_omega_star_and_gast

PURPOSE: Compute the Earth rotation rate in precessing frame and the 
    Greenwich apparent sidereal time at a given UTC time and irregular Earth
    rotation.  This is a helper routine for the other two routines here.

RETURNS: SUCCESS or ERROR

*****************************************************************************/
static int get_omega_star_and_gast
(
    double ut1_utc,        /* I: UT1-UTC, in seconds, due to variation of
                                 Earth's spin rate */
    const double ephem_time[3],/* I: Time of ephemeris point (year, doy, sec)*/
    double *omega_star,    /* O: Earth rotation rate in precessing frame, in
                                 rad/sec */
    double *gast           /* O: Greenwich apparent sidereal time, in rad */
)
{
    double gast_at_delta; /* Greenwich apparent siderial time, in rad */
    double delta = 1.0;   /* Seconds between gast and gast_at_delta */
    double delta_days = delta / IAS_SEC_PER_DAY;
        /* Days between gast and gast_at_delta */
    double jd_tdb;        /* TDB Julian date of ephemeris time */
    double jd_tt;         /* TT Julian date of ephemeris time */
    double jd_ut1;        /* UT1 Julian date of ephemeris time */

    /* Convert the input time into the different time standards needed */
    if (ias_geo_convert_utc2times(ut1_utc, ephem_time, &jd_ut1, &jd_tdb,
        &jd_tt) != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to convert UTC time to other time standards");
        return ERROR;
    }

    /* Calculate the GAST at the ephemeris time and convert it to radians */
    if (ias_geo_get_sidereal_time(jd_ut1, jd_tt, gast) != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to get Greenwich sidereal time");
        return ERROR;
    }

    /* Calculate the GAST at delta seconds later */
    if (ias_geo_get_sidereal_time(jd_ut1 + delta_days, jd_tt + delta_days,
        &gast_at_delta) != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to get Greenwich sidereal time");
        return ERROR;
    }

    /* Calculate omega_star as the rate of change between the two GAST 
       values */
    *omega_star = (gast_at_delta - *gast) / delta;

    return SUCCESS;
}

/****************************************************************************
NAME: ias_geo_transform_sidereal_eci2ecef

PURPOSE: Transform inertial cartesian coordinates from (ECI/true-of-date) to
    Earth-fixed cartesian coordinates (ECEF/true-of-date) at the specified GMT
    (UTC) time by applying Earth rotation effects.

RETURNS: SUCCESS or ERROR

*****************************************************************************/
int ias_geo_transform_sidereal_eci2ecef
(
    const IAS_VECTOR *craft_pos,/* I: Satellite position in ECI */ 
    const IAS_VECTOR *craft_vel,/* I: Satellite velocity in ECI */
    double ut1_utc,             /* I: UT1-UTC, in seconds, due to variation of
                                       Earth's spin rate */
    const double ephem_time[3], /* I: Time of ephemeris point (year, doy, sec)*/
    IAS_VECTOR *ecef_satpos,    /* O: Satellite position in ECEF */
    IAS_VECTOR *ecef_satvel     /* O: Satellite velocity in ECEF */
)
{
    IAS_VECTOR ecftod_vel;  /* velocity vector in ECFTOD system */
    double gast;        /* Greenwich apparent sidereal time, in rad */
    double omega_star;  /* Earth rotation rate in precessing frame, in
                           rad/sec */

    /* Get the omega_star and gast values for this time */
    if (get_omega_star_and_gast(ut1_utc, ephem_time, &omega_star, &gast)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Failed calculating the Omega Star and Gast values");
        return ERROR;
    }

    /* Convert the satellite position vector from ECI to ECF - rotate around
       true pole for gast*/
    ias_math_rotate_3dvec_around_z(craft_pos, gast, ecef_satpos);
    ias_math_rotate_3dvec_around_z(craft_vel, gast, &ecftod_vel);

    /* Apply correction to velocity based on rate of change of coordinate 
       frame */
    ecftod_vel.x += (-omega_star * sin(gast) * craft_pos->x
                    + omega_star * cos(gast) * craft_pos->y);
    ecftod_vel.y += (-omega_star * cos(gast) * craft_pos->x
                    - omega_star * sin(gast) * craft_pos->y);

    ecef_satvel->x = ecftod_vel.x;        /* copy to the output vector */
    ecef_satvel->y = ecftod_vel.y;
    ecef_satvel->z = ecftod_vel.z;

    return SUCCESS;
}

/****************************************************************************
NAME: ias_geo_transform_sidereal_ecef2eci

PURPOSE: Transform from Earth-fixed cartesian coordinates (ECEF/true-of-date)
    to inertial cartesian coordinates (ECI/true-of-date) at the specified GMT
    (UTC) time by applying Earth rotation effects.

RETURNS: SUCCESS or ERROR

*****************************************************************************/
int ias_geo_transform_sidereal_ecef2eci
(
    const IAS_VECTOR *craft_pos, /* I: Satellite position in ECEF */ 
    const IAS_VECTOR *craft_vel, /* I: Satellite velocity in ECEF */
    double ut1_utc,              /* I: UT1-UTC, in seconds, due to variation of
                                       Earth's spin rate */
    const double ephem_time[3],/* I: Time of ephemeris point (year, doy, sec) */
    IAS_VECTOR *eci_satpos, /* O: Satellite position in ECI */
    IAS_VECTOR *eci_satvel  /* O: Satellite velocity in ECI */
)
{
    IAS_VECTOR ecftod_vel;  /* velocity vector in ECFTOD system */
    double gast;        /* Greenwich apparent sidereal time, in rad */
    double omega_star;  /* Earth rotation rate in precessing frame, in
                           rad/sec */

    /* Get the omega_star and gast values for this time */
    if (get_omega_star_and_gast(ut1_utc, ephem_time, &omega_star, &gast)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Failed calculating the Omega Star and Gast values");
        return ERROR;
    }

    /* Convert the satellite position vector from ECF to ECI - rotate around
       true pole for gast*/
    ias_math_rotate_3dvec_around_z(craft_pos, -gast, eci_satpos);
    ecftod_vel.x = craft_vel->x;
    ecftod_vel.y = craft_vel->y;
    ecftod_vel.z = craft_vel->z;

    /* Apply correction to velocity based on rate of change of coordinate
       frame */
    ecftod_vel.x -= (-omega_star * sin(gast) * eci_satpos->x
                    + omega_star * cos(gast) * eci_satpos->y);
    ecftod_vel.y -= (-omega_star * cos(gast) * eci_satpos->x
                    - omega_star * sin(gast) * eci_satpos->y);

    /* rotate around true pole for gast */
    ias_math_rotate_3dvec_around_z(&ecftod_vel, -gast, eci_satvel);

    return SUCCESS;
}
