/******************************************************************************

 The following routines are provided to support
 ias_los_model_build_jitter_table()

******************************************************************************/
#include <stdlib.h>
#include <math.h>
#include <string.h>       /* memcpy */
#include "remez.h"
#include "ias_logging.h"
#include "ias_const.h"
#include "ias_spacecraft_model.h"

/* scaling factor that determines the filter frequency resolution, as an
   oversampling factor relative to the filter cutoff frequency */
#define REMEZ_FILTER_FREQ_SAMP_FACTOR   3
/* configuration option to remez - number of frequency bands to use;
   one pass band (low frequency), one stop band (high frequency) */
#define REMEZ_NUM_FREQUENCY_BANDS   2

/******************************************************************************
Name: ias_sc_model_copy_attitude

Purpose: Makes a copy of an attitude model into an existing attitude model
   given. Free's the contents of the output structure and then copies the
   data from the input attitude model into the output attitude model provided.

Returns: SUCCESS or ERROR
******************************************************************************/
int ias_sc_model_copy_attitude
(
    const IAS_SC_ATTITUDE_MODEL *in_att,    /* I: Input attitude to be copied */
    IAS_SC_ATTITUDE_MODEL *out_att          /* O: Copy of input attitude data */
)
{
    /* Initialize the output structure */
    ias_sc_model_free_attitude(out_att);
    out_att->utc_epoch_time[0] = in_att->utc_epoch_time[0];
    out_att->utc_epoch_time[1] = in_att->utc_epoch_time[1];
    out_att->utc_epoch_time[2] = in_att->utc_epoch_time[2];
    out_att->nominal_sample_time = in_att->nominal_sample_time;
    out_att->sample_count = in_att->sample_count;

    /* Allocate the attitude records */
    out_att->sample_records
        = (IAS_SC_ATTITUDE_RECORD *)malloc(out_att->sample_count
                * sizeof(IAS_SC_ATTITUDE_RECORD));
    if(out_att->sample_records == NULL)
    {
        IAS_LOG_ERROR("Allocating attitude sample records");
        ias_sc_model_free_attitude(out_att);
        return ERROR;
    }

    /* Copy the data */
    memcpy(out_att->sample_records, in_att->sample_records,
            out_att->sample_count * sizeof(IAS_SC_ATTITUDE_RECORD));

    return SUCCESS;
}

/******************************************************************************
Name: ias_sc_model_find_jitter_attitude_at_time

Purpose: Computes the high frequency attitude for the given line time.
    Linear interpolation to the line time is done with the provided attitude
    sequence.

Returns: SUCCESS or ERROR
******************************************************************************/
int ias_sc_model_find_jitter_attitude_at_time
(
    const IAS_SC_ATTITUDE_MODEL *att,   /* I: Attitude structure */
    double seconds_from_epoch,          /* I: Actual time
                                              (seconds from attitude epoch) */
    IAS_VECTOR *jit_attitude            /* O: Interpolated roll, pitch, yaw
                                              (radians) at time */
)
{
    int index;      /* 1st required index to the "left of" seconds_from_epoch */
    double offset;  /* Interpolation point offset */
    double w1;      /* Interpolation weight 1 */
    double w2;      /* Interpolation weight 2 */
    double w3;      /* Interpolation weight 3 */
    double w4;      /* Interpolation weight 4 */

    /* Initialize R/P/Y */
    jit_attitude->x = 0.0;
    jit_attitude->y = 0.0;
    jit_attitude->z = 0.0;

    /* Compute starting index for interpolation */
    index = (int)floor((seconds_from_epoch
                - att->sample_records[0].seconds_from_epoch)
            / att->nominal_sample_time) - 1;
    if ((index < 0) || (index > (att->sample_count - 4)))
    {
        IAS_LOG_ERROR("Attitude correction not found for time offset %f",
                seconds_from_epoch);
        return ERROR;
    }

    /* Compute the fractional sample offset into the line time */
    offset = fmod(seconds_from_epoch, att->nominal_sample_time)
        / att->nominal_sample_time;

    /* Compute the Lagrange weights */
    w1 = -offset * (offset - 1.0) * (offset - 2.0) / 6.0;
    w2 = (offset + 1.0) * (offset - 1.0) * (offset - 2.0) / 2.0;
    w3 = -offset * (offset + 1.0) * (offset - 2.0) / 2.0;
    w4 = (offset + 1.0) * offset * (offset - 1.0) / 6.0;

    /* Interpolate */
    jit_attitude->x = att->sample_records[index].precision_attitude.x * w1
        + att->sample_records[index + 1].precision_attitude.x * w2
        + att->sample_records[index + 2].precision_attitude.x * w3
        + att->sample_records[index + 3].precision_attitude.x * w4;
    jit_attitude->y = att->sample_records[index].precision_attitude.y * w1
        + att->sample_records[index + 1].precision_attitude.y * w2
        + att->sample_records[index + 2].precision_attitude.y * w3
        + att->sample_records[index + 3].precision_attitude.y * w4;
    jit_attitude->z = att->sample_records[index].precision_attitude.z * w1
        + att->sample_records[index + 1].precision_attitude.z * w2
        + att->sample_records[index + 2].precision_attitude.z * w3
        + att->sample_records[index + 3].precision_attitude.z * w4;

    return SUCCESS;
}

/******************************************************************************
Name: ias_sc_model_free_attitude

Purpose: Frees memory for an attitude model structure.

Returns: SUCCESS or ERROR
******************************************************************************/
void ias_sc_model_free_attitude
(
    IAS_SC_ATTITUDE_MODEL *att      /* I: Attitude structure to free */
)
{
    if (att->sample_records != NULL)
        free(att->sample_records);

    ias_sc_model_initialize_attitude(att);
}

/******************************************************************************
Name: ias_sc_model_initialize_attitude

Purpose: Initializes components of a given attitude structure to all 0's, NULL
         pointer for records.

Returns: nothing
******************************************************************************/
void ias_sc_model_initialize_attitude
(
    IAS_SC_ATTITUDE_MODEL *att      /* I: Attitude structure to initialize */
)
{
    att->utc_epoch_time[0] = 0.0;
    att->utc_epoch_time[1] = 0.0;
    att->utc_epoch_time[2] = 0.0;
    att->nominal_sample_time = 0.0;
    att->sample_count = 0;
    att->sample_records = NULL;
}

/******************************************************************************
Name: ias_sc_model_remez_filter_attitude

Purpose: Separates out the high frequency jitter using the REMEZ Exchange
    filter algorithm.

Returns: SUCCESS or ERROR
******************************************************************************/
int ias_sc_model_remez_filter_attitude
(
    const IAS_SC_ATTITUDE_MODEL *orig_att,  /* I: Original satellite attitude */
    double cutoff_frequency,                /* I: Low pass filter cutoff
                                                  frequency */
    IAS_SC_ATTITUDE_MODEL *low_att          /* O: Attitude after high frequency
                                                  aspects filtered out */
)
{
    int number_samples;             /* Samples per cycle at cutoff frequency */
    int filter_size;                /* Filter kernel size */
    int half_filter_size;           /* Half of filter kernel size */
    int i, j, k;                    /* Looping indices */
    int att_index;                  /* Attitude looping index */
    int status;                     /* Function return status */
    double norm_cutoff_frequency;   /* Normalized cutoff frequency */
    double *filter_kernel;          /* Filter kernel */
    /* Filter band frequency bounds */
    double band_frequency_bounds[REMEZ_NUM_FREQUENCY_BANDS * 2];
    /* Desired response in frequency bands */
    double target_band_gains[REMEZ_NUM_FREQUENCY_BANDS];
    /* Weights to apply to frequency bands */
    double band_weights[REMEZ_NUM_FREQUENCY_BANDS];
    /* Filter taps summation to check normalization */
    double filter_taps_sum;

    /* Initialize the output structure */
    ias_sc_model_free_attitude(low_att);
    low_att->utc_epoch_time[0] = orig_att->utc_epoch_time[0];
    low_att->utc_epoch_time[1] = orig_att->utc_epoch_time[1];
    low_att->utc_epoch_time[2] = orig_att->utc_epoch_time[2];
    low_att->nominal_sample_time = orig_att->nominal_sample_time;
    low_att->sample_count = orig_att->sample_count;

    /* Allocate the low frequency attitude data records */
    low_att->sample_records
        = (IAS_SC_ATTITUDE_RECORD *)malloc(low_att->sample_count
                * sizeof(IAS_SC_ATTITUDE_RECORD));
    if (low_att->sample_records == NULL)
    {
        IAS_LOG_ERROR("Allocating low frequency attitude records");
        ias_sc_model_free_attitude(low_att);
        return ERROR;
    }

    /* Design a low pass filter for REMEZ with the desired cutoff frequency */
    /* Compute the normalized cutoff frequency */
    norm_cutoff_frequency = cutoff_frequency * orig_att->nominal_sample_time;

    /* Compute the number of samples per cycle at the cutoff frequency */
    number_samples = (int)(1.0 / norm_cutoff_frequency);

    /* Compute the desired filter size */
    filter_size = (number_samples * REMEZ_FILTER_FREQ_SAMP_FACTOR) + 1;

    /* If result is even filter size, add one to make it odd */
    if ((filter_size % 2) == 0)
        filter_size++;

    /* Allocate the filter kernel */
    filter_kernel = (double *)malloc(filter_size * sizeof(double));
    if (filter_kernel == NULL)
    {
        IAS_LOG_ERROR("Allocating REMEZ filter kernel");
        ias_sc_model_free_attitude(low_att);
        return ERROR;
    }

    /* Set up the filter inputs */
    /* Band frequency bounds
       [0],[1]: 0 to normalized cutoff for the pass band
       [2],[3]: 1.5 * normalized cutoff to 0.5 (normalized Nyquist frequency)
       for the stop band */
    band_frequency_bounds[0] = 0.0;
    band_frequency_bounds[1] = norm_cutoff_frequency;
    band_frequency_bounds[2] = norm_cutoff_frequency*1.5;
    band_frequency_bounds[3] = 0.5;

    /* Desired band gains. [0] pass band, [1] stop band */
    target_band_gains[0] = 1.0;
    target_band_gains[1] = 0.0; 

    /* Band weights - how tightly to constrain the actual filter response to
       the design filter response in each band. [0] pass band, [1] stop band */
    band_weights[0] = 1.0;
    band_weights[1] = 10.0;

    /* Synthesize the filter kernel using the Remez Exchange algorithm */
    status = remez(filter_kernel, filter_size, REMEZ_NUM_FREQUENCY_BANDS,
            band_frequency_bounds, target_band_gains,
            band_weights, BANDPASS);
    if (status != REMEZ_SUCCESS)
    {
        IAS_LOG_ERROR("Creating REMEZ filter kernel");
        free(filter_kernel);
        ias_sc_model_free_attitude(low_att);
        return ERROR;
    }

    /* Make sure the filter is normalized (weights sum to 1) */
    filter_taps_sum = 0.0;
    for (i = 0; i < filter_size; i++)
        filter_taps_sum += filter_kernel[i];
    for (i = 0; i < filter_size; i++)
        filter_kernel[i] /= filter_taps_sum;

    /* Apply the filter */
    /* Loop through the attitude points */
    half_filter_size = filter_size / 2;
    for (att_index = 0; att_index < orig_att->sample_count; att_index++)
    {
        IAS_SC_ATTITUDE_RECORD *low_att_rec
            = &low_att->sample_records[att_index];
        IAS_SC_ATTITUDE_RECORD *orig_att_rec
            = &orig_att->sample_records[att_index];

        IAS_VECTOR *low_sat_att = &low_att_rec->satellite_attitude;
        IAS_VECTOR *low_prec_att = &low_att_rec->precision_attitude;

        /* Initialize the R/P/Y sequences */
        low_att_rec->seconds_from_epoch = orig_att_rec->seconds_from_epoch;
        low_sat_att->x = 0.0;
        low_sat_att->y = 0.0;
        low_sat_att->z = 0.0;
        low_prec_att->x = 0.0;
        low_prec_att->y = 0.0;
        low_prec_att->z = 0.0;

        k = 0;
        for(i = -half_filter_size; i <= half_filter_size; i++)
        {
            IAS_VECTOR *orig_sat_att;
            IAS_VECTOR *orig_prec_att;

            if((att_index + i) < 0)
                j = -att_index - i;
            else if(att_index + i >= orig_att->sample_count)
                j = 2 * orig_att->sample_count - att_index - i - 1;
            else
                j = att_index + i;

            orig_sat_att = &orig_att->sample_records[j].satellite_attitude;
            orig_prec_att = &orig_att->sample_records[j].precision_attitude;

            /* Apply the filter impulse response to all six sequences */
            low_sat_att->x += orig_sat_att->x * filter_kernel[k];
            low_sat_att->y += orig_sat_att->y * filter_kernel[k];
            low_sat_att->z += orig_sat_att->z * filter_kernel[k];
            low_prec_att->x += orig_prec_att->x * filter_kernel[k];
            low_prec_att->y += orig_prec_att->y * filter_kernel[k];
            low_prec_att->z += orig_prec_att->z * filter_kernel[k];
            k++;
        }
    }

    /* Release the filter kernel */
    free(filter_kernel);

    return SUCCESS;
}

/******************************************************************************
Name: ias_sc_model_subtract_attitude

Purpose: Subtracts one attitude sequence of data from the other. Puts results
    into an existing attitude model given. Anything already in the results
    attitude model is free'd and replaced.

Returns: SUCCESS or ERROR
******************************************************************************/
int ias_sc_model_subtract_attitude
(
    const IAS_SC_ATTITUDE_MODEL *in_att1,   /* I: First sequence */
    const IAS_SC_ATTITUDE_MODEL *in_att2,   /* I: Sequence to subtract */
    IAS_SC_ATTITUDE_MODEL *out_att          /* O: Result */
)
{
    int i;         /* Sample looping index */

    /* Make sure a valid subtraction will be possible with given input
       attitude sequences */
    if (in_att2->sample_count < in_att1->sample_count)
    {
        IAS_LOG_ERROR("Second attitude sequence to subtract contains fewer "
                "samples than first sequence");
        return ERROR;
    }

    /* Initialize the output structure */
    ias_sc_model_free_attitude(out_att);
    out_att->utc_epoch_time[0] = in_att1->utc_epoch_time[0];
    out_att->utc_epoch_time[1] = in_att1->utc_epoch_time[1];
    out_att->utc_epoch_time[2] = in_att1->utc_epoch_time[2];
    out_att->nominal_sample_time = in_att1->nominal_sample_time;
    out_att->sample_count = in_att1->sample_count;

    /* Allocate the output attitude record set */
    out_att->sample_records
        = (IAS_SC_ATTITUDE_RECORD *)malloc(out_att->sample_count
                * sizeof(IAS_SC_ATTITUDE_RECORD));
    if (out_att->sample_records == NULL)
    {
        IAS_LOG_ERROR("Allocating attitude sample records");
        ias_sc_model_free_attitude(out_att);
        return ERROR;
    }

    /* Extract the high frequency portion of the data through subtraction */
    for (i = 0; i < out_att->sample_count; i++)
    {
        IAS_SC_ATTITUDE_RECORD *in1_att_rec = &in_att1->sample_records[i];
        IAS_SC_ATTITUDE_RECORD *in2_att_rec = &in_att2->sample_records[i];
        IAS_SC_ATTITUDE_RECORD *out_att_rec = &out_att->sample_records[i];

        IAS_VECTOR *in1_sat_att = &in1_att_rec->satellite_attitude;
        IAS_VECTOR *in2_sat_att = &in2_att_rec->satellite_attitude;
        IAS_VECTOR *out_sat_att = &out_att_rec->satellite_attitude;
        IAS_VECTOR *in1_prec_att = &in1_att_rec->precision_attitude;
        IAS_VECTOR *in2_prec_att = &in2_att_rec->precision_attitude;
        IAS_VECTOR *out_prec_att = &out_att_rec->precision_attitude;

        out_att_rec->seconds_from_epoch = in1_att_rec->seconds_from_epoch;
        out_sat_att->x = in1_sat_att->x - in2_sat_att->x;
        out_sat_att->y = in1_sat_att->y - in2_sat_att->y;
        out_sat_att->z = in1_sat_att->z - in2_sat_att->z;
        out_prec_att->x = in1_prec_att->x - in2_prec_att->x;
        out_prec_att->y = in1_prec_att->y - in2_prec_att->y;
        out_prec_att->z = in1_prec_att->z - in2_prec_att->z;
    }

    return SUCCESS;
}

/******************************************************************************
Name: ias_sc_model_transfer_jitter_attitude_bias

Purpose: Transfers any residual bias in the high pass sequence to the low
    pass sequence.

Returns: SUCCESS or ERROR
******************************************************************************/
int ias_sc_model_transfer_jitter_attitude_bias
(
    double start_time,                  /* I: Time to start bias computation */
    double stop_time,                   /* I: Time to stop bias computation */
    IAS_SC_ATTITUDE_MODEL *from_att,    /* I/O: Sequence to remove bias from */
    IAS_SC_ATTITUDE_MODEL *to_att       /* I/O: Sequence to add bias to */
)
{
    IAS_VECTOR satellite_bias;  /* Measured satellite bias */
    IAS_VECTOR precision_bias;  /* Measured precision bias */
    int start_index;            /* Index to start bias computation */
    int stop_index;             /* Index to stop bias computation */
    int count;                  /* Number of samples in window */
    int i;                      /* Sample index */

    /* Make sure we have data */
    if ((from_att->sample_count < 1) || (to_att->sample_count < 1))
    {
        IAS_LOG_ERROR("Invalid attitude sequence; no samples");
        return ERROR;
    }

    /* Compute the time window */
    start_index = (int)floor((start_time
                - from_att->sample_records[0].seconds_from_epoch)
            / from_att->nominal_sample_time);
    if (start_index < 0)
        start_index = 0;
    stop_index = (int)floor((stop_time
                - from_att->sample_records[0].seconds_from_epoch)
            / from_att->nominal_sample_time ) + 1;
    if (stop_index > (from_att->sample_count - 1))
        stop_index = from_att->sample_count - 1;
    if (start_index > stop_index)
    {
        IAS_LOG_ERROR("Determining bias computation start and stop indices");
        return ERROR;
    }

    /* Initialize the bias vectors */
    satellite_bias.x = 0.0;
    satellite_bias.y = 0.0;
    satellite_bias.z = 0.0;
    precision_bias.x = 0.0;
    precision_bias.y = 0.0;
    precision_bias.z = 0.0;

    /* Compute the bias for from_att */
    for (i = start_index; i <= stop_index; i++)
    {
        satellite_bias.x += from_att->sample_records[i].satellite_attitude.x;
        satellite_bias.y += from_att->sample_records[i].satellite_attitude.y;
        satellite_bias.z += from_att->sample_records[i].satellite_attitude.z;
        precision_bias.x += from_att->sample_records[i].precision_attitude.x;
        precision_bias.y += from_att->sample_records[i].precision_attitude.y;
        precision_bias.z += from_att->sample_records[i].precision_attitude.z;
    }
    count = stop_index - start_index + 1;
    satellite_bias.x /= (double)count;
    satellite_bias.y /= (double)count;
    satellite_bias.z /= (double)count;
    precision_bias.x /= (double)count;
    precision_bias.y /= (double)count;
    precision_bias.z /= (double)count;

    /* Remove the bias from from_att */
    for (i = 0; i < from_att->sample_count; i++)
    {
        from_att->sample_records[i].satellite_attitude.x -= satellite_bias.x;
        from_att->sample_records[i].satellite_attitude.y -= satellite_bias.y;
        from_att->sample_records[i].satellite_attitude.z -= satellite_bias.z;
        from_att->sample_records[i].precision_attitude.x -= precision_bias.x;
        from_att->sample_records[i].precision_attitude.y -= precision_bias.y;
        from_att->sample_records[i].precision_attitude.z -= precision_bias.z;
    }

    /* Add the bias to to_att */
    for (i = 0; i < to_att->sample_count; i++)
    {
        to_att->sample_records[i].satellite_attitude.x += satellite_bias.x;
        to_att->sample_records[i].satellite_attitude.y += satellite_bias.y;
        to_att->sample_records[i].satellite_attitude.z += satellite_bias.z;
        to_att->sample_records[i].precision_attitude.x += precision_bias.x;
        to_att->sample_records[i].precision_attitude.y += precision_bias.y;
        to_att->sample_records[i].precision_attitude.z += precision_bias.z;
    }

    return SUCCESS;
}
