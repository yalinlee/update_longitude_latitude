/****************************************************************************
NAME:    ias_geo_transform_j2k2tod

PURPOSE: Transform inertial cartesian coordinates (ECI of epoch J2000) to
         true-of-date inertial cartesian coordinates (ECITOD) at the
         specified UTC time

RETURNS: SUCCESS or ERROR

*****************************************************************************/
#include "ias_logging.h"
#include "ias_geo.h"

int ias_geo_transform_j2k2tod
(
    double ut1_utc,               /* I: UT1-UTC, in seconds, due to variation
                                        of Earth's spin rate */
    const IAS_VECTOR *ecij2k_pos, /* I: Satellite position in ECIJ2K */
    const double ephem_time[3],   /* I: Time of the ephemeris point,
                                        in year, doy and sod */
    IAS_VECTOR *ecitod_pos        /* O: Satellite position in ECITOD */
)
{
    double jd_tdb;          /* TDB Julian date of ephemeris time */
    double jd_tt;           /* TT Julian date of ephemeris time */
    double jd_ut1;          /* UT1 Julian date of ephemeris time */
    IAS_VECTOR ecimod_pos;  /* ECI mean of date position after precession */

    /* Convert the input time into the different time standards needed */
    if (ias_geo_convert_utc2times(ut1_utc, ephem_time, &jd_ut1, &jd_tdb,
        &jd_tt) != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to convert UTC time to other time standards");
        return ERROR;
    }

    /* Precession transformation converts ECI of epoch J2000 to ECI mean of
       date */
    if (ias_geo_transform_precession_j2k2mod(ecij2k_pos, jd_tdb, &ecimod_pos)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Failed performing the precession tranformation");
        return ERROR;
    }

    /* Nutation transformation converts ECI mean of date to ECI true of date */
    ias_geo_transform_nutation_mod2tod(&ecimod_pos, jd_tdb, ecitod_pos);

    return SUCCESS;
}
