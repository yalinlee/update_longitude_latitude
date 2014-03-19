/****************************************************************************
NAME: ias_geo_transform_ecef2j2k

PURPOSE: Transform Earth-fixed cartesian coordinates (ECEF WGS84) to inertial
    cartesian coordinates (ECI of epoch J2000) at the specified GMT (UTC) time.

DEVELOPMENT HISTORY:
Derived from calval geo_ecf2j2k

RETURNS: SUCCESS or ERROR

*****************************************************************************/
#include "ias_logging.h"
#include "ias_geo.h"

int ias_geo_transform_ecef2j2k
(
    double xp, /* I: Earth's true pole offset from mean pole, in arc second */
    double yp, /* I: Earth's true pole offset from mean pole, in arc second */
    double ut1_utc, /* I: UT1-UTC, in seconds, due to variation of Earth's 
                          spin rate */
    const IAS_VECTOR *craft_pos, /* I: Satellite position in ECEF */
    const IAS_VECTOR *craft_vel, /* I: Satellite velocity in ECEF */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_VECTOR *eci_satpos,      /* O: Satellite position in ECI */
    IAS_VECTOR *eci_satvel       /* O: Satellite velocity in ECI */
)
{
    IAS_VECTOR ecimod_pos;  /* ECI mean of date position after precession */
    IAS_VECTOR ecitod_pos;  /* ECI true of date position after nutation */
    IAS_VECTOR ecftod_pos;  /* ECEF true of date position after sidereal time */
    IAS_VECTOR ecimod_vel;  /* ECI mean of date velocity after precession */
    IAS_VECTOR ecitod_vel;  /* ECI true of date velocity after nutation */
    IAS_VECTOR ecftod_vel;  /* ECEF true of date velocity after sidereal time */
    double jd_tdb;          /* TDB Julian date of ephemeris time */
    double jd_tt;           /* TT Julian date of ephemeris time */
    double jd_ut1;          /* UT1 Julian date of ephemeris time */
 

    /* Convert the input time into the different time standards needed */
    if (ias_geo_convert_utc2times(ut1_utc, ephem_time, &jd_ut1, &jd_tdb,
        &jd_tt) != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to convert UTC time to other time standards");
        return ERROR;
    }

    /* Polar motion correction converts mean WGS84 ECEF to ECEF true of date
       from mean pole to true pole   */
    ias_geo_transform_polar_motion_mean_pole_to_true(craft_pos, xp, yp, jd_tdb,
        &ecftod_pos);
    ias_geo_transform_polar_motion_mean_pole_to_true(craft_vel, xp, yp, jd_tdb,
        &ecftod_vel);

    /* Sidereal time rotation converts ECEF true of date to ECI true of date */
    if (ias_geo_transform_sidereal_ecef2eci(&ecftod_pos, &ecftod_vel, ut1_utc,
        ephem_time, &ecitod_pos, &ecitod_vel) != SUCCESS)
    {
        IAS_LOG_ERROR("Failed converting ECEF true-of-date to ECI");
        return ERROR;
    }

    /* Nutation transformation converts ECI true of date to ECI mean of date */
    ias_geo_transform_nutation_tod2mod(&ecitod_pos, jd_tdb, &ecimod_pos);
    ias_geo_transform_nutation_tod2mod(&ecitod_vel, jd_tdb, &ecimod_vel);

    /* Precession transformation converts ECI mean of date to ECI of epoch
       J2000 */
    if (ias_geo_transform_precession_mod2j2k(&ecimod_pos, jd_tdb, eci_satpos)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Failed performing the precession tranformation");
        return ERROR;
    }

    if (ias_geo_transform_precession_mod2j2k(&ecimod_vel, jd_tdb, eci_satvel)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Failed performing the precession tranformation");
        return ERROR;
    }

    return SUCCESS;
}
