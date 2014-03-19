/******************************************************************************
NAME: ias_sensor_set_ssm_from_l0r

PURPOSE: Set the Scene Select Mirror telemetry into the model from the L0R.
    Information needed include the sample times for each SSM angle and the SSM
    encoder angles.

RETURNS
    SUCCESS or ERROR
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../logging_channel.h" /* define the debug logging channel */
#include "ias_logging.h"
#include "ias_cpf.h"
#include "ias_sensor_model.h"
#include "local_defines.h"

/* Constant value for expected TIRS scene select mirror electronics mode */
/* The SSM has two modes of operation. MCE (mechanical electronics) mode 4 is
   what we expect it to be using normally. There is also a mode 10 that was
   added during instrument testing to address some unusual behavior being seen.
   It has been decided that mode 10 won't be used on orbit and even if it were,
   it doesn't really affect us... much. So, if the SSM electronics isn't in
   mode 4, a warning is appropriate at this time. If it ends up being in mode
   10 much, we'll probably have to do something extra some day to beef up a few
   things. */
#define SSM_MCE_NORMAL_MODE 4

int ias_sensor_set_ssm_from_l0r
(
    TIRS_SENSOR_L0R_DATA *l0r_data, /* I: L0R data to create a model */
    IAS_CPF *cpf,                   /* I: Calibration parameter file */
    IAS_SENSOR_MODEL *sensor,       /* IO: Sensor model to update with flags */
    IAS_SENSOR_ID sensor_id         /* I: Sensor id to set SSM info into */
)
{
    int l0r_record_count;           /* Number of records in L0R */
    double start_secs;              /* Image start time from spacecraft epoch */
    double end_secs;                /* Image end time from spacecraft epoch */
    double ssm_secs;                /* SSM record time from spacecraft epoch */
    IAS_L0R_TIRS_SSM_ENCODER *l0r_ssm_records;/* SSM records from l0r_data */
    int field;                      /* Loop counter */
    int index;                      /* Loop counter */
    int start_rec;                  /* First SSM record to include in model */
    int current_rec;                /* Current SSM record when looping */
    int scene_ssm_records;          /* SSM record count to include in model */
    int num_frames;                 /* Count of frames for a sensor */
    IAS_SENSOR_SSM_RECORD *ssm_data;/* Structure array to hold model SSM data */
    double sample_time;             /* The time between SSM samples */
    double ssm_scale;               /* SSM scale factor */
    double pi;                      /* Constant */
    double encoder_origin;          /* Encoder angle at nadir - from CPF
                                       (depends on MCE side) */
    unsigned char sideA = 0x01;     /* Electronics enabled mask - MCE side A */
    unsigned char sideB = 0x02;     /* Electronics enabled mask - MCE side B */
    int mce_vote = 0;               /* MCE majority vote variable */
    int mech_mode_warn = 0;         /* Flag to track if warning issued about
                                       unexpected mechanical mode yet */
    int ssm_position_warn = 0;      /* Flag to track if warning issued about 
                                       the SSM position yet */
    IAS_L0R_TIRS_SSM_ENCODER *record; /* Pointer to the current ssm_record */
    const struct IAS_CPF_TIRS_PARAMETERS *tirs_params; 
                                    /* Pointer to CPF TIRS parameters */
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm_model;
                                    /* Cached pointer to the SSM model */

    /* Make sure the sensor id is valid */
    if (sensor_id < 0 || sensor_id >= IAS_MAX_SENSORS)
    {
        IAS_LOG_ERROR("Invalid sensor id: %d", sensor_id);
        return ERROR;
    }

    /* Cache a pointer to the ssm model for the sensor */
    ssm_model = sensor->sensors[sensor_id].ssm_model;

    /* Get the pi constant */
    pi = ias_math_get_pi();

    /* Get the TIRS parameters from the CPF */
    tirs_params = ias_cpf_get_tirs_parameters(cpf);
    if (!tirs_params)
    {
        IAS_LOG_ERROR("Failed to retrieve TIRS parameters from the CPF");
        return ERROR;
    }

    l0r_ssm_records = l0r_data->ssm_records;
    l0r_record_count = l0r_data->num_ssm_records;

    /* Find a band from this sensor and copy the image epoch time to the SSM
       structure */
    for (index = 0; index < sensor->band_count; index++)
    {
        IAS_SENSOR_BAND_MODEL *band = &sensor->bands[index];

        if (band->band_present && (band->sensor->sensor_id == sensor_id))
        {
            ssm_model->utc_epoch_time[0] = band->utc_epoch_time[0];
            ssm_model->utc_epoch_time[1] = band->utc_epoch_time[1];
            ssm_model->utc_epoch_time[2] = band->utc_epoch_time[2];
            break;
        }
    }
    if (index >= sensor->band_count)
    {
        IAS_LOG_ERROR("Unable to find a band present for the sensor");
        return ERROR;
    }

    /* Logic to check MCE A/B side flags */
    for (index = 0; index < l0r_record_count; index++)
    {
        if ((l0r_ssm_records[index].elec_enabled_flags & sideA) > 0)
            mce_vote++;
        if ((l0r_ssm_records[index].elec_enabled_flags & sideB) > 0)
            mce_vote--;

        /* While looping through all the records, also check the SSM
           mechanical mode and warn if any mode other than the normal
           mode expected is found for any telemetry sample */
        if (!mech_mode_warn
                && (l0r_ssm_records[index].ssm_mech_mode
                    != SSM_MCE_NORMAL_MODE))
        {
            IAS_LOG_WARNING("MCE mode %d in use, not mode %d as expected",
                    l0r_ssm_records[index].ssm_mech_mode,
                    SSM_MCE_NORMAL_MODE);
            mech_mode_warn = 1;
        }

        /* Also check the SSM position and issue a warning if it is not nadir- 
           pointing for any telemetry sample */
        if (!ssm_position_warn && (l0r_ssm_records[index].ssm_position_sel !=
            IAS_L0R_TIRS_SSM_NADIR_POSITION))
        {
            IAS_LOG_WARNING("SSM position at index %d is %d, not %d (nadir "
                "position) as expected", index,
                l0r_ssm_records[index].ssm_position_sel, 
                IAS_L0R_TIRS_SSM_NADIR_POSITION);
            ssm_position_warn = 1;
        }
    }

    /* Set the encoder origin value based on the majority MCE electronics
       side being used */
    if (mce_vote < 0)
        encoder_origin = tirs_params->ssm_encoder_origin_side_b;
    else
        encoder_origin = tirs_params->ssm_encoder_origin_side_a;

    /* Align the TIRS SSM encoder telemetry and trim back down to 20 samples */
    ias_sensor_align_ssm_data(l0r_ssm_records, l0r_record_count);

    /* Check the SSM data time codes and angle values for consistency */
    if (ias_sensor_check_ssm_encoder_data(l0r_ssm_records, l0r_record_count,
                encoder_origin, cpf, &l0r_data->num_bad_ssm_times,
                &l0r_data->num_bad_ssm_encoder_values) != SUCCESS)
    {
        IAS_LOG_ERROR("SSM telemetry is bad");
        return ERROR;
    }

    /* Convert the epoch time to J2000 seconds */
    if (ias_math_convert_year_doy_sod_to_j2000_seconds(
                ssm_model->utc_epoch_time, &start_secs) != SUCCESS)
    {
        IAS_LOG_ERROR("Failed to convert epoch year/doy/sod to J2000");
        return ERROR;
    }

    /* Find the SSM record at the start of the scene */
    start_rec = 0;
    for (field = 0; field < l0r_record_count; field++)
    {
        record = &l0r_ssm_records[field];

        ssm_secs = (double)record->l0r_time.days_from_J2000 * IAS_SEC_PER_DAY
            + record->l0r_time.seconds_of_day
            + tirs_params->ssm_encoder_time_offset;

        if (ssm_secs > start_secs)
            break;

        start_rec = field;
    }

    /* Back up one record, if possible, to account for the need to calculate
       the terrain sensitivity using the maximum offset which can result in
       needing SSM information from approximately 10 * sampling time before
       the start of the image.  A single record is enough to cover that time. */
    if (start_rec > 0)
        start_rec--;

    /* Initialize the count of SSM records to use for the scene to one since
       we always include the first record and start looking for the ending
       record one record after the starting record */
    scene_ssm_records = 1;

    /* Calculate the end time for the scene */
    num_frames = sensor->frame_counts[sensor_id];
    end_secs = start_secs
        + sensor->frame_seconds_from_epoch[sensor_id][num_frames - 1];
    
    /* Starting with the record after the starting record (since we have to
       include at least one record), count the number of records available
       until the record after the ending time */
    for (field = (start_rec + 1); field < l0r_record_count; field++)
    {
        record = &l0r_ssm_records[field];

        ssm_secs = (double)record->l0r_time.days_from_J2000 * IAS_SEC_PER_DAY
            + record->l0r_time.seconds_of_day
            + tirs_params->ssm_encoder_time_offset;

        scene_ssm_records++;

        if (ssm_secs > end_secs)
            break;
    }

    /* Add an extra record at the end, if possible, just in case */
    if ((start_rec + scene_ssm_records) < (l0r_record_count - 1))
    {
        scene_ssm_records++;
    }
    else if ((start_rec + scene_ssm_records) > l0r_record_count)
    {
        /* Make sure the scene_ssm_records + start_rec isn't greater than the
           l0r_record_count.  Note that this should never happen, but extra
           insurance doesn't hurt since a bug caused this to happen at one
           point. */
        IAS_LOG_WARNING("Number of scene_ssm_records (%i) + start_rec (%i) is "
            "> l0r_record_count (%i), adjusting accordingly",
            scene_ssm_records, start_rec, l0r_record_count);
        scene_ssm_records = l0r_record_count - start_rec;
    }

    /* Store number of SSM samples in model, there are
       IAS_L0R_TIRS_SSM_ENCODER_SAMPLES samples per record */
    ssm_model->ssm_record_count = IAS_L0R_TIRS_SSM_ENCODER_SAMPLES
        * scene_ssm_records;
    IAS_LOG_DEBUG("Number of SSM samples %d", ssm_model->ssm_record_count);

    /* Allocate space for the output SSM data */
    ssm_data = malloc(sizeof(*ssm_data) * ssm_model->ssm_record_count);
    if (ssm_data == NULL)
    {
        IAS_LOG_ERROR("Error allocating SSM data array");
        return ERROR;
    }

    /* Calculate the scaling factor to convert from SSM encoder counts to
       radians */
    ssm_scale = 2.0 * pi/(double)TIRS_SSM_OVERFLOW;

    /* Convert the SSM times to seconds from epoch */
    current_rec = start_rec;
    sample_time = 1.0 / IAS_L0R_TIRS_SSM_ENCODER_SAMPLES;
    for (field = 0 ; field < ssm_model->ssm_record_count;
            field += IAS_L0R_TIRS_SSM_ENCODER_SAMPLES)
    {
        int prev;                  /* Previous SSM encoder sample */
        double seconds_from_epoch; /* Cached copy of seconds from epoch
                                      for the 24-bit record */

        record = &l0r_ssm_records[current_rec++];

        /* Calculate the time at the current record */
        seconds_from_epoch
            = (double)record->l0r_time.days_from_J2000 * IAS_SEC_PER_DAY
            + record->l0r_time.seconds_of_day
            + tirs_params->ssm_encoder_time_offset - start_secs;

        ssm_data[field].seconds_from_epoch = seconds_from_epoch;

        /* Compute the offset times for each additional sample */
        for (index = 1; index < IAS_L0R_TIRS_SSM_ENCODER_SAMPLES; index++)
        {
            ssm_data[field + index].seconds_from_epoch =
                seconds_from_epoch + (sample_time * index);
        }

        /* Convert the angles */
        prev = record->encoder_position[0] & TIRS_SSM_BIT_MASK;
        ssm_data[field].mirror_angle = (double)prev * ssm_scale
            - encoder_origin;

        /* Make sure the angle is between -pi and pi */
        while (ssm_data[field].mirror_angle > pi)
            ssm_data[field].mirror_angle -= 2.0 * pi;
        while (ssm_data[field].mirror_angle < -pi)
            ssm_data[field].mirror_angle += 2.0 * pi;

        /* Convert the SSM encoder samples to angles */
        for (index = 1; index < IAS_L0R_TIRS_SSM_ENCODER_SAMPLES; index++)
        {
            int delta_ssm;          /* Change in SSM encoder reading */
            int current;            /* Current SSM encoder sample */
            int last24;             /* Previous 24-bit encoder sample */
            double mirror_angle;    /* Temporary mirror angle */

            /* Convert the encoder samples */
            last24 = prev & TIRS_SSM_BIT_MASK;
            current = record->encoder_position[index] & TIRS_SSM_BIT_MASK;
            delta_ssm = current - last24;
            if (delta_ssm > TIRS_SSM_LIMIT)
                delta_ssm -= TIRS_SSM_OVERFLOW;
            if (delta_ssm < -TIRS_SSM_LIMIT)
                delta_ssm += TIRS_SSM_OVERFLOW;

            current = prev + delta_ssm;
            if (current < 0)
                current += TIRS_SSM_OVERFLOW;

            /* Convert the ssm encoder count to radians and limit to be
               between -pi and pi */
            mirror_angle = (double)current * ssm_scale - encoder_origin;
            while (mirror_angle > pi)
                mirror_angle -= 2.0 * pi;
            while (mirror_angle < -pi)
                mirror_angle += 2.0 * pi;

            /* Save the mirror angle */
            ssm_data[field + index].mirror_angle = mirror_angle;

            prev = current;
        }
    }

    /* Save the SSM data in the model */
    ssm_model->records = ssm_data;

    /* Smooth the SSM data to suppress encoder jitter */
    if (ias_sensor_smooth_ssm_data(ssm_model) != SUCCESS)
    {
        IAS_LOG_ERROR("Error smoothing SSM data");
        free(ssm_model->records);
        ssm_model->records = NULL;
        return ERROR;
    }

    return SUCCESS;
}
