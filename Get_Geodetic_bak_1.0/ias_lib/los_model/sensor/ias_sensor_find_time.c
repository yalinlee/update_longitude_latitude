/******************************************************************************
Name: ias_sensor_find_time

Purpose: Find the time into the scene given the line, sample, SCA, and band.
    Time is calculated from sensor and scene based information that is stored
    within the LOS model.  Time can be calculated on a nominal line basis
    (NOMINAL) or a detector specific basis (ACTUAL or EXACT).  There is also a
    LOS calculation made based on a maximum offset, which is really an
    arbritrary number (MAXIMUM).  This maximum prevents calculations done with
    small detector offsets when generating odd/even adjustments that are placed
    in the resampling grid.

NOTES:
   The input sample number is 0-relative and relative to the SCA

RETURNS: SUCCESS or ERROR

******************************************************************************/
#include <math.h>
#include "ias_logging.h"
#include "ias_sensor_model.h"
#include "local_defines.h"

int ias_sensor_find_time
(
   double line,         /* I: Line from start of the image (0-rel)*/
   double sample,       /* I: Sample from start of the line (0-rel)*/
   int band_index,      /* I: Band in the image */
   int sca_index,       /* I: Current SCA in image */
   const IAS_SENSOR_MODEL *model,/* I: model structure */
   IAS_SENSOR_DETECTOR_TYPE type,/* I: Detector option */
   double *time         /* O: Time from start of image (seconds) */
)
{
    int detector;        /* Detector number */
    int iline;           /* Integer line location */
    double delay;        /* Detector delay for current SCA, Band, Detector */
    int time_index;      /* Index into image times */
    int frame_index;     /* Index to use in time calculation, potentially
                            adjusted for the OLI frame time delay in the time
                            codes */
    int l0r_detector_offset_pixels; /* Fill pixels in L0R */
    double round( double );  /* Declaration of round() function */
    const IAS_SENSOR_BAND_MODEL *band = &model->bands[band_index];
    const IAS_SENSOR_SCA_MODEL *sca = &band->scas[sca_index];
    int integration_sign; /* holds +1 or -1 for the sign of the integration
                             time adjustment, based on whether time codes are
                             collected at the start or end of the frame */

    /* Compute the detector number */
    detector = (int)floor(sample + 0.5);
    if ((detector < 0) || (detector >= sca->detectors))
    {
        IAS_LOG_ERROR("Sample out of range: %d not in [0...%d]", detector,
            sca->detectors - 1);
        return ERROR;
    }

    /* Compute line number */
    iline = (int)floor(line + 0.5);

    /* Find index into image time.  Use MAXIMUM value if that option is
       selected. */
    if (type == IAS_MAXIMUM_DETECTOR)
    {
        delay = band->sampling_char.maximum_detector_delay;
        l0r_detector_offset_pixels = (int)round(delay) + sca->nominal_fill;
    }
    else
    {
        delay = sca->detector_offsets_along_track[detector];
        l0r_detector_offset_pixels = sca->l0r_detector_offsets[detector];
    }

    time_index = (iline - l0r_detector_offset_pixels)
        / band->sampling_char.lines_per_frame;
    /* Adjust the computed time index by one to account for one frame delay in
       the OLI time codes, if needed */
    if (band->sampling_char.frame_delay)
        time_index += 1;

    /* If the computed time index is outside the image range,
       use the first/last time code and extrapolate using the
       nominal line time. */
    if (time_index < 0)
        time_index = 0;
    if (time_index > band->frame_count - 1)
        time_index = band->frame_count - 1;

    if (band->sampling_char.time_codes_at_frame_start)
        integration_sign = 1;
    else
        integration_sign = -1;

    /* Calculate the time. Since the frame times are obtained at the end of
       the frame, the settling time and half of the integration time need to
       be subtracted to get the time at the center of the pixel. Then the
       offset in the time needs to be added in that is caused by the SCA
       misalignment and the even/odd pixel misalignment. */

    frame_index = time_index;
    /* Adjust the frame index for this computation back one if the OLI frame
       time code delay is present */
    if (band->sampling_char.frame_delay)
        frame_index -= 1;

    *time = band->frame_seconds_from_epoch[time_index] 
        - band->sampling_char.settling_time
        + (integration_sign * band->sampling_char.integration_time / 2.0)
        + (line - l0r_detector_offset_pixels
                - band->sampling_char.lines_per_frame
                * frame_index) * band->sampling_char.sampling_time;

    /* Remove delay effects if NOMINAL time is selected. */
    if (type == IAS_NOMINAL_DETECTOR)
    {
        *time += (sca->l0r_detector_offsets[detector] - sca->nominal_fill)
                 * band->sampling_char.sampling_time;
    }

    return SUCCESS;
}
