/******************************************************************************
NAME: ias_sensor_check_ssm_encoder_data

PURPOSE: Perform quality checks on Scene Select Mirror telemetry and correct
    bad data.

RETURNS:
    SUCCESS or ERROR
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../logging_channel.h" /* define debug logging channel */
#include "ias_logging.h"
#include "ias_cpf.h"
#include "ias_sensor_model.h"
#include "local_defines.h"

/* Define some constants for the SSM mirror data. Confirmed with Cal/Val
   that these do not need to be in the CPF. */
#define SSM_TIME 1.0            /* seconds between SSM records */
#define DTIME_TOL 0.1           /* tolerance in seconds between SSM records */
#define MIN_COUNT_TOL 5         /* minimum allowed number of SSM encoder counts
                                   for the SSM encoder tolerance.  This is just
                                   to prevent a ridiculously small CPF value
                                   from causing problems. */

int ias_sensor_check_ssm_encoder_data
(
    IAS_L0R_TIRS_SSM_ENCODER *l0r_tirs_ssm, /* I/O: L0R SSM encoder records */
    int count,                          /* I: Number of SSM encoder records */
    double encoder_origin,              /* I: Encoder nadir angle for current
                                              MCE side */
    IAS_CPF *cpf,                       /* I: Calibration parameter info */
    int *num_bad_ssm_times,             /* O: number of SSM times corrected */
    int *num_bad_ssm_encoder_values     /* O: number of SSM encoder values
                                              corrected */
)
{
    double prev_ssm_secs = 0.0; /* SSM record time from the previous record */
    int record_index;           /* loop counter */
    int first_valid;            /* Index of first good SSM record */
    int num_bad_ssm;            /* Number of bad SSM records */
    int delta_ssm;              /* Change in SSM encoder reading */
    double ssm_scale;           /* SSM scale factor */
    double ssm_angle_tol;       /* SSM delta angle tolerance in radians */
    int ssm_count_tol;          /* SSM delta angle tolerance in counts */
    int ssm_nadir_counts;       /* SSM nadir reference angle in counts */
    double ssm_nadir_counts_calc;   /* Temp var to calculate ssm_nadir_counts */
    double pi;                  /* constant */
    IAS_L0R_TIRS_SSM_ENCODER *ssm_record;/* pointer to the current ssm_record */
    const struct IAS_CPF_TIRS_PARAMETERS *tirs_params; 
                                /* pointer to CPF TIRS parameters */

    /* Get the TIRS parameters from the CPF */
    tirs_params = ias_cpf_get_tirs_parameters(cpf);
    if (!tirs_params)
    {
        IAS_LOG_ERROR("Failed to retrieve TIRS parameters from the CPF");
        return ERROR;
    }

    /* Check the SSM data time codes for consistency */
    first_valid = -1;
    num_bad_ssm = 0;
    for (record_index = 0; record_index < count; record_index++)
    {
        double time_diff;           /* Time difference between samples */
        double ssm_secs;            /* SSM record time from spacecraft epoch */

        ssm_record = &l0r_tirs_ssm[record_index];

        ssm_secs = (double)ssm_record->l0r_time.days_from_J2000
                 * IAS_SEC_PER_DAY + ssm_record->l0r_time.seconds_of_day;
        if (record_index == 0)
        {
            prev_ssm_secs = ssm_secs;
            continue;
        }
        /* Compute time difference between consecutive SSM records */
        time_diff = fabs(ssm_secs - SSM_TIME - prev_ssm_secs);

        if (first_valid < 0 && time_diff <= DTIME_TOL)
        {
            first_valid = record_index - 1;

            /* Found first good point, so fix all previous */
            while (first_valid > 0)
            {
                IAS_L0R_TIME *l0r_time;

                first_valid--;
                num_bad_ssm++;
                ssm_secs = prev_ssm_secs - SSM_TIME;
                prev_ssm_secs = ssm_secs;

                l0r_time = &l0r_tirs_ssm[first_valid].l0r_time;
                l0r_time->days_from_J2000 = (int)(ssm_secs / IAS_SEC_PER_DAY);
                ssm_secs -= (double)l0r_time->days_from_J2000 * IAS_SEC_PER_DAY;
                l0r_time->seconds_of_day = ssm_secs;
            }
            prev_ssm_secs = (double)ssm_record->l0r_time.days_from_J2000
                       * IAS_SEC_PER_DAY + ssm_record->l0r_time.seconds_of_day;
        }
        else if (first_valid >= 0 && time_diff > DTIME_TOL)
        {
            /* Have a previous valid point, so use it to fix this one */
            num_bad_ssm++;
            ssm_secs = prev_ssm_secs + SSM_TIME;
            prev_ssm_secs = ssm_secs;

            ssm_record->l0r_time.days_from_J2000
                    = (int)(ssm_secs / IAS_SEC_PER_DAY);

            ssm_secs -= (double)ssm_record->l0r_time.days_from_J2000
                      * IAS_SEC_PER_DAY;
            ssm_record->l0r_time.seconds_of_day = ssm_secs;
        }
        else
        {
            /* Current point is OK */
            prev_ssm_secs = ssm_secs;
        }
    }
    if (first_valid < 0)
    {
        IAS_LOG_ERROR("No valid SSM times found");
        return ERROR;
    }

    /* return the number of corrected time codes */
    *num_bad_ssm_times = num_bad_ssm;

    IAS_LOG_DEBUG("%d bad SSM time codes found and corrected", num_bad_ssm);

    /* convert the CPF SSM tolerance in radians to encoder counts */
    pi = ias_math_get_pi();
    ssm_scale = (2.0 * pi) / (double)TIRS_SSM_OVERFLOW;
    ssm_nadir_counts_calc = encoder_origin;
    while (ssm_nadir_counts_calc < 0.0)
        ssm_nadir_counts_calc += 2.0 * pi;
    while (ssm_nadir_counts_calc >= (2.0 * pi))
        ssm_nadir_counts_calc -= 2.0 * pi;
    ssm_nadir_counts_calc /= ssm_scale;
    ssm_nadir_counts = (int)floor(ssm_nadir_counts_calc + 0.5);
    ssm_angle_tol = tirs_params->ssm_tolerance / ssm_scale;
    ssm_count_tol = (int)floor(ssm_angle_tol + 0.5);

    /* make sure the tolerance is no less than the minimum */
    if (ssm_count_tol < MIN_COUNT_TOL)
        ssm_count_tol = MIN_COUNT_TOL;

    IAS_LOG_DEBUG("SSM angle telemetry outlier tolerance is %d counts",
                 ssm_count_tol);

    /* Find the first valid record by checking the 24-bit samples */
    first_valid = -1;
    num_bad_ssm = 0;
    for (record_index = 0; record_index < count; record_index++)
    {
        /* find the difference between the current and previous ssm 1 encoder
           position */
        delta_ssm = abs((int)(l0r_tirs_ssm[record_index].encoder_position[0]
                    & TIRS_SSM_BIT_MASK)
                - (int)(ssm_nadir_counts & TIRS_SSM_BIT_MASK));

        /* check for rollover */
        if (delta_ssm > TIRS_SSM_LIMIT)
            delta_ssm = abs(delta_ssm - TIRS_SSM_OVERFLOW);

        /* if no valid SSM values have been found and this one is valid,
           fix the previous values */
        if (first_valid < 0 && delta_ssm <= ssm_count_tol)
        {
            /* the previous value was the first valid one */
            first_valid = record_index;

            /* repair the previous values and count the bad ones */
            while (first_valid > 0)
            {
                first_valid--;
                num_bad_ssm++;
                l0r_tirs_ssm[first_valid].encoder_position[0]
                    = l0r_tirs_ssm[first_valid+1].encoder_position[0];
            }
        }
        else if (first_valid >= 0 && delta_ssm > ssm_count_tol)
        {
            /* repair the bad SSM values once a good one is found */
            num_bad_ssm++;
            l0r_tirs_ssm[record_index].encoder_position[0]
                = l0r_tirs_ssm[record_index-1].encoder_position[0];
        }
    }
    if (first_valid < 0)
    {
        IAS_LOG_WARNING("No valid SSM angles found; using nominal value");
        for (record_index = 0; record_index < count; record_index++)
        {
            l0r_tirs_ssm[record_index].encoder_position[0] = ssm_nadir_counts;
            num_bad_ssm++;
        }
    }

    /* Now clean up the rest of the 24-bit samples */
    for (record_index = 0; record_index < count; record_index++)
    {
        int index;

        ssm_record = &l0r_tirs_ssm[record_index];

        /* Restore the angle telemetry to 24 bits */
        ssm_record->encoder_position[0] &= TIRS_SSM_BIT_MASK;
        for (index = 1; index < IAS_L0R_TIRS_SSM_ENCODER_SAMPLES; index++)
        {
            int last24;
            int current;

            ssm_record->encoder_position[index] &= TIRS_SSM_BIT_MASK;

            /* get the difference between the current and previous samples */
            last24 = ssm_record->encoder_position[index - 1];
            delta_ssm = (int)ssm_record->encoder_position[index] - last24;

            /* handle the 24-bit rollover */
            if (delta_ssm > TIRS_SSM_LIMIT)
                delta_ssm -= TIRS_SSM_OVERFLOW;
            if (delta_ssm < -TIRS_SSM_LIMIT)
                delta_ssm += TIRS_SSM_OVERFLOW;

            /* get the current 24-bit encoder sample from the previous and
               delta */
            current = last24 + delta_ssm;

            /* Handle underflow by adding TIRS_SSM_OVERFLOW to get back to a
               24-bit unsigned value.  Overflow is handled later by masking
               with TIRS_SSM_BIT_MASK. */
            if (current < 0)
                current += TIRS_SSM_OVERFLOW;

            ssm_record->encoder_position[index] = current;

            /* Now check the tolerance and replace the current one with the
               previous if it is out of tolerance */
            delta_ssm = abs(ssm_record->encoder_position[index]
                            - ssm_record->encoder_position[index - 1]);
            if (delta_ssm > ssm_count_tol)
            {
                num_bad_ssm++;
                ssm_record->encoder_position[index]
                    = ssm_record->encoder_position[index - 1];
            }
        }

        /* Now update the original data */
        for (index = 0; index < IAS_L0R_TIRS_SSM_ENCODER_SAMPLES; index++)
            ssm_record->encoder_position[index] &= TIRS_SSM_BIT_MASK;
    }

    /* return the number of corrected SSM encoder values */
    *num_bad_ssm_encoder_values = num_bad_ssm;

    IAS_LOG_DEBUG("%d bad SSM encoder angles found and corrected", num_bad_ssm);

    return SUCCESS;
}
