/******************************************************************************
Name:           ias_sensor_get_jitter

PURPOSE: Get the high frequency jitter disturbances in the scene given the
    line, sample, sca, and band. Jitter is stored in the model jitter table
    indexed by image line. This high frequency signal is only used when the
    detector type is EXACT.

Returns:
   SUCCESS or ERROR

NOTES:
   The input sample number is 0-relative and relative to the SCA

******************************************************************************/
#include <math.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_sensor_model.h"

int ias_sensor_get_jitter
(
    double line,                    /* I: Line from start of image (0-rel) */
    double sample,                  /* I: Sample from start of line (0-rel) */
    int band_index,                 /* I: Band index */
    int sca_index,                  /* I: Current SCA index */
    const IAS_SENSOR_MODEL *sensor, /* I: IAS sensor model structure */
    double *roll,                   /* O: Roll jitter (radians) */
    double *pitch,                  /* O: Pitch jitter (radians) */
    double *yaw                     /* O: Yaw jitter (radians) */
)
{
    int detector;        /* Detector number */
    int iline;           /* Integer line location */
    int index;           /* Index into image times */
    int l0r_fill_pixels; /* Fill pixels in L0R */
    /* local pointers into the sensor model structure for easier use */
    IAS_SENSOR_BAND_MODEL *band_model = &sensor->bands[band_index];
    IAS_SENSOR_LOCATION_MODEL *sensor_loc = band_model->sensor;
    IAS_SENSOR_SCA_MODEL *sca_model = &band_model->scas[sca_index];
    int band_lines_per_frame = band_model->sampling_char.lines_per_frame;

    /* Compute integer detector number */
    detector = (int)floor(sample + 0.5);
    if (detector < 0 || detector > sca_model->detectors)
    {
        IAS_LOG_ERROR("Sample %d out of range", detector);
        return ERROR;
    }

    /* Compute integer line number */
    iline = (int)floor(line + 0.5);

    /* Find index into the jitter table */
    l0r_fill_pixels = sca_model->l0r_detector_offsets[detector];

    /* The jitter table count is based on the resolution of the band for each
       sensor with the greatest lines_per_frame attribute. That is saved in
       the model and must be accounted for when accessing the jitter table.
       To be most generic/flexible, don't assume the lines_per_frame for
       the current band is 1. */
    index = (iline - l0r_fill_pixels) * sensor_loc->jitter_entries_per_frame
        / band_lines_per_frame;

    /* If the computed index is outside the jitter table range, return 0 */
    if (index < 0 || index > (sensor_loc->jitter_table_count - 1)) 
    {
        *roll = 0.0;
        *pitch = 0.0;
        *yaw = 0.0;
    }
    else
    {
        *roll = sensor_loc->jitter_table[index].x;
        *pitch = sensor_loc->jitter_table[index].y;
        *yaw = sensor_loc->jitter_table[index].z;
    }

    return SUCCESS;
}
