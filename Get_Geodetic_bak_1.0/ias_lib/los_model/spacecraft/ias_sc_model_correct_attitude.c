/******************************************************************************
NAME: ias_sc_model_correct_attitude

PURPOSE: Find the precision roll, pitch and yaw 

RETURNS: SUCCESS or ERROR

******************************************************************************/
#include <stdio.h>
#include <math.h>
#include "ias_const.h"
#include "ias_math.h"
#include "ias_logging.h"
#include "ias_spacecraft_model.h"

int ias_sc_model_correct_attitude
(
    const double *image_epoch,  /* I: Image epoch time (year/doy/sod array) */
    const IAS_SC_PRECISION_MODEL *prec_model,   /* I: Precision parameters */
    IAS_SC_ATTITUDE_MODEL *att_model            /* IO: Attitude to correct */
)
{
    double p_roll;          /* Precision roll value */
    double p_pitch;         /* Precision pitch value */
    double p_yaw;           /* Precision yaw value */
    double cr;              /* cosine of roll */
    double sr;              /* sine of roll */
    double cp;              /* cosine of pitch */
    double sp;              /* sine of pitch */
    double cy;              /* cosine of yaw */
    double sy;              /* sine of yaw */
    double preccor[3][3];   /* Precision r/p/y */
    double acs2sat[3][3];   /* For Earth = ACS2ORB for Space = ACS2ECI */
    double precacs[3][3];   /* [Precision Correction] x [OLI2ACS] */
    double att_time;        /* Current attitude time with respect to
                               precision epoch */
    double seconds_from_image_epoch; /* Time different from ephemeris epoch to
                                     image epoch */
    int i, j;               /* Looping counters */
    /* calculate the time difference between the ephemeris epoch and the 
       image epoch */

    if (ias_math_get_time_difference(att_model->utc_epoch_time,
            image_epoch, &seconds_from_image_epoch) != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating time difference between the attitude and "
            "image epoch times");
        return ERROR;
    }

    for (i = 0; i < att_model->sample_count; i++)
    {
        /* Calculate the attitude correction. */
        att_time = seconds_from_image_epoch
            + att_model->sample_records[i].seconds_from_epoch
            - prec_model->seconds_from_image_epoch;
        p_roll = prec_model->roll_corr[0];
        p_pitch = prec_model->pitch_corr[0];
        p_yaw = prec_model->yaw_corr[0];
        for (j = 1; j < prec_model->attitude_order; j++)
        {
            p_roll += prec_model->roll_corr[j] * pow(att_time, (double)j);
            p_pitch += prec_model->pitch_corr[j] * pow(att_time, (double)j);
            p_yaw += prec_model->yaw_corr[j] * pow(att_time, (double)j);
        }

        /* Build the precision rotation matrix */
        cr = cos(p_roll);
        sr = sin(p_roll);
        cp = cos(p_pitch);
        sp = sin(p_pitch);
        cy = cos(p_yaw);
        sy = sin(p_yaw);
        preccor[0][0] = cp * cy;    
        preccor[0][1] = cy * sr * sp + cr * sy;     
        preccor[0][2] = sr * sy - sp * cr * cy;
        preccor[1][0] = -sy * cp;   
        preccor[1][1] = cy * cr - sy * sr * sp;     
        preccor[1][2] = sp * sy * cr + cy * sr;
        preccor[2][0] = sp;       
        preccor[2][1] = -sr * cp;               
        preccor[2][2] = cp * cr;

        /* Build the original rotation matrix */
        cr = cos(att_model->sample_records[i].satellite_attitude.x);
        sr = sin(att_model->sample_records[i].satellite_attitude.x);
        cp = cos(att_model->sample_records[i].satellite_attitude.y);
        sp = sin(att_model->sample_records[i].satellite_attitude.y);
        cy = cos(att_model->sample_records[i].satellite_attitude.z);
        sy = sin(att_model->sample_records[i].satellite_attitude.z);
        acs2sat[0][0] = cp * cy;    
        acs2sat[0][1] = cy * sr * sp + cr * sy;     
        acs2sat[0][2] = sr * sy - sp * cr * cy;
        acs2sat[1][0] = -sy * cp;   
        acs2sat[1][1] = cy * cr - sy * sr * sp;     
        acs2sat[1][2] = sp * sy * cr + cy * sr;
        acs2sat[2][0] = sp;       
        acs2sat[2][1] = -sr * cp;               
        acs2sat[2][2] = cp * cr;

        /* Multiply to get net rotation matrix */
        ias_math_multiply_3x3_matrix(acs2sat, preccor, precacs);

        /* Back out net roll, pitch, yaw */
        att_model->sample_records[i].precision_attitude.x
            = atan2(-precacs[2][1],precacs[2][2]);
        att_model->sample_records[i].precision_attitude.y = asin(precacs[2][0]);
        att_model->sample_records[i].precision_attitude.z
            = atan2(-precacs[1][0],precacs[0][0]);
    }

    return SUCCESS;
}
