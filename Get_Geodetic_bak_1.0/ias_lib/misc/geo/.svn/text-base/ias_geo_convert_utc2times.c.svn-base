/****************************************************************************
Name: ias_geo_convert_utc2times

Purpose: Convert UTC time to Universal Time (UT1), Terrestrial Time (TT), 
    and Barycentric Dynamical Time (TDB).

    TT  = UTC + Leap Seconds + TAI
    UT1 = UTC + (UT1_UTC)
    TT differs from TDB by peroidic variations

Returns: SUCCESS or ERROR

NOTES:
Reference: United States Naval Observatory Circular 180.

*****************************************************************************/
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_geo.h"
#include "local_novas_wrapper.h"

int ias_geo_convert_utc2times
(
    double ut1_utc,     /* I: UT1-UTC, in seconds, due to variation of Earth's
                              spin rate */
    const double ephem_time[3],/* I: Time year, day and seconds of day (UTC) */
    double *ut1,        /* O: Univeral Time (UT1) Time */
    double *tdb,        /* O: Barycentric Dynamical Time (TDB) */
    double *tt          /* O: Terrestrial Time (TT) */
)
{
    double j2000_seconds;/* Input time converted to J2000 seconds */
    double jd_tt;        /* Julian date (TT) for ephemeris time */
    double jd_tdb;       /* Julian date (TDB) for ephemeris time */
    double secdiff;      /* Difference between TDB and TT in seconds. */
    double jd_utc;       /* Julian date (UTC) at start of year */
    double dummy;

    /* Convert the spacecraft ephemeris time to seconds since J2000 */
    if (ias_math_convert_year_doy_sod_to_j2000_seconds(ephem_time,
        &j2000_seconds) != SUCCESS)
    {
        IAS_LOG_ERROR("Failed converting time to J2000 seconds");
        return ERROR;
    }

    /* Add the Terrestrial Time at noon of the J2000 epoch to the seconds
       since J2000 to get the total Terrestrial Time */
    jd_tt = IAS_EPOCH_2000 + j2000_seconds / IAS_SEC_PER_DAY;

    /* Use NOVAS routine to calculate small delta between Terrestrial and
       Barycentric times.  This difference is a small periodic difference that
       must be solved for.  Note that the TT time is used as an input even 
       though the NOVAS code considers it the TDB time, but they're close
       enough that the difference is valid. */
    jd_tdb = jd_tt;
    NOVAS_TDB2TT(jd_tdb, &dummy, &secdiff);

    /* Calculate the TDB time */
    jd_tdb = jd_tt + secdiff / IAS_SEC_PER_DAY;

    /* Calculate full julian day for current year.  Note this will contain leap
       seconds up to the start of the current year. */
    jd_utc = ias_math_compute_full_julian_date(ephem_time[0], 1, 1, 0);

    /* Note ephem_time[1] and ephem_time[2] is UTC and will contain leap
       seconds */
    *ut1 = jd_utc + (ephem_time[1] - 1.0)
        + (ut1_utc + ephem_time[2]) / IAS_SEC_PER_DAY;

    *tt  = jd_tt;
    *tdb = jd_tdb;

    return SUCCESS;
}
