/*************************************************************************
Name: ias_sensor_set_frame_times

Purpose: Sets the time from epoch in the model for a specific sensor.  A
    pointer to the already allocated array should be provided.  After this
    routine is called, the model owns the data and will free it when the
    model is freed.

Returns: SUCCESS if the operation succeeded, or ERROR if it failed

**************************************************************************/
#include "ias_logging.h"
#include "ias_satellite_attributes.h"
#include "ias_sensor_model.h"

int ias_sensor_set_frame_times
(
    IAS_SENSOR_MODEL *model,          /* I: model to set the frame times in */
    IAS_SENSOR_ID sensor_id,          /* I: sensor id to set frame times for */
    double *frame_seconds_from_epoch, /* I: array of frame times */
    int frame_count                   /* I: count of frame times in the array */
)
{
    int band_index;

    /* make sure the sensor id is in a legal range */
    if (sensor_id >= IAS_MAX_SENSORS)
    {
        IAS_LOG_ERROR("Unsupported sensor id provided: %d", sensor_id);
        return ERROR;
    }

    /* make sure the value hasn't already been set since that will cause a
       memory leak.  This should only be called once per sensor in the 
       satellite. */
    if (model->frame_seconds_from_epoch[sensor_id] != NULL)
    {
        IAS_LOG_ERROR("Setting the frame time from epoch a second time");
        return ERROR;
    }

    /* loop through all the bands to set the frame_seconds_from_epoch to the
       provided value */
    for (band_index = 0; band_index < model->band_count; band_index++)
    {
        IAS_SENSOR_BAND_MODEL *band = &model->bands[band_index];

        if (band->sensor->sensor_id == sensor_id)
        {
            band->frame_seconds_from_epoch = frame_seconds_from_epoch;
            band->frame_count = frame_count;
        }
    }
    model->frame_seconds_from_epoch[sensor_id] = frame_seconds_from_epoch;
    model->frame_counts[sensor_id] = frame_count;

    return SUCCESS;
}
