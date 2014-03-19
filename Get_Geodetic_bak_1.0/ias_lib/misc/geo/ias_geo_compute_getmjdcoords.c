/******************************************************************************
NAME: ias_geo_compute_getmjdcoords

PURPOSE:
calculates the modified Julian date and then sees if the date is in the CPF

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion 
ERROR    Operation failed

******************************************************************************/
#include <stdio.h>
#include <math.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_cpf.h"
#include "ias_geo.h"

int ias_geo_compute_getmjdcoords(
    const double ephem_time[3],/* I: year, day, & second of the ephemeris time*/
    IAS_CPF *cpf,        /* I: the information from the CPF */
    double *mjd,        /* O: modified Julian date */
    double *x,          /* O: x shift pole wander in arc sec. */
    double *y,          /* O: y shift pole wander in arc sec. */
    double *ut1_utc     /* O: UT1 - UTC time difference in seconds */
)
{
    int year = 1;       /* Year */
    int mo = 1;         /* First Month */
    int day = 1;        /* First Day */
    int status;

    /* Calculate the full Julian Date                                       */
    year = (int)ephem_time[0];
    *mjd = ias_math_compute_full_julian_date(year, mo, day, 0);
    *mjd += (ephem_time[1] - 1.0);
    *mjd += (ephem_time[2] / IAS_SEC_PER_DAY);

    /* Calculate the Modified Julian Date with:
           Modified Julian Date = Full Julian Date - MJD */
    *mjd -= IAS_MODIFIED_JULIAN_DATE;

    /* Get the coordinates that correspond to the mjd from the CPF */
    status = ias_geo_find_mjdcoords(*mjd,cpf,x,y,ut1_utc);
    if (status != SUCCESS) 
    {
        IAS_LOG_ERROR("Returned from ias_geo_find_mjdcoords");
        return ERROR;
    }

    return SUCCESS;
}
