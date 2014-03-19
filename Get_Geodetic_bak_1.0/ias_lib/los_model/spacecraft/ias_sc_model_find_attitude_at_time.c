/******************************************************************************
Name: ias_sc_model_find_attitude_at_time

Purpose: Find the precise roll, pitch and yaw at requested time.  Values are 
    interpolated between two closet neighbors (roll, pitch, yaw) of requested
    time. 

RETURNS: SUCCESS or ERROR

******************************************************************************/
#include <stdio.h>
#include <math.h>
#include "ias_logging.h"
#include "ias_spacecraft_model.h"

int ias_sc_model_find_attitude_at_time
(
   const IAS_SC_ATTITUDE_MODEL *att,    /* I: Attitude structure */
   double att_time,                     /* I: Actual time (sec from epoch) */
   double *roll,                        /* O: Output roll (radians) */
   double *pitch,                       /* O: Output pitch (radians) */
   double *yaw                          /* O: Output yaw (radians) */
)
{
    int index1;                  /* Index to the "left of" att_time */
    int index2;                  /* Index to the "right of" att_time */
    double w;                    /* Interpolation weight */

    /* Find the values of roll, pitch, and yaw before and after input time */
    index1 = (int)floor(att_time / att->nominal_sample_time);
    index2 = index1 + 1;

    if (index1 < 0)
    {
        IAS_LOG_ERROR("Attitude correction not found for time offset %f. "
        "Index calculated falls \"left of\" first attitude sample.", att_time);
        return ERROR;
    }
    if (index2 > (att->sample_count - 1))
    {
        IAS_LOG_ERROR("Attitude correction not found for time offset %f. "
        "Index calculated falls \"right of\" last attitude sample.", att_time);
        return ERROR;
    }

    /* Linear interpolate between the points */
    w  = fmod(att_time, att->nominal_sample_time)/att->nominal_sample_time;
    *roll  =  att->sample_records[index1].precision_attitude.x
           + (att->sample_records[index2].precision_attitude.x 
           -  att->sample_records[index1].precision_attitude.x) * w;
    *pitch =  att->sample_records[index1].precision_attitude.y
           + (att->sample_records[index2].precision_attitude.y 
           -  att->sample_records[index1].precision_attitude.y) * w;
    *yaw   =  att->sample_records[index1].precision_attitude.z
           + (att->sample_records[index2].precision_attitude.z 
           -  att->sample_records[index1].precision_attitude.z) * w;

    return SUCCESS;
}
