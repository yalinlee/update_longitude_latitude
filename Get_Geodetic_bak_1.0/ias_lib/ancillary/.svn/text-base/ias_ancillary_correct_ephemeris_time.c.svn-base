/******************************************************************************
NAME: ias_ancillary_correct_ephemeris_time

PURPOSE: Correct jitter in ephemeris time codes.  Inconsistencies between
         time stamps stored within an array of times, associated with the
         spacecraft ephemeris, are compared against times based on the
         ephemeris position and veloctiy vectors.  Smoothing of the time
         stamps is done by adjusting the times stamps according to the
         average of the differences and the differences themselves.

RETURN VALUE: Type = int
    Value    Description
    -----    -----------
    SUCCESS  Successful completion
    ERROR    Operation failed
******************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "ias_logging.h"
#include "ias_ancillary_private.h"

int ias_ancillary_correct_ephemeris_time
(
    const IAS_VECTOR *raw_pos,  /* I: array of ephemeris position data */
    const IAS_VECTOR *raw_vel,  /* I: array of ephemeris velocity data */
    int valid_ephemeris_count,  /* I: number of ephemeris points in the arrays*/
    double *ephemeris_time_data /* I/O: array of ephemeris time data */
)
{
    int index = 0;          /* loop counter */
    double dpx, dpy, dpz;   /* position difference ([i] - [i-1]) */
    double vx, vy, vz;      /* veloctiy of [i-1] */
    double mag_pos;         /* magnitude of position differences */
    double mag_vel;         /* magnitude of velocity */
    double sum;             /* sum of corrected delta time */
    double *dtc = NULL;     /* corrected delta time */
    double avg;             /* average time correction */

    /* Verify valid_ephemeris_count > 1 to prevent divide by zero later on */
    if (valid_ephemeris_count < 2)
    {
        IAS_LOG_ERROR("Not enough valid ephemeris points to continue: %d",
            valid_ephemeris_count);
        return ERROR;
    }

    /* Allocate array that will hold delta times */
    dtc = (double *)malloc(sizeof(double) * valid_ephemeris_count);
    if (dtc == NULL)
    {
        IAS_LOG_ERROR("Allocating dtc");
        return ERROR;
    }

    /* loop over ephemeris */
    dtc[index] = 0.0;
    for (index = 1, sum = 0.0; index < valid_ephemeris_count; index++)
    {
        dpx = raw_pos[index].x - raw_pos[index - 1].x;
        dpy = raw_pos[index].y - raw_pos[index - 1].y;
        dpz = raw_pos[index].z - raw_pos[index - 1].z;

        vx = raw_vel[index - 1].x;
        vy = raw_vel[index - 1].y;
        vz = raw_vel[index - 1].z;

        mag_pos = sqrt(dpx * dpx + dpy * dpy + dpz * dpz);
        mag_vel = sqrt(vx * vx + vy * vy + vz * vz);

        /* Calculate corrected delta time. */
        dtc[index] = mag_pos / mag_vel -
            (ephemeris_time_data[index] - ephemeris_time_data[index - 1]);

        sum += dtc[index];
    }

    /* Smooth times stamps according to the average of the differences and
       the differences themselves. */
    avg = sum / (double)(valid_ephemeris_count - 1);

    for (index = 1; index < valid_ephemeris_count; index++)
        dtc[index] += dtc[index - 1] - avg;

    for (index = 1; index < valid_ephemeris_count - 1; index++)
        ephemeris_time_data[index] += dtc[index];

    free(dtc);

    return SUCCESS;
}
