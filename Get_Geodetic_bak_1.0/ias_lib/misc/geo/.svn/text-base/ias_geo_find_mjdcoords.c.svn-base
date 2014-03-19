/***************************************************************************
NAME: ias_geo_find_mjdcoords

PURPOSE:
Finds the x, y, and UT1-UTC coords for a given Modified Julian Date (MJD)
and the CPF's UT1_Time_ Parameters. The function returns true if the MJD 
value was found, otherwise the function returns false.  The x, y, and ut1_utc
values are 0.0 if the MJD was not found, otherwise their values are those
coords for the specified MJD.

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion 
ERROR    Operation failed

**************************************************************************/
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_cpf.h"
#include "ias_geo.h"

int ias_geo_find_mjdcoords 
(
    double mjd,        /* I: MJD value to search for (days) */
    IAS_CPF *cpf,      /* I: info from Calibration Parameter File */
    double *x,         /* O: x shift pole wander in are/sec */
    double *y,         /* O: y shift pole wander in are/sec */
    double *ut1_utc    /* O: UT1-UTC coord time difference in sec */
)
{
    int i;          /* loop variable */
    int currmjd;    /* current MJD value */
    int found_flag; /* the flag to show we found the date we want
                       if = 0 not found, = 1 found exact match */
    int lmjd;       /* the MJD rounded to nearest integer  */
    const struct IAS_CPF_UT1_TIME_PARAMETERS *ut1_times;

    /* get the UT1 times from the CPF */
    ut1_times = ias_cpf_get_ut1_times(cpf);
    if (!ut1_times)
    {
        IAS_LOG_ERROR("Getting UT1 times from CPF");
        return ERROR;
    }

    /* Set the return values to 0.0 for defaults.  */
    *x = 0.0;
    *y = 0.0;
    *ut1_utc = 0.0;

    /* round the MJD to the nearest integer */
    lmjd = (int)(mjd + .5);

    found_flag = 0;
    i = 0;
    currmjd = 0;

    /* Search for the desired MJD value. (there are values for IAS_UT1_PERIOD
       days) */
    while (found_flag == 0 && i < IAS_UT1_PERIOD)
    {
        /* If the desired MJD is found then we are done.  */
        currmjd = ut1_times->ut1_modified_julian[i];
        if (currmjd == lmjd)
            found_flag = 1;
        else
            i++;
    }

    /* If the MJD is found then read the x, y, and UT1-UTC coords.  */
    if (found_flag == 1)
    {
        *x = ut1_times->ut1_x[i];
        *y = ut1_times->ut1_y[i];
        *ut1_utc = ut1_times->ut1_utc[i];
        return SUCCESS;
    }
    else
    { 
        /* The MJD was not found. */
        IAS_LOG_ERROR("Modified Julian Date %d not found in CPF", lmjd);
        return ERROR;
    }
}
