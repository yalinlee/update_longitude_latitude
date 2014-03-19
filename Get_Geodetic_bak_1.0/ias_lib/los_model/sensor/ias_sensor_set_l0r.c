/*************************************************************************
Name: ias_sensor_set_l0r

Purpose: Accepts L0R structures and a pointer to a sensor model structure
    to copy the L0R values into.

Notes:
    Meant to be used in conjunction with ias_los_model_set_l0r(). Should
    only be called by that routine, not used independently.

RETURNS: SUCCESS or ERROR
**************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../logging_channel.h" /* define the debug logging channel */
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_sensor_model.h"
#include "local_defines.h"

#define TOLERANCE_FACTOR 6.0 /* Track time codes that exceed threshold by this
            amount */ 

static void log_l0r_values(IAS_SENSOR_MODEL *sensor);

/*************************************************************************
Name: set_band_and_sensor_flags

Purpose: Sets the sensor present and band present flags for each sensor in
    the model and for each band in the model.

Notes:
    local routine

RETURNS: nothing
**************************************************************************/
static void set_band_and_sensor_flags
(
    IAS_SENSOR_L0R_DATA *l0r_data,  /* I: L0R data to create a model */
    IAS_SENSOR_MODEL *sensor        /* IO: Sensor model to update with flags */
)
{
    int band_index;                     /* Band index for looping */

    sensor->sensors[IAS_OLI].sensor_present = l0r_data->oli_data.sensor_present;
    sensor->sensors[IAS_TIRS].sensor_present
        = l0r_data->tirs_data.sensor_present;

    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        sensor->bands[band_index].band_present
            = l0r_data->bands_present[band_index];
    }
}

/*************************************************************************
Name: setup_frames

Purpose: Set the number of frames at the sensor and band levels of the model
    structure. Also allocate memory for and prepare for calculation of times
    from epoch for each frame.

Notes:
    local routine

RETURNS: SUCCESS or ERROR
**************************************************************************/
static int setup_frames
(
    IAS_SENSOR_ID sensor_id,            /* I: Satellite attribs sensor ID */
    int num_frames,                     /* I: Number of frames for this image */
    IAS_SENSOR_MODEL *sensor            /* O: Sensor model to populate */
)
{
    int status;                         /* Routine status */
    int sensor_bands[IAS_MAX_NBANDS];   /* Array of band numbers for sensor */
    int num_sensor_bands;               /* Number of bands in sensor_bands */
    int sensor_band_index;              /* Index for sensor_bands */
    int band_index;                     /* Band array index for looping */

    /* Retrieve the list of bands for this sensor from the satellite
       attributes library */ 
    status = ias_sat_attr_get_sensor_band_numbers(sensor_id, IAS_NORMAL_BAND,
            0, &sensor_bands[0], IAS_MAX_NBANDS, &num_sensor_bands);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Getting bands for sensor");
        return ERROR;
    }

    /* Save the number of frames at the sensor level of the model also */
    sensor->frame_counts[sensor_id] = num_frames;

    /* Allocate memory for frame times; size appropriate to sensor */
    sensor->frame_seconds_from_epoch[sensor_id]
        = malloc(sizeof(*sensor->frame_seconds_from_epoch[sensor_id])
                * num_frames);
    if (sensor->frame_seconds_from_epoch[sensor_id] == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for frame times");
        return ERROR;
    }

    /* Setup the number of frames for the bands for this sensor and save the
       pointer to the sensor level frame times from epoch array */
    for (sensor_band_index = 0; sensor_band_index < num_sensor_bands;
            sensor_band_index++)
    {
        /* Get the band index for the model from the band number */
        band_index = ias_sat_attr_convert_band_number_to_index(
                sensor_bands[sensor_band_index]);
        if (band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting band number %d to index",
                    sensor_bands[sensor_band_index]);
            return ERROR;
        }

        /* Set the band's frame count value */
        sensor->bands[band_index].frame_count = num_frames;

        /* Set the current band's frame times pointer to the frame times
           array at the sensor level of the model structure. Frame times are
           all the same for all bands of the same sensor. */
        sensor->bands[band_index].frame_seconds_from_epoch
            = sensor->frame_seconds_from_epoch[sensor_id];
    }

    return SUCCESS;
}

/*************************************************************************
Name: set_sampling_time

Purpose:  Set the sampling time for each band.

Notes:
    local routine

RETURNS: SUCCESS or ERROR
**************************************************************************/
static int set_sampling_time
(
    IAS_SENSOR_ID sensor_id,            /* I: Satellite attribs sensor ID */
    double base_sample_time,            /* I: Sample time to set in model */
    IAS_SENSOR_MODEL *sensor            /* O: Sensor model to populate */
)
{
    int status;                         /* Routine status */
    int sensor_bands[IAS_MAX_NBANDS];   /* Array of band numbers for sensor */
    int num_sensor_bands;               /* Number of bands in sensor_bands */
    int sensor_band_index;              /* Index for sensor_bands */
    const IAS_BAND_ATTRIBUTES *band_attribs; /* Satellite bands attributes */

    /* Retrieve the list of bands for this sensor from the
       satellite attributes */ 
    status = ias_sat_attr_get_sensor_band_numbers(sensor_id, IAS_NORMAL_BAND,
            0, &sensor_bands[0], IAS_MAX_NBANDS, &num_sensor_bands);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Getting bands for sensor");
        return ERROR;
    }

    /* If no error in frame by frame sample times validation, then save
       the base sample time for all bands for the current sensor. */
    for (sensor_band_index = 0; sensor_band_index < num_sensor_bands;
            sensor_band_index++)
    {
        /* Get band attributes because we need to scale the sampling
           time for the Pan band, which samples twice per frame where
           MS samples once per frame */
        band_attribs = ias_sat_attr_get_band_attributes(
                sensor_bands[sensor_band_index]);
        if (band_attribs == NULL)
        {
            IAS_LOG_ERROR("Getting band attributes for band %d",
                    sensor_bands[sensor_band_index]);
            return ERROR;
        }

        /* Set the appropriate sampling time */
        if (band_attribs->band_type == IAS_PAN_BAND)
        {
            sensor->bands[band_attribs->band_index].sampling_char.sampling_time
                = base_sample_time / PAN_SCALE;
        }
        else
        {
            sensor->bands[band_attribs->band_index].sampling_char.sampling_time
                = base_sample_time;
        }
    }

    return SUCCESS;
}

/*************************************************************************
Name: smooth_frame_times

Purpose:  Smooth the frame times from epoch using a clock correction model.

Notes:
    local routine

RETURNS: SUCCESS or ERROR
**************************************************************************/
static int smooth_frame_times
(
    int num_frames,                 /* I: Number of image frames for sensor */
    double nominal_frame_time,      /* I: Nominal frame time for sensor */
    double frame_time_fill_offset_tolerance, /* I: Frame to frame tolerance */
    double frame_time_clock_drift_tolerance, /* I: Tolerance for clock drift */
    const char *sensor_name,        /* I: String name of the sensor for msg */
    double *time_from_epoch,        /* I/O: Time of each frame in image, to
                                       review and correct; size is num_frames */
    int *num_bad_frame_times        /* O: Number of frame times corrected */
)
{
    int frame_index;                    /* Frame counter for times from epoch */
    double frame_time_diff_from_nominal;
    int first_valid_frame_index = -1;   /* Index of first valid time code */
    double valid_frame_time_count;      /* norm[0][0] in Cal/Val ADD; number of
                                           valid frame times */
    double frame_index_diff_sum;        /* norm[0][1] in Cal/Val ADD; sum of
                                           frame index offsets from first
                                           valid frame */
    double frame_index_diff_squared_sum;/* norm[1][1] in Cal/Val ADD; sum of
                                           squares of frame_index_diff_sum's */
    double time_diff_sum;               /* lhs[0] in Cal/Val ADD; sum of
                                           time_diff's */
    double frame_index_and_time_diff_product;   /* lhs[1] in Cal/Val ADD;
                                           product of offsets from first valid
                                           frame's index and time */
    double frame_index_diff;            /* Frame index offset from frame with
                                           first valid frame time */
    double time_diff;                   /* Frame time offset from frame with
                                           first valid frame time */
    double determinant;                 /* Temp var for time code smoothing */
    double offset;                      /* Clock correction model param */
    double time_rate;                   /* Clock correction model param */
    double original_time_from_epoch;    /* Temp variable for logging */
    double scaled_frame_time_tolerance; /* Scaled frame time fill offset 
                                           tolerance */
    int num_outlier_bad_frame_times = 0; /* Number of frame times that exceed
                                           threshold by a specified amount */

    /* Initialize the clock correction logic */
    valid_frame_time_count = 0.0;
    frame_index_diff_sum = 0.0;
    frame_index_diff_squared_sum = 0.0;
    time_diff_sum = 0.0;
    frame_index_and_time_diff_product = 0.0;

    for (frame_index = 1; frame_index < num_frames; frame_index++)
    {
        /* Compute the difference between [the frame time between the current
           and previous time codes] and [the nominal frame time] */
        frame_time_diff_from_nominal = fabs(time_from_epoch[frame_index]
                - time_from_epoch[frame_index - 1]
                - nominal_frame_time);

        /* Find first valid time code by finding first adjacent pair with valid
           frame time */
        if ((first_valid_frame_index < 0)
                && (frame_time_diff_from_nominal
                    <= frame_time_fill_offset_tolerance))
        {
            first_valid_frame_index = frame_index - 1;
            valid_frame_time_count += 1.0;
        }

        /* Accumulate the correction model data */
        if ((first_valid_frame_index >= 0)
                && (frame_time_diff_from_nominal
                    < frame_time_clock_drift_tolerance))
        {
            frame_index_diff = (double)(frame_index - first_valid_frame_index);
            time_diff = time_from_epoch[frame_index]
                - time_from_epoch[first_valid_frame_index];
            valid_frame_time_count += 1.0;
            frame_index_diff_sum += frame_index_diff;
            frame_index_diff_squared_sum += frame_index_diff * frame_index_diff;
            time_diff_sum += time_diff;
            frame_index_and_time_diff_product += frame_index_diff * time_diff;
        }
    }

    if (first_valid_frame_index < 0)
    {
        IAS_LOG_ERROR("No valid image time codes found");
        return ERROR;
    }

    /* Compute the clock correction model parameters */
    determinant = valid_frame_time_count * frame_index_diff_squared_sum
        - frame_index_diff_sum * frame_index_diff_sum;
    if (fabs(determinant) > 0.0)
    {
        /* Solve for the linear clock model parameters */
        offset = time_from_epoch[first_valid_frame_index]
            + ((frame_index_diff_squared_sum * time_diff_sum
                        - frame_index_diff_sum
                        * frame_index_and_time_diff_product) / determinant);
        time_rate = (valid_frame_time_count * frame_index_and_time_diff_product
                - frame_index_diff_sum * time_diff_sum) / determinant;
    }
    else
    {
        IAS_LOG_ERROR("Clock correction model failed - not enough valid "
                "time codes found");
        return ERROR;
    }

    /* Calculate a larger offset tolerance that will only be exceeded by
       particularly bad frame times */
    scaled_frame_time_tolerance = TOLERANCE_FACTOR 
        * frame_time_fill_offset_tolerance;

    /* Use the clock correction model to replace bad time codes */
    for (frame_index = 0; frame_index < num_frames; frame_index++)
    {
        double corrected_frame_time;

        corrected_frame_time = offset
            + (double)(frame_index - first_valid_frame_index) * time_rate;
        frame_time_diff_from_nominal
            = fabs(corrected_frame_time - time_from_epoch[frame_index]);

        /* If the tolerance threshold is exceeded, replace the current time
           code with the model value */
        if (frame_time_diff_from_nominal > frame_time_fill_offset_tolerance)
        {
            (*num_bad_frame_times)++;
            original_time_from_epoch = time_from_epoch[frame_index];
            time_from_epoch[frame_index] = corrected_frame_time;

            IAS_LOG_DEBUG("Corrected %s time code %d from: %lf, to: %lf",
                    sensor_name, frame_index, original_time_from_epoch,
                    time_from_epoch[frame_index]);

            /* If the tolerance threshold is exceeded by this amount, track this
               since it is a stronger indication of a potential problem */ 
            if (frame_time_diff_from_nominal > scaled_frame_time_tolerance)
            {
                num_outlier_bad_frame_times++;
            }
        }
    }

    /* Print how many bad frame times were found if there were any */ 
    if (*num_bad_frame_times > 0)
    {
        IAS_LOG_INFO("Corrected %d out of %d time codes for %s sensor", 
            *num_bad_frame_times, num_frames, sensor_name); 
    }

    /* Print how many very bad frame times were found if there were any */ 
    if (num_outlier_bad_frame_times > 0)
    {
        IAS_LOG_WARNING("There were %d out of %d time codes that exceeded the "
            "frame time fill offset tolerance by %1.1f times for %s sensor", 
            num_outlier_bad_frame_times, num_frames, TOLERANCE_FACTOR,
            sensor_name); 
    }

    return SUCCESS;
}

/*************************************************************************
Name: set_oli_l0r

Purpose: Set various values into the model from the frame headers.

Notes:
    local routine

    This routine is very similar to set_tirs_l0r. They have the basic same
    logic for setting up the model with information from the frame headers,
    just using the OLI rather than the TIRS frame headers. If either routine
    is modified, it is likely the other routine needs a very similar
    modification.

RETURNS: SUCCESS or ERROR
**************************************************************************/
static int set_oli_l0r
(
    IAS_CPF *cpf,                       /* I: CPF parameters */
    OLI_SENSOR_L0R_DATA *sensor_data,   /* I: Frame headers, num frames, etc. */
    const IAS_MATH_LEAP_SECONDS_DATA *leap_seconds_data, /* I: Leap seconds */
    IAS_SENSOR_MODEL *sensor            /* O: Model structure to populate */
)
{
    double time[3];                     /* Year, day of year, seconds of day */
    double *time_from_epoch = NULL;     /* Time of each frame in image */
    double sample_time;                 /* Time between samples (cur - last) */
    double epoch_time;                  /* Seconds from J2000 of first frame */
    double frame_time_fill_offset_tolerance; /* Frame to frame tolerance */
    double frame_time_clock_drift_tolerance; /* Tolerance for clock drift */
    int status;                         /* Routine status */
    int sensor_bands[IAS_MAX_NBANDS];   /* Array of band numbers for sensor */
    int num_sensor_bands;               /* Number of bands in sensor_bands */
    int sensor_band_index;              /* Index for sensor_bands */
    int frame_index;                    /* Frame counter for times from epoch */
    int band_index;                     /* Band array index for looping */
    const struct IAS_CPF_OLI_PARAMETERS *oli_params;
                                        /* Pointer to CPF OLI parameters */

    /* Get the OLI parameters from the CPF */
    oli_params = ias_cpf_get_oli_parameters(cpf);
    if (!oli_params)
    {
        IAS_LOG_ERROR("Failed to retrieve OLI parameters from the CPF");
        return ERROR;
    }

    /* Get the frame time tolerances */
    frame_time_fill_offset_tolerance
        = oli_params->frame_time_fill_offset_tolerance;
    frame_time_clock_drift_tolerance
        = oli_params->frame_time_clock_drift_tolerance;

    /* Set up memory for the frame times from epoch times. */
    status = setup_frames(IAS_OLI, sensor_data->num_frames, sensor);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Setting up frames information for OLI");
        return ERROR;
    }
    time_from_epoch = sensor->frame_seconds_from_epoch[IAS_OLI];

    /* Set the first frame time outside of the loop since the rest of
       the looping below calculates sampling time between frames */
    time_from_epoch[0] = (double)
        ((int) sensor_data->frame_headers[0].l0r_time.days_from_J2000
         * IAS_SEC_PER_DAY)
        + (double) sensor_data->frame_headers[0].l0r_time.seconds_of_day;

    /* Initialize the sensor array of frame seconds from UTC epoch */
    for (frame_index = 1; frame_index < sensor_data->num_frames; frame_index++)
    {
        time_from_epoch[frame_index]
            = (double)
            ((int)
             sensor_data->frame_headers[frame_index].l0r_time.days_from_J2000
             * IAS_SEC_PER_DAY)
            + (double)
            sensor_data->frame_headers[frame_index].l0r_time.seconds_of_day;
    }

    sensor_data->num_bad_frame_times = 0;

    /* Smooth the frame times from epoch */
    status = smooth_frame_times(sensor_data->num_frames,
            sensor_data->nominal_frame_time, frame_time_fill_offset_tolerance,
            frame_time_clock_drift_tolerance, "OLI", time_from_epoch, 
            &sensor_data->num_bad_frame_times);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Smoothing frame times for OLI");
        return ERROR;
    }

    /* Set the UTC epoch time information from the first frame */
    status = ias_math_init_leap_seconds(time_from_epoch[0], leap_seconds_data);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Initializing leap seconds");
        return ERROR;
    }

    /* Convert the total seconds from J2000 to year, day of year,
       seconds of day for our storage of the information in the model */
    status = ias_math_convert_j2000_seconds_to_year_doy_sod(time_from_epoch[0],
            time);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Converting J2000 seconds %lf to Year, DOY, SOD format",
                time_from_epoch[0]);
        return ERROR;
    }

    /* Retrieve the list of bands for this sensor from the
       satellite attributes */ 
    status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, IAS_NORMAL_BAND, 0,
            &sensor_bands[0], IAS_MAX_NBANDS, &num_sensor_bands);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Getting bands for sensor");
        return ERROR;
    }

    /* Setup the UTC epoch time for OLI bands from first frame header.
       Stored per band, but the epoch time is the same for all bands
       from the same sensor. */
    for (sensor_band_index = 0; sensor_band_index < num_sensor_bands;
            sensor_band_index++)
    {
        /* Get the band index for the model from the band number */
        band_index = ias_sat_attr_convert_band_number_to_index(
                sensor_bands[sensor_band_index]);

        sensor->bands[band_index].utc_epoch_time[0] = time[0];
        sensor->bands[band_index].utc_epoch_time[1] = time[1];
        sensor->bands[band_index].utc_epoch_time[2] = time[2];
    }

    epoch_time = time_from_epoch[0];
    for (frame_index = 0; frame_index < sensor_data->num_frames; frame_index++)
    {
        /* Convert all frame times to seconds from image start (image epoch) */
        /* Need to round to the nearest microseconds due to doing math on these
           large numbers with fractional components can result in the double
           register in the end being messed up a bit. We are only interested in
           accuracy to whole microseconds and up to there of the double
           register is accurate, so round to there. */
        time_from_epoch[frame_index] = (int)(((time_from_epoch[frame_index]
                        - epoch_time) * 1.0e6) + 0.5) * 1.0e-6;
    }

    /* Calculate the sample time over the range of times */
    sample_time = (time_from_epoch[sensor_data->num_frames - 1]
            - time_from_epoch[0]) / (double)(sensor_data->num_frames - 1);

    status = set_sampling_time(IAS_OLI, sample_time, sensor);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Setting sampling time for OLI");
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************
Name: set_tirs_l0r

Purpose: Set various values into the model from the frame headers.

Notes:
    local routine

    This routine is very similar to set_oli_l0r. They have the basic same
    logic for setting up the model with information from the frame headers,
    just using the TIRS rather than the OLI frame headers. If either routine
    is modified, it is likely the other routine needs a very similar
    modification.

RETURNS: SUCCESS or ERROR
**************************************************************************/
static int set_tirs_l0r
(
    IAS_CPF *cpf,                        /* I: CPF for SSM related fields */
    TIRS_SENSOR_L0R_DATA *sensor_data,  /* I: Frame headers, num frames, etc. */
    const IAS_MATH_LEAP_SECONDS_DATA *leap_seconds_data, /* I: Leap seconds */
    IAS_SENSOR_MODEL *sensor            /* O: Model structure to populate */
)
{
    double time[3];                     /* Year, day of year, seconds of day */
    double *time_from_epoch = NULL;     /* Time of each frame in image */
    double sample_time;                 /* Time between samples (cur - last) */
    double epoch_time;                  /* Seconds from J2000 of first frame */
    double frame_time_fill_offset_tolerance; /* Frame to frame tolerance */
    double frame_time_clock_drift_tolerance; /* Tolerance for clock drift */
    int status;                         /* Routine status */
    int sensor_bands[IAS_MAX_NBANDS];   /* Array of band numbers for sensor */
    int num_sensor_bands;               /* Number of bands in sensor_bands */
    int sensor_band_index;              /* Index for sensor_bands */
    int frame_index;                    /* Frame counter for times from epoch */
    int band_index;                     /* Band array index for looping */
    const struct IAS_CPF_TIRS_PARAMETERS *tirs_params; 
                                        /* Pointer to CPF TIRS parameters */

    /* Get the TIRS parameters from the CPF */
    tirs_params = ias_cpf_get_tirs_parameters(cpf);
    if (!tirs_params)
    {
        IAS_LOG_ERROR("Failed to retrieve TIRS parameters from the CPF");
        return ERROR;
    }

    /* Get the frame time tolerances */
    frame_time_fill_offset_tolerance
        = tirs_params->frame_time_fill_offset_tolerance;
    frame_time_clock_drift_tolerance
        = tirs_params->frame_time_clock_drift_tolerance;

    /* Set up memory for the frame times from epoch times. */
    status = setup_frames(IAS_TIRS, sensor_data->num_frames, sensor);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Setting up frames information for TIRS");
        return ERROR;
    }
    time_from_epoch = sensor->frame_seconds_from_epoch[IAS_TIRS];

    /* Set the first frame time outside of the loop since the rest of
       the looping below calculates sampling time between frames */
    time_from_epoch[0] = (double)
        ((int) sensor_data->frame_headers[0].l0r_time.days_from_J2000
         * IAS_SEC_PER_DAY)
        + (double) sensor_data->frame_headers[0].l0r_time.seconds_of_day;

    /* Set the sensor array of frame seconds from UTC epoch and find
       sampling time */
    for (frame_index = 1; frame_index < sensor_data->num_frames; frame_index++)
    {
        time_from_epoch[frame_index]
            = (double)
            ((int)
             sensor_data->frame_headers[frame_index].l0r_time.days_from_J2000
             * IAS_SEC_PER_DAY)
            + (double)
            sensor_data->frame_headers[frame_index].l0r_time.seconds_of_day;
    }

    sensor_data->num_bad_frame_times = 0;

    /* Smooth the frame times from epoch */
    status = smooth_frame_times(sensor_data->num_frames,
            sensor_data->nominal_frame_time, frame_time_fill_offset_tolerance,
            frame_time_clock_drift_tolerance, "TIRS", time_from_epoch, 
            &sensor_data->num_bad_frame_times);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Smoothing frame times for TIRS");
        return ERROR;
    }

    /* Set the UTC epoch time information from the first frame */
    status = ias_math_init_leap_seconds(time_from_epoch[0], leap_seconds_data);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Initializing leap seconds");
        return ERROR;
    }

    /* Convert the total seconds from J2000 to year, day of year,
       seconds of day for our storage of the information in the model */
    status = ias_math_convert_j2000_seconds_to_year_doy_sod(time_from_epoch[0],
            time);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Converting J2000 seconds %lf to Year, DOY, SOD format",
                time_from_epoch[0]);
        return ERROR;
    }

    /* Retrieve the list of bands for this sensor from the
       satellite attributes */ 
    status = ias_sat_attr_get_sensor_band_numbers(IAS_TIRS, IAS_NORMAL_BAND, 0,
            &sensor_bands[0], IAS_MAX_NBANDS, &num_sensor_bands);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Getting bands for sensor");
        return ERROR;
    }

    /* Setup the UTC epoch time for TIRS bands from first frame header.
       Stored per band, but the epoch time is the same for all bands
       from the same sensor. */
    for (sensor_band_index = 0; sensor_band_index < num_sensor_bands;
            sensor_band_index++)
    {
        /* Get the band index for the model from the band number */
        band_index = ias_sat_attr_convert_band_number_to_index(
                sensor_bands[sensor_band_index]);

        sensor->bands[band_index].utc_epoch_time[0] = time[0];
        sensor->bands[band_index].utc_epoch_time[1] = time[1];
        sensor->bands[band_index].utc_epoch_time[2] = time[2];
    }

    epoch_time = time_from_epoch[0];
    for (frame_index = 0; frame_index < sensor_data->num_frames; frame_index++)
    {
        /* Convert all frame times to seconds from image start (image epoch) */
        /* Need to round to the nearest microseconds due to doing math on these
           large numbers with fractional components can result in the double
           register in the end being messed up a bit. We are only interested in
           accuracy to whole microseconds and up to there of the double
           register is accurate, so round to there. */
        time_from_epoch[frame_index] = (int)(((time_from_epoch[frame_index]
                        - epoch_time) * 1.0e6) + 0.5) * 1.0e-6;
    }

    /* Calculate the sample time over the range of times */
    sample_time = (time_from_epoch[sensor_data->num_frames - 1]
            - time_from_epoch[0]) / (double)(sensor_data->num_frames - 1);

    status = set_sampling_time(IAS_TIRS, sample_time, sensor);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Setting sampling time for TIRS");
        return ERROR;
    }

    /* Add the SSM related information to the model */
    status = ias_sensor_set_ssm_from_l0r(sensor_data, cpf, sensor, IAS_TIRS);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Setting SSM data for TIRS");
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************
Name: set_integration_and_offsets

Purpose: Set the integration time and L0R detector offsets into the model.

Notes:
    local routine
    The nominal integration time was previously set in the model from the CPF.
    Override the nominal time with actual integration time from the L0R, if
    available and valid.

RETURNS: SUCCESS or ERROR
**************************************************************************/
static int set_integration_and_offsets
(
    IAS_CPF *cpf,                    /* I: CPF SSM info and tolerance checks */
    IAS_SENSOR_L0R_DATA *l0r_data,  /* I: L0R data to create a model */
    IAS_SENSOR_MODEL *sensor        /* O: Model structure to populate */
)
{
    const IAS_BAND_ATTRIBUTES *band_attribs; /* Satellite bands attributes */
    int band_index;                     /* Band array index for looping */
    int sca_index;                      /* SCA counter for looping */
    int num_detectors;                  /* Number detectors per SCA for band */
    int detector;                       /* Detector counter for looping */
    int det_index;
    int i;                              /* Generic looping counter */
    int tirs_integration_set = 0;       /* To track if found int time in hdr */
    int tirs_nominal_message_output = 0;
    double tirs_integration_time = 0.0;
    int ms_message_output = 0;
    int ms_nominal_message_output = 0;
    const struct IAS_CPF_OLI_PARAMETERS *oli_params;
    const struct IAS_CPF_TIRS_PARAMETERS *tirs_params; 

    /* get the OLI and TIRS parameters from the CPF */
    oli_params = ias_cpf_get_oli_parameters(cpf);
    if (!oli_params)
    {
        IAS_LOG_ERROR("Retrieving CPF OLI parameters");
        return ERROR;
    }
    tirs_params = ias_cpf_get_tirs_parameters(cpf);
    if (!tirs_params)
    {
        IAS_LOG_ERROR("Failed to retrieve TIRS parameters from the CPF");
        return ERROR;
    }

    /* Set the integration times, and L0R detector offsets per band, per SCA */
    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        /* Get band attributes because the integration time is saved in
           different components of the frame header for MS vs. Pan vs.
           TIRS bands */
        band_attribs = ias_sat_attr_get_band_attributes(
                ias_sat_attr_convert_band_index_to_number(band_index));
        if (!band_attribs)
        {
            IAS_LOG_ERROR("Unable to get band attributes for band index %d",
                          band_index);
            return ERROR;
        }

        if (band_attribs->sensor_id == IAS_OLI)
        {
            /* Set the integration times per band */
            if (band_attribs->band_type == IAS_PAN_BAND)
            {
                if (l0r_data->image_header_flag
                        && l0r_data->image_header.pan_integration_time > 0)
                {
                    /* The image header was available and it has valid values
                       for the integration time, so use this value to replace
                       in the model the previous nominal integration time set
                       from the CPF.  Convert the time from microseconds in
                       the L0R to seconds. */
                    sensor->bands[band_index].sampling_char.integration_time
                        = l0r_data->image_header.pan_integration_time
                        * oli_params->integration_time_scale;
                }
                else
                {
                    IAS_LOG_WARNING("PAN integration time is 0. Using nominal"
                        " value from the CPF");
                }

                /* if the integration time is not within the nominal
                   integration time +/- the integration time tolerance, issue a
                   warning message, but use it anyway */
                if (fabs(
                      sensor->bands[band_index].sampling_char.integration_time
                      - oli_params->nominal_integration_time_pan * 1.0e-3)
                    > oli_params->integration_time_tolerance * 1.0e-3)
                {
                    IAS_LOG_WARNING("PAN integration time of %f exceeds the "
                        "tolerance of %f from the nominal integration time "
                        "of %f", sensor->bands[band_index].sampling_char
                        .integration_time,
                        oli_params->integration_time_tolerance * 1.0e-3,
                        oli_params->nominal_integration_time_pan * 1.0e-3);
                }
            }
            else
            {
                if (l0r_data->image_header_flag
                        && l0r_data->image_header.ms_integration_time > 0)
                {
                    /* The image header was available and it has valid values
                       for the integration time, so use this value to replace
                       in the model the previous nominal integration time set
                       from the CPF.  Convert the time from microseconds in
                       the L0R to seconds. */
                    sensor->bands[band_index].sampling_char.integration_time
                        = l0r_data->image_header.ms_integration_time
                        * oli_params->integration_time_scale;
                }
                else
                {
                    if (!ms_nominal_message_output)
                    {
                        ms_nominal_message_output = 1;
                        IAS_LOG_WARNING("OLI multispectral integration time"
                        " is 0. Using nominal value from the CPF");
                    }
                }

                /* if the integration time is not within the nominal
                   integration time +/- the integration time tolerance, issue a
                   warning message, but use it anyway.  Note that the message
                   should only be output once. */
                if (!ms_message_output)
                {
                    ms_message_output = 1;

                    if (fabs(
                      sensor->bands[band_index].sampling_char.integration_time
                          - oli_params->nominal_integration_time_ms * 1.0e-3)
                        > oli_params->integration_time_tolerance * 1.0e-3)
                    {
                        IAS_LOG_WARNING("OLI multispectral integration time of "
                            "%f exceeds the tolerance of %f from the nominal "
                            "integration time of %f",
                            sensor->bands[band_index].sampling_char
                            .integration_time,
                            oli_params->integration_time_tolerance * 1.0e-3,
                            oli_params->nominal_integration_time_ms * 1.0e-3);
                    }
                }
            }
        }

        if (band_attribs->sensor_id == IAS_TIRS)
        {
            if (tirs_integration_set)
            {
                sensor->bands[band_index].sampling_char.integration_time
                    = tirs_integration_time;
            }
            else if (l0r_data->tirs_data.num_frames > 0)
            {
                /* TIRS integration time not yet found, so find it now */

                double prev_integration_time
                    = l0r_data->tirs_data.frame_headers[0].integration_duration;

                /* Find the first two consecutive non-zero TIRS integration
                   times and use that for the integration time.  If no valid
                   value is found, it will leave the value set to the
                   nominal time that has already been set from the CPF */
                tirs_integration_time
                    = sensor->bands[band_index].sampling_char.integration_time;
                for (i = 1; i < l0r_data->tirs_data.num_frames; i++)
                {
                    double integration_time = l0r_data->tirs_data
                        .frame_headers[i].integration_duration;

                    if ((integration_time > 0)
                        && (integration_time == prev_integration_time))
                    {
                        /* Convert the time to seconds from microseconds */
                        tirs_integration_time = integration_time
                            * tirs_params->integration_time_scale;
                        sensor->bands[band_index].sampling_char.integration_time
                            = tirs_integration_time;
                        tirs_integration_set = 1;
                        break;
                    }
                    prev_integration_time = integration_time;
                }

                if (tirs_integration_set == 0)
                {
                    if (tirs_nominal_message_output == 0)
                    {
                        tirs_nominal_message_output = 1;
                        IAS_LOG_WARNING("Two consecutive TIRS integration time"
                        " values not found. Using nominal value from the CPF");
                    }
                }

                /* issue a warning if the integration time is not within
                   tolerance */
                if (fabs(tirs_integration_time
                         - tirs_params->nominal_integration_time * 1.0e-3)
                    > tirs_params->integration_time_tolerance * 1.0e-3)
                {
                    IAS_LOG_WARNING("TIRS integration time of %f exceeds the "
                        "tolerance of %f from the nominal integration time "
                        "of %f", tirs_integration_time, 
                        tirs_params->integration_time_tolerance * 1.0e-3,
                        tirs_params->nominal_integration_time * 1.0e-3);
                }
            }
        }

        /* Only set the detector offsets if the sensor for this band was
           collected for this image */
        if ((band_attribs->sensor_id == IAS_OLI
                    && sensor->sensors[IAS_OLI].sensor_present)
                || (band_attribs->sensor_id == IAS_TIRS
                    && sensor->sensors[IAS_TIRS].sensor_present))
        {
            for (sca_index = 0; sca_index < sensor->bands[band_index].sca_count;
                    sca_index++)
            {
                IAS_SENSOR_SCA_MODEL *sca;

                sca = &sensor->bands[band_index].scas[sca_index];
                num_detectors
                    = sensor->bands[band_index].scas[sca_index].detectors;

                /* Copy the detector offsets from the L0R inputs structure
                   into the model */
                for (detector = 0; detector < num_detectors; detector++)
                {
                    /* Now just save the value from the provided L0R fill array
                       for this band into the model structure. */
                    det_index = (sca_index * num_detectors) + detector;

                    sca->l0r_detector_offsets[detector]
                        = l0r_data->l0r_detector_offsets[band_index][det_index];
                }

                /* Update the nominal fill offset for the case of image
                   with no fill */
                /* Details (from Cal/Val):
                   Since OLI detectors can deselect in either direction
                   (requiring either more or less fill), the fill required for
                   a nominal detector (the set modeled by the Legendre
                   polynomials) must be large enough to ensure that none of
                   the individual detector fill values go negative. This has
                   the effect of pushing the detectors modeled by the Legendre
                   polynomials into the image by the amount of nominal fill.
                   This has to be taken into account by the model. If no fill
                   is inserted in the L0R image this correction is not needed
                   and should not be made, so the nominal detector fill values
                   read out of the CPF are overridden by zeros.
                   The assumption for TIRS was that we would never insert
                   detector alignment fill in the L0R image, so the whole
                   issue could be avoided, but Cal/Val and the developers
                   ended up agreeing this logic should be allowed to apply
                   for all sensors. */
                if ((sca->l0r_detector_offsets[0] == 0)
                        && (sca->l0r_detector_offsets[1] == 0))
                    sca->nominal_fill = 0;
            }
        }
    }

    return SUCCESS;
}

/*************************************************************************
Name: ias_sensor_set_l0r

Purpose: Set the L0R values related to the sensor into the model structure.
    Farms out the tasks to sub-routines, some that are common to all
    sensors, and some that need to be sensor specific.

RETURNS: SUCCESS or ERROR
**************************************************************************/
int ias_sensor_set_l0r
(
    IAS_CPF *cpf,                    /* I: CPF SSM info and tolerance checks */
    IAS_SENSOR_L0R_DATA *l0r_data,  /* I: L0R data to create a model */
    IAS_SENSOR_MODEL *sensor        /* O: Model structure to populate */
)
{
    int status;                         /* General routine return status */

    set_band_and_sensor_flags(l0r_data, sensor);

    /* Set the OLI frame times and sampling times only if the OLI sensor
       is actually in this L0R. */
    if (sensor->sensors[IAS_OLI].sensor_present)
    {
        status = set_oli_l0r(cpf, &l0r_data->oli_data,
                l0r_data->leap_seconds_data, sensor);
        if (status == ERROR)
        {
            IAS_LOG_ERROR("Setting OLI L0R frame times to model");
            return ERROR;
        }
    }
    /* Set the TIRS frame times and sampling times only if the TIRS sensor
       is actually in this L0R. */
    if (sensor->sensors[IAS_TIRS].sensor_present)
    {
        status = set_tirs_l0r(cpf, &l0r_data->tirs_data,
                l0r_data->leap_seconds_data, sensor);
        if (status == ERROR)
        {
            IAS_LOG_ERROR("Setting TIRS L0R frame times to model");
            return ERROR;
        }
    }
    else
    {
        /* since TIRS isn't present, release the SSM model for it */
        free(sensor->sensors[IAS_TIRS].ssm_model);
        sensor->sensors[IAS_TIRS].ssm_model = NULL;
    }

    status = set_integration_and_offsets(cpf, l0r_data, sensor);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Setting integration times and L0R offsets");
        return ERROR;
    }

    /* Only does something if log level set to IAS_LOG_LEVEL_DEBUG */
    if (IAS_LOG_DEBUG_ENABLED())
        log_l0r_values(sensor);

    return SUCCESS;
}

/*************************************************************************
Name: log_l0r_values

Purpose: Prints debugging log data for the L0R source values.

RETURNS: nothing
**************************************************************************/
static void log_l0r_values
(
    IAS_SENSOR_MODEL *sensor          /* I: Model structure */
)
{
    int band_index;                     /* Band array index for looping */
    int frame_index;                    /* Frame counter for times from epoch */
    int sca_index;                      /* SCA counter for looping */
    int detector;                       /* Detector counter for looping */

    /*== DEBUG LOGGING ======================================================*/
    IAS_LOG_DEBUG("====> SET L0R TO MODEL REPORT <====");

    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        IAS_LOG_DEBUG("Band present flag for band %d: %d", band_index,
                sensor->bands[band_index].band_present);
        IAS_LOG_DEBUG("Set frame count to %d for band %d",
                sensor->bands[band_index].frame_count, band_index);
    }

    /* ==> OLI */
    IAS_LOG_DEBUG("OLI present flag: %d",
            sensor->sensors[IAS_OLI].sensor_present);
    /* only print debug info if sensor is present */
    if (sensor->sensors[IAS_OLI].sensor_present == 1)
    {
        IAS_LOG_DEBUG("OLI frame count set to %d",
                      sensor->frame_counts[IAS_OLI]);
        for (band_index = 0; band_index < sensor->band_count; band_index++)
        {
            if (sensor->bands[band_index].sensor->sensor_id == IAS_OLI)
            {
                IAS_LOG_DEBUG("UTC epoch date/time for band %d set to: "
                    "Year: %d Day of Year: %d Seconds of Day: %lf",
                    band_index,
                    (int)sensor->bands[band_index].utc_epoch_time[0],
                    (int)sensor->bands[band_index].utc_epoch_time[1],
                    sensor->bands[band_index].utc_epoch_time[2]);
            }
        }
        for (frame_index = 0; frame_index < 10; frame_index++)
        {
            IAS_LOG_DEBUG("OLI frame time from epoch for frame %d: %lf",
                frame_index,
                sensor->frame_seconds_from_epoch[IAS_OLI][frame_index]);
        }
        for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
        {
            if (sensor->bands[band_index].sensor->sensor_id == IAS_OLI
                    && sensor->bands[band_index].band_present)
            {
                for (frame_index = 0; frame_index < 10; frame_index++)
                {
                    IAS_LOG_DEBUG(
                            "Frame time for OLI band %d for frame %d: %lf",
                            band_index, frame_index, sensor->bands[band_index].
                            frame_seconds_from_epoch[frame_index]);
                }
            }
        }
        for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
        {
            if (sensor->bands[band_index].sensor->sensor_id == IAS_OLI
                    && sensor->bands[band_index].band_present)
            {
                IAS_LOG_DEBUG("Sampling time for OLI band %d: %lf", band_index,
                        sensor->bands[band_index].sampling_char.sampling_time);
            }
        }
    }

    /* ==> TIRS */
    IAS_LOG_DEBUG("TIRS present flag: %d",
            sensor->sensors[IAS_TIRS].sensor_present);
    if (sensor->sensors[IAS_TIRS].sensor_present == 1)
    {
        IAS_LOG_DEBUG("TIRS frame count set to %d",
                      sensor->frame_counts[IAS_TIRS]);
        for (band_index = 0; band_index < sensor->band_count; band_index++)
        {
            if (sensor->bands[band_index].sensor->sensor_id == IAS_TIRS)
            {
                IAS_LOG_DEBUG("UTC epoch date/time for band %d set to: "
                    "Year: %d Day of Year: %d Seconds of Day: %lf", band_index,
                    (int)sensor->bands[band_index].utc_epoch_time[0],
                    (int)sensor->bands[band_index].utc_epoch_time[1],
                    sensor->bands[band_index].utc_epoch_time[2]);
            }
        }
        for (frame_index = 0; frame_index < 10; frame_index++)
        {
            IAS_LOG_DEBUG("TIRS frame time from epoch for frame %d: %lf",
                    frame_index,
                    sensor->frame_seconds_from_epoch[IAS_TIRS][frame_index]);
        }
        for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
        {
            if (sensor->bands[band_index].sensor->sensor_id == IAS_TIRS
                    && sensor->bands[band_index].band_present)
            {
                for (frame_index = 0; frame_index < 10; frame_index++)
                {
                    IAS_LOG_DEBUG("Frame time for band %d for frame %d: %lf",
                        band_index, frame_index, sensor->bands[band_index].
                        frame_seconds_from_epoch[frame_index]);
                }
            }
        }
        for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
        {
            if (sensor->bands[band_index].sensor->sensor_id == IAS_TIRS
                    && sensor->bands[band_index].band_present)
            {
                IAS_LOG_DEBUG("Sampling time for TIRS band %d: %lf", band_index,
                        sensor->bands[band_index].sampling_char.sampling_time);
            }
        }
    }

    /* ==> All sensors */
    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        for (sca_index = 0; sca_index < 2; sca_index++)
        {
            for (detector = 0; detector < 5; detector++)
            {
                IAS_LOG_DEBUG("Band %d, SCA %d, Detector %d; "
                        "L0R detector offset: %d", band_index, sca_index,
                        detector, sensor->bands[band_index].scas[sca_index].
                        l0r_detector_offsets[detector]);
            }
        }
        IAS_LOG_DEBUG("Integration time for band %d: %lf", band_index,
                sensor->bands[band_index].sampling_char.integration_time);
    }
    /*== END DEBUG ==========================================================*/
}
