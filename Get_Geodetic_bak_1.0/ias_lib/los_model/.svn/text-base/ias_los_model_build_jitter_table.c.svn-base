/****************************************************************************
NAME:           ias_los_model_build_jitter_table

PURPOSE:  
Jitter accomodation routine that preprocesses the attitude data to separate
it into low frequency attitude (used to build the grid) and high frequency
jitter components. Module does the following:
1)  Create a copy of the full resolution (20 msec) attitude sequence.
2)  Low pass filter the attitude sequence to 1 Hz bandwidth.
3)  Subtract the low pass sequence from the original sequence to create a
    20 msec high frequency sequence.
4)  Transfer any residual bias in the high pass sequence to the low pass
    sequence.
5)  Store the resulting low pass sequence in the model attitude group.
6)  Loop through the image lines to compute the high frequency attitude
    for each line. This is done using ias_sensor_find_time (to get the line
    time) and ias_sc_model_find_attitude_at_time (using the HF sequence as
    input) to do the linear interpolation to the line time.
7)  Store the resulting jitter table in the OLI model.

RETURNS: SUCCESS or ERROR

*****************************************************************************/
#include <stdlib.h>
#include "ias_logging.h"
#include "ias_types.h"
#include "ias_satellite_attributes.h"
#include "ias_los_model.h"

static int setup_reference_band_indices(IAS_LOS_MODEL *model,
        int ref_band_index[IAS_MAX_SENSORS]);

int ias_los_model_build_jitter_table
(
    IAS_CPF *cpf,               /*I: CPF structure pointer */
    IAS_LOS_MODEL *model        /* I/O: Model structure to separate low/high
                                        attitude data within */
)
{
    const IAS_CPF_GEO_SYSTEM *cpf_geo_sys; /* CPF GEO_SYSTEM group for cutoff */
    IAS_SC_ATTITUDE_MODEL orig;     /* Original attitude model */
    IAS_SC_ATTITUDE_MODEL low;      /* Low pass attitude model */
    IAS_SC_ATTITUDE_MODEL high;     /* High pass attitude model */
    int status;                     /* Routine return status value */
    int sensor_id;                  /* IAS_SENSOR_ID */
    int num_lines;                  /* Number of image lines */
    int time_is_initialized;        /* Flag to help with setting start and
                                       stop times */
    int ref_band_index[IAS_MAX_SENSORS];
                                    /* Band index per sensor to use in
                                       computations that use sensor-level
                                       data */
    double low_pass_cutoff;         /* Low pass filter cutoff frequency
                                       from CPF */
    double start_time = 0;          /* Image start time relative to
                                       attitude epoch */
    double stop_time = 0;           /* Image stop time relative to
                                       attitude epoch */
    double delta_time[IAS_MAX_SENSORS]; /* Time between image and high epoch
                                           for each sensor */

    status = setup_reference_band_indices(model, ref_band_index);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting reference band indicesfor sensors");
        return ERROR;
    }

    /* Use the cutoff frequency specified in the CPF to separate the high
       frequency attitude data out from the original data. */
    cpf_geo_sys = ias_cpf_get_geo_system(cpf);
    if (cpf_geo_sys == NULL)
    {
        IAS_LOG_ERROR("Reading CPF for the low pass filter cutoff frequency");
        return ERROR;
    }
    low_pass_cutoff = cpf_geo_sys->low_pass_cutoff;

    /* Initialize the attitude structure for the original attitude sequence */
    ias_sc_model_initialize_attitude(&orig);

    /* Copy the original attitude data */
    status = ias_sc_model_copy_attitude(&model->spacecraft.attitude, &orig);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Making local copy of original attitude data");
        return ERROR;
    }

    /* Initialize the attitude structures for the separated low and high
       frequency sequences */
    ias_sc_model_initialize_attitude(&low);
    ias_sc_model_initialize_attitude(&high);

    /* Design a low pass filter with the desired cutoff frequency using
       the Remez exchange algorithm to design the filter and generate the
       filter weights. Then apply the filter to remove the high frequency
       portion from the original attitude, giving back the isolated low
       frequency attitude sequence. */
    status = ias_sc_model_remez_filter_attitude(&orig, low_pass_cutoff,
            &low);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Applying low pass filter (%lfHz) to attitude data",
                low_pass_cutoff);
        ias_sc_model_free_attitude(&orig);
        return ERROR;
    }

    /* Subtract the low pass filtered sequences from the original sequences
       to extract the high frequency portion of the data */
    status = ias_sc_model_subtract_attitude(&orig, &low, &high);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Error subtracting the low frequency attitude data");
        ias_sc_model_free_attitude(&orig);
        ias_sc_model_free_attitude(&low);
        return ERROR;
    }

    /* Determine the image start and stop times for each sensor */
    time_is_initialized = 0;
    for (sensor_id = 0; sensor_id < IAS_MAX_SENSORS; sensor_id++)
    {
        double cur_start_time;
        double cur_stop_time;

        /* Skip any sensor that is not present */
        if (model->sensor.sensors[sensor_id].sensor_present == 0)
            continue;

        /* Get the difference between the image start time for the reference
           band for this sensor (all are same) and the high attitude sequence
           start */
        status = ias_math_get_time_difference(
                model->sensor.bands[ref_band_index[sensor_id]].utc_epoch_time,
                high.utc_epoch_time, &delta_time[sensor_id]);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Calculating time difference between the attitude "
                    "and image epoch times for sensor %d", sensor_id);
            ias_sc_model_free_attitude(&orig);
            ias_sc_model_free_attitude(&low);
            ias_sc_model_free_attitude(&high);
            return ERROR;
        }

        cur_start_time = model->sensor.frame_seconds_from_epoch[sensor_id][0]
            + delta_time[sensor_id];
        cur_stop_time = model->sensor.frame_seconds_from_epoch[sensor_id]
            [model->sensor.frame_counts[sensor_id] - 1] + delta_time[sensor_id];

        /* Use the maxbox time frame over all sensors for the start and stop
           times in the transferring jitter bias routine */
        if (!time_is_initialized)
        {
            /* start and stop times were initialized at top just to eliminate
               a compiler warning, so initialize them here for real with the
               start and stop times of the first sensor that is present */
            start_time = cur_start_time;
            stop_time = cur_stop_time;
            time_is_initialized = 1;
        }
        else
        {
            if (cur_start_time < start_time)
                start_time = cur_start_time;
            if (cur_stop_time > stop_time)
                stop_time = cur_stop_time;
        }
    }

    /* Transfer any residual bias (non-zero mean value) from the high
       frequency sequence to the low frequency sequence */
    status = ias_sc_model_transfer_jitter_attitude_bias(start_time,
            stop_time, &high, &low);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Transferring residual bias from high frequency to "
                "low frequency attitude data");
        ias_sc_model_free_attitude(&orig);
        ias_sc_model_free_attitude(&low);
        ias_sc_model_free_attitude(&high);
        return ERROR;
    }

    /* Now "low" contains the attitude signal at frequency from the CPF
       and "high" contains the high frequency signal */

    /* Build the jitter table for the high frequency attitude sequence */
    for (sensor_id = 0; sensor_id < IAS_MAX_SENSORS; sensor_id++)
    {
        IAS_VECTOR *jitter_table;    /* Jitter model table pointer */
        double time;                 /* Time in imagery */
        int line;                    /* Line counter */
        IAS_SENSOR_BAND_MODEL *band_model
            = &model->sensor.bands[ref_band_index[sensor_id]];
        IAS_SENSOR_LOCATION_MODEL *sensor_loc
            = &model->sensor.sensors[sensor_id];

        /* Skip any sensor that is not present */
        if (sensor_loc->sensor_present == 0)
            continue;

        /* Calculate the table size */
        num_lines = band_model->frame_count
            * sensor_loc->jitter_entries_per_frame;

        /* Allocate the jitter table */
        jitter_table = (IAS_VECTOR *)malloc(num_lines * sizeof(IAS_VECTOR));
        if (jitter_table == NULL)
        {
            IAS_LOG_ERROR("Allocating the jitter model table");
            ias_sc_model_free_attitude(&orig);
            ias_sc_model_free_attitude(&low);
            ias_sc_model_free_attitude(&high);
            return ERROR;
        }

        /* Load the jitter table. Interpolate the high frequency sequence
           values at the line sampling times to create the model jitter table */
        for (line = 0; line < num_lines; line++)
        {
            IAS_VECTOR jit_attitude;    /* Interpolated attitude point, at
                                           current line time */
            int index;                  /* Index into frame seconds */
            int integration_sign;       /* Holds +1 or -1 for the sign of the
                                           integration time adjustment, based
                                           on whether time codes are collected
                                           at the start or end of the frame */
            IAS_SENSOR_DETECTOR_SAMPLING_CHARACTERISTICS *sampling_char
                = &band_model->sampling_char;

            /* Use the line and the sensor info to get to the index to the
               frame this line is in */
            index = line / sensor_loc->jitter_entries_per_frame;

            if (sampling_char->time_codes_at_frame_start)
                integration_sign = 1;
            else
                integration_sign = -1;

            /* Some of these components of this computation fall out for TIRS.
               Settling time for TIRS is 0, and the (line - jit_ent_frm * index)
               reduces to 0 also. For OLI however, this will add one more
               sample time for the 2nd pan line time for every other line to
               interpolate the pan line times. */
            time = model->sensor.frame_seconds_from_epoch[sensor_id][index]
                - sampling_char->settling_time
                + (integration_sign
                        * sampling_char->integration_time / 2.0)
                + (line - sensor_loc->jitter_entries_per_frame * index)
                * sampling_char->sampling_time;

            /* Convert to time from attitude epoch */
            time += delta_time[sensor_id];

            /* Interpolate high frequency roll-pitch-yaw values using four
               point Lagrange interpolation */
            status = ias_sc_model_find_jitter_attitude_at_time(&high, time,
                    &jit_attitude);
            if(status != SUCCESS)
            {
                IAS_LOG_ERROR("Finding attitude at time %f",time);
                ias_sc_model_free_attitude(&orig);
                ias_sc_model_free_attitude(&low);
                ias_sc_model_free_attitude(&high);
                free(jitter_table);
                return ERROR;
            }

            jitter_table[line].x = jit_attitude.x;
            jitter_table[line].y = jit_attitude.y;
            jitter_table[line].z = jit_attitude.z;
        }

        /* Add the high frequency attitude jitter table to the model */
        free(sensor_loc->jitter_table);
        sensor_loc->jitter_table = jitter_table;
        sensor_loc->jitter_table_count = num_lines;
    }

    /* Free the raw high frequency attitude sequence now that it's been
       interpolated to per-line jitter_table data. Also done with the copy
       of the original attitude. */
    ias_sc_model_free_attitude(&high);
    ias_sc_model_free_attitude(&orig);

    /* Replace the original model attitude data sequence with the low pass
       filtered attitude data sequence */
    status = ias_sc_model_copy_attitude(&low, &model->spacecraft.attitude);
    if (status != SUCCESS)
    {
        /* The low pass attitude sequence is now messed up */
        IAS_LOG_ERROR("Copying low frequency attitude data to model structure");
        ias_sc_model_free_attitude(&low);
        return ERROR;
    }

    /* Free the low attitude sequence */
    ias_sc_model_free_attitude(&low);

    return SUCCESS;
}

/*************************************************************************

Name: setup_reference_band_indices

Purpose: Get a reference band index to use per sensor for the places that
         building the jitter table needs to access band-level data. For OLI
         to work right, need to interpolate the attitude to the PAN band
         resolution of number of lines, so to generalize, get the band for
         each sensor that has the maximum lines_per_frame attribute. This will
         turn out to be the first band from get_sensor_band_numbers when all
         bands have the same lines_per_frame or the PAN band for OLI.
         jitter_entries_per_frame is then initialized per sensor to the maximum
         lines_per_frame value found for that sensor's present bands.

**************************************************************************/
static int setup_reference_band_indices
(
    IAS_LOS_MODEL *model,               /* I: Model structure for band info */
    int ref_band_index[IAS_MAX_SENSORS] /* O: Per sensor reference band index
                                              to use in computations that use
                                              sensor-level band_index */
)
{
    int status;                     /* Routine return status value */
    int sensor_id;                  /* IAS_SENSOR_ID */

    /* Get a reference band index per sensor present. */
    for (sensor_id = 0; sensor_id < IAS_MAX_SENSORS; sensor_id++)
    {
        int sensor_bands[IAS_MAX_NBANDS]; /* Sensor's band numbers */
        int num_bands;                    /* Number of sensor_bands */
        int max_lines_per_frame = 0;      /* Max lines_per_frame for bands */
        int i;

        /* Skip any sensor that is not present */
        if (model->sensor.sensors[sensor_id].sensor_present == 0)
            continue;

        /* Get the normal bands for this sensor */
        status = ias_sat_attr_get_sensor_band_numbers(sensor_id,
                IAS_NORMAL_BAND, 0, sensor_bands, IAS_MAX_NBANDS,
                &num_bands);
        if ((status != SUCCESS) || (num_bands < 1))
        {
            IAS_LOG_ERROR("Getting normal band numbers for sensor %d\n",
                    sensor_id);
            return ERROR;
        }

        /* Initialize this sensors reference band index to be able to later
           check if it got setup properly */
        ref_band_index[sensor_id] = -1;

        /* Determine which band to use as the reference and get its
           band index */
        for (i = 0; i < num_bands; i++)
        {
            /* Get the band index for the current band number */
            int band_index = ias_sat_attr_convert_band_number_to_index(
                    sensor_bands[i]);
            if (band_index == ERROR)
            {
                IAS_LOG_ERROR("Converting band number %d to index",
                        sensor_bands[i]);
                return ERROR;
            }

            /* If this band is present in the model and its lines_per_frame
               is greater than the maximum one for this sensor so far, then
               use this as the (new) reference band and update the maximum
               lines_per_frame. If all bands for this sensor have the same
               lines_per_frame, this results in the first band being the
               reference band. */
            if (model->sensor.bands[band_index].band_present &&
                    (model->sensor.bands[band_index].sampling_char
                     .lines_per_frame > max_lines_per_frame))
            {
                ref_band_index[sensor_id] = band_index;
                max_lines_per_frame = model->sensor.bands[band_index]
                    .sampling_char.lines_per_frame;
            }
        }

        /* Make sure the reference band index was setup for this sensor */
        if (ref_band_index[sensor_id] == -1)
        {
            IAS_LOG_ERROR("Determining reference band index for sensor id %d",
                    sensor_id);
            return ERROR;
        }

        /* Save the lines_per_frame that will be used for this sensor for
           building the jitter table, as this value is important to know
           when using the jitter table values later on */
        model->sensor.sensors[sensor_id].jitter_entries_per_frame
            = max_lines_per_frame;
    }

    return SUCCESS;
}
