/*****************************************************************************
NAME: ias_math_compute_stdev

PURPOSE: Calculates the sample standard deviation.

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Standard deviation was calculated.
ERROR    Sample size was zero.

*******************************************************************************/
#include <math.h>               /* sqrt prototype */
#include "ias_logging.h"        /* ias_logging prototype */
#include "ias_const.h"          /* ERROR/SUCCESS defines */
#include "ias_math.h"           /* prototype */


int ias_math_compute_stdev
(
    const double *sample,   /* I: pointer to the data */
    int nsamps,             /* I: number of samples */
    double mean,            /* I: mean of the samples */
    double *std_dev         /* O: standard deviation of the data */
)
{
    int i;                  /* loop variable */
    double sum_sqr = 0.0;   /* sum of the squared difference from mean */

    /* Standard deviation calculation.  If number of samples is invalid
       return ERROR. */
    if (nsamps <= 1)
    {
        IAS_LOG_ERROR("Sample size is invalid");
        *std_dev = 0.0;
        return ERROR;
    }

    /* compute the sum of squares */
    for (i = 0; i < nsamps; i++)
        sum_sqr += sample[i] * sample[i];
        
    /* subtract mean squared for each sample */
    sum_sqr -= nsamps * (mean * mean);

    /* Use the absolute value to avoid round off errors */
    *std_dev = sqrt(fabs(sum_sqr) / (nsamps-1));

    return SUCCESS;
}
