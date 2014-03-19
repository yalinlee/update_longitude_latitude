/******************************************************************************
NAME: ias_sensor_smooth_ssm_data

PURPOSE: Apply a moving average filter to smooth the Scene Select Mirror
    telemetry.

RETURNS:
    SUCCESS or ERROR

NOTES:
    - This assumes the SSM angle will not rollover at some point.
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "ias_logging.h"
#include "ias_sensor_model.h"

/* define the size of the window to use for smoothing */
#define SSM_SMOOTH_WINDOW 5

int ias_sensor_smooth_ssm_data
(
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm_model /* I/O: SSM data to smooth*/
)
{
    int window_size;            /* window size to use for filtering */
    int half_window;            /* half the window size */
    int record;                  /* current record index */
    double *filtered_angles;    /* filtered angle buffer */
    IAS_SENSOR_SSM_RECORD *records = ssm_model->records;

    /* Set up the smoother window */
    window_size = SSM_SMOOTH_WINDOW;
    half_window = window_size/2;

    /* verify there are enough angles to filter */
    if (ssm_model->ssm_record_count < window_size)
    {
        IAS_LOG_ERROR("At least %d SSM records required for filtering. Only %d"
                      " present", window_size, ssm_model->ssm_record_count);
        return ERROR;
    }

    /* allocate space for the filtered angles */
    filtered_angles = malloc(ssm_model->ssm_record_count 
                             * sizeof(*filtered_angles));
    if (filtered_angles == NULL)
    {
        IAS_LOG_ERROR("Failed to allocate a buffer to smooth SSM angles");
        return ERROR;
    }

    /* filter all the values */
    for (record = 0; record < ssm_model->ssm_record_count; record++)
    {
        int start_index;
        int index;
        double filtered_angle;

        start_index = record - half_window;

        /* force the filtering window to fall within the data */
        if ( start_index < 0 )
            start_index = 0;
        if ((start_index + window_size) > ssm_model->ssm_record_count)
            start_index = ssm_model->ssm_record_count - window_size;

        /* filter the data */
        filtered_angle = 0.0;
        for (index = start_index; index < start_index + window_size; index++)
            filtered_angle += records[index].mirror_angle;
        filtered_angle /= (double)window_size;

        /* save the filtered value */
        filtered_angles[record] = filtered_angle;
    }

    /* replace the original data with the filtered angles */
    for (record = 0; record < ssm_model->ssm_record_count; record++)
    {
        records[record].mirror_angle = filtered_angles[record];
    }

    /* Release the temporary buffer */
    free(filtered_angles);

    return SUCCESS;
}
