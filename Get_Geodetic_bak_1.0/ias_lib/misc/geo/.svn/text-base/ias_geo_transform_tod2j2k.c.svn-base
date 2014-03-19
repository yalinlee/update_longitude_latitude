/****************************************************************************
NAME: ias_geo_transform_tod2j2k

PURPOSE: Transform true-of-date inertial cartesian coordinates (ECITOD) to 
    inertial cartesian coordinates (ECI of epoch J2000) at the specified GMT
    (UTC) time.

RETURNS: SUCCESS or ERROR

*****************************************************************************/
#include "ias_logging.h"
#include "ias_geo.h"

int ias_geo_transform_tod2j2k
(
    double ut1_utc,              /* I: UT1-UTC, in seconds, due to variation
                                       of Earth's spin rate */
    const IAS_VECTOR *ecitod_pos,/* I: Satellite position in ECITOD */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_VECTOR *ecij2k_pos       /* O: Satellite position in ECIJ2K */
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

    /* Convert the satellite position vector from ECITOD to ECIJ2K */

    /* Nutation transformation converts ECI true of date to ECI mean of date */
    ias_geo_transform_nutation_tod2mod(ecitod_pos, jd_tdb, &ecimod_pos);

    /* Precession transformation converts ECI mean of date to ECI of epoch
       J2000 */
    if (ias_geo_transform_precession_mod2j2k(&ecimod_pos, jd_tdb, ecij2k_pos)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Failed performing the precession tranformation");
        return ERROR;
    }

    return SUCCESS;
}
