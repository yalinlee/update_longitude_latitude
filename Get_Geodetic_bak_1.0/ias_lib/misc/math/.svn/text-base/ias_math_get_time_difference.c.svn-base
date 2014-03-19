/*****************************************************************************
NAME: ias_math_get_time_difference

PURPOSE: Calculate the difference in seconds between two dates in 
    year/doy/seconds-of-day arrays.


RETURNS: SUCCESS or ERROR

NOTES:
    This routine currently only handles dates on two consecutive days.  If
    the dates are longer apart than that, it returns an error.  If needed,
    it can be modified to handle dates that are farther apart.

    The code assumes that dates with year differences are Dec. 31st and
    Jan. 1st only.  Maybe that is obvious from the previous note.
*****************************************************************************/
#include <math.h>

#include "ias_logging.h"
#include "ias_const.h"
#include "ias_math.h"

int ias_math_get_time_difference
(
    const double *epoch_1, /* I: epoch year, DOY, SOD */
    const double *epoch_2, /* I: epoch year, DOY, SOD */
    double *seconds        /* O: the difference in seconds. */
)
{
    double secs;

    /* default is just the seconds difference */
    secs = epoch_1[2] - epoch_2[2];

    /* check to see if the year and DOY match or not */
    if (epoch_1[0] != epoch_2[0] || epoch_1[1] != epoch_2[1])
    {
        /* check if the year is off by 1 and if one of the days is the 1st */
        if ((fabs(epoch_1[0] - epoch_2[0]) == 1)
            && (epoch_1[1] == 1 || epoch_2[1] == 1))
        {
            if (epoch_1[0] > epoch_2[0])
                secs = secs + IAS_SEC_PER_DAY;
            else
                secs = secs - IAS_SEC_PER_DAY;
        }
        /* else check to see if the DOY is off by 1 */
        else if (fabs(epoch_1[1] - epoch_2[1]) == 1)
        {
            if (epoch_1[1] > epoch_2[1])
                secs = secs + IAS_SEC_PER_DAY;
            else
                secs = secs - IAS_SEC_PER_DAY;
        }
        else
        {
            IAS_LOG_ERROR("The two dates differ by more than one "
                          "epoch 1: year %f DOY %f SOD %f "
                          "epoch 2: year %f DOY %f SOD %f ",
                          epoch_1[0], epoch_1[1], epoch_1[2],
                          epoch_2[0], epoch_2[1], epoch_2[2]);
            *seconds = 0.0;
            return ERROR;
        }
    }

    /* Set the output parameter with the results. */
    *seconds = secs;

    return SUCCESS;
}
