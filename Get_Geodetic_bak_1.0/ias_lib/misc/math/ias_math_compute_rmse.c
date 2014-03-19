/*****************************************************************************
NAME: ias_math_compute_rmse        

PURPOSE: Calculates the root mean square error.

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  RMSE was calculated.
ERROR    Sample size was zero.

******************************************************************************/
#include <math.h>                /* sqrt prototype */
#include "ias_logging.h"         /* ias_logging prototype */
#include "ias_const.h"           /* ERROR/SUCCESS defines */
#include "ias_math.h"            /* prototype */


int ias_math_compute_rmse
(
    const double *sample,   /* I: pointer to the data */
    int  nsamps,            /* I: number of samples */
    double *rms_error       /* O: root mean square error of the data */
)
{
    int  i;                 /* loop variable */
    double sum_sqr = 0.0;   /* sum of the squared samples */

    /* RMSE calculation.  If number of samples is invalid return ERROR. */
    if (nsamps <= 0)
    {
        IAS_LOG_ERROR("Sample size is invalid");
        *rms_error = 0.0;
        return ERROR;
    }

    for (i = 0; i < nsamps; i++)
        sum_sqr += sample[i] * sample[i];

    *rms_error = sqrt(sum_sqr / nsamps);

    return SUCCESS;
}
