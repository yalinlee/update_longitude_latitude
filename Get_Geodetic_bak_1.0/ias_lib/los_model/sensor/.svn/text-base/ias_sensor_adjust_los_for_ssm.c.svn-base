/******************************************************************************
NAME: ias_sensor_adjust_los_for_ssm

PURPOSE: Adjust the line of sight vector to account for the scene select
    mirror (ssm) position

******************************************************************************/
#include <stdio.h>
#include <math.h>
#include "ias_logging.h"
#include "ias_sensor_model.h"

/******************************************************************************
NAME: get_ssm_position_at_time

PURPOSE: Interpolates the SSM position at the requested time

RETURNS:
    SUCCESS or ERROR

******************************************************************************/
static int get_ssm_position_at_time
(
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm, /* I: SSM structure */
    double seconds_from_epoch,   /* I: seconds from epoch */
    double *ssm_angle            /* O: Output SSM angle (rads) */
)
{
    int index;                   /* Index after seconds_from_epoch */
    double weight;               /* Interpolation weight */
    IAS_SENSOR_SSM_RECORD *record;

    /* See if the input time is within the SSM data interval */
    if (seconds_from_epoch < ssm->records[0].seconds_from_epoch)
    {
        IAS_LOG_ERROR("Requested time (%f) falls before SSM data",
                      seconds_from_epoch);
        return ERROR;
    }
    if (seconds_from_epoch 
        > ssm->records[ssm->ssm_record_count - 1].seconds_from_epoch)
    {
        IAS_LOG_ERROR("Requested time (%f) falls after SSM data",
                      seconds_from_epoch);
        return ERROR;
    }

    /* Compute the index of the record before the needed time */
    index = 1;
    while ((ssm->records[index].seconds_from_epoch < seconds_from_epoch)
           && (index < ssm->ssm_record_count - 1))
    {
        index++;
    }

    record = &ssm->records[index - 1];

    /* Linear interpolate between the points */
    weight = (record[1].mirror_angle - record[0].mirror_angle)
           / (record[1].seconds_from_epoch - record[0].seconds_from_epoch);

    *ssm_angle = record[0].mirror_angle 
               + (seconds_from_epoch - record[0].seconds_from_epoch) * weight;

    return SUCCESS;
}

/******************************************************************************
NAME: ias_sensor_adjust_los_for_ssm

PURPOSE: Adjust the line of sight vector to account for the scene select
    mirror (ssm) position

RETURNS:
    SUCCESS or ERROR
******************************************************************************/
int ias_sensor_adjust_los_for_ssm
(
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm, /* I: SSM structure */
    double seconds_from_epoch,   /* I: seconds from epoch */
    IAS_VECTOR *in_los,          /* I: telescope LOS vector */
    IAS_VECTOR *out_los          /* O: SSM adjusted LOS vector */
)
{
    double ssm_angle;            /* SSM angle at the image time */
    double ssm_rot[3][3];        /* SSM rotation matrix */
    double cosa, sina;           /* Trig functions of SSM angle */
    IAS_VECTOR tlos;             /* Telescope LOS */

    if (get_ssm_position_at_time(ssm, seconds_from_epoch, &ssm_angle)
            != SUCCESS)
    {
        IAS_LOG_ERROR("No valid SSM angle available");
        return ERROR;
    }

    /* Construct the SSM matrix */
    cosa = cos(ssm_angle);
    sina = sin(ssm_angle);
    ssm_rot[0][0] = cosa;
    ssm_rot[0][1] = -sina;
    ssm_rot[0][2] = 0.0;
    ssm_rot[1][0] = sina * cosa;
    ssm_rot[1][1] = cosa * cosa;
    ssm_rot[1][2] = -sina;
    ssm_rot[2][0] = sina * sina;
    ssm_rot[2][1] = sina * cosa;
    ssm_rot[2][2] = cosa;

    /* Apply the telescope aligment matrix to the original LOS */
    tlos.x = ssm->alignment_matrix[0][0] * in_los->x
           + ssm->alignment_matrix[0][1] * in_los->y
           + ssm->alignment_matrix[0][2] * in_los->z;
    tlos.y = ssm->alignment_matrix[1][0] * in_los->x
           + ssm->alignment_matrix[1][1] * in_los->y
           + ssm->alignment_matrix[1][2] * in_los->z;
    tlos.z = ssm->alignment_matrix[2][0] * in_los->x
           + ssm->alignment_matrix[2][1] * in_los->y
           + ssm->alignment_matrix[2][2] * in_los->z;

    /* Apply the SSM matrix */
    out_los->x = ssm_rot[0][0] * tlos.x
               + ssm_rot[0][1] * tlos.y
               + ssm_rot[0][2] * tlos.z;
    out_los->y = ssm_rot[1][0] * tlos.x
               + ssm_rot[1][1] * tlos.y
               + ssm_rot[1][2] * tlos.z;
    out_los->z = ssm_rot[2][0] * tlos.x
               + ssm_rot[2][1] * tlos.y
               + ssm_rot[2][2] * tlos.z;

    return SUCCESS;
}
