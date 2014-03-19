/*****************************************************************************
NAME: ias_math_compute_mean        

PURPOSE: Calculates the mean of the data.

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Mean was calculated.
ERROR    Sample size was zero.


******************************************************************************/
#include "ias_logging.h"        /* ias_logging prototype */
#include "ias_const.h"          /* ERROR/SUCCESS defines */
#include "ias_math.h"           /* ias_mean prototype */

int ias_math_compute_mean
(
    const double *sample,       /* I: pointer to the data */
    int nsamps,                 /* I: number of samples */
    double *mean                /* O: mean of the data */
)
{
    int i;                               /* loop variable */
    double sum=0.0;             /* sum of the samples */

    /* Mean calculation.  If number of samples is invalid return ERROR. */
    if (nsamps <= 0)
    {
        IAS_LOG_ERROR("Sample size is invalid");
        *mean = 0.0;
        return ERROR;
    }

    for (i = 0; i < nsamps; i++)
        sum += sample[i];

    *mean = sum / nsamps;

    return SUCCESS;
}
