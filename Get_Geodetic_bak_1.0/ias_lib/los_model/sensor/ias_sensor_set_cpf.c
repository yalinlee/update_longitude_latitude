/*************************************************************************
Name: ias_sensor_set_cpf

Purpose: Accepts a CPF structure and a pointer to a sensor model structure
    to copy the CPF values into.

Notes:
    Sets the OLI nominal integration time in the model from the CPF. May be
    replaced/overriden later by the L0R value, if available and valid. TIRS
    has no nominal integration time in the CPF, so this value is not dealt
    with for TIRS in this function.

    Meant to be used in conjunction with ias_los_model_set_cpf(). Should
    only be called by that routine, not used independently.

RETURNS: SUCCESS or ERROR

**************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "../logging_channel.h" /* define debug logging channel */
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_sensor_model.h"
#include "local_defines.h"

static void log_sensor_cpf_values(IAS_SENSOR_MODEL *sensor);
static void set_ssm_alignment_matrix
(
    const struct IAS_CPF_TIRS_PARAMETERS *cpf_tirs, /* I: CPF TIRS parameters */
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm_model /* O: SSM model to set */
);


int ias_sensor_set_cpf
(
    IAS_CPF *cpf,                /* I: CPF structure pointer with values to
                                   set into the sensor structure */
    IAS_SENSOR_L0R_DATA *l0r_sensor_data,
                                /* O: Sensor L0R data needed by other funcs */
    IAS_SENSOR_MODEL *sensor    /* O: Sensor model structure to populate */
)
{
    int status;                       /* routine return statuses */
    int band_index;                   /* index to band array */
    int i, j;                         /* basic loop counters */
    int band_number;                  /* the band attributes band number */
    int sca_index;                    /* index to SCA array for current band */
    double sensor2arc[TRANSFORMATION_MATRIX_DIM][TRANSFORMATION_MATRIX_DIM];
                                      /* sensor to attitude control reference */
    const IAS_BAND_ATTRIBUTES *band_attribs;
                                      /* band attributes for current band num */
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm_model;
                                      /* cached pointer to the ssm model */
    /* CPF groups needed */
    const struct IAS_CPF_ATTITUDE_PARAMETERS *cpf_attitude;
    const struct IAS_CPF_OLI_PARAMETERS *cpf_oli;
    const struct IAS_CPF_TIRS_PARAMETERS *cpf_tirs;
    const struct IAS_CPF_DETECTOR_OFFSETS *cpf_offsets;
    const struct IAS_CPF_FOCAL_PLANE *cpf_focal_plane;
    const struct IAS_CPF_EARTH_CONSTANTS *cpf_earth_consts;

    /* Get the needed CPF groups */
    cpf_attitude = ias_cpf_get_attitude_params(cpf);
    if (!cpf_attitude)
    {
        IAS_LOG_ERROR("Retrieving CPF attitude parameters");
        return ERROR;
    }
    cpf_oli = ias_cpf_get_oli_parameters(cpf);
    if (!cpf_oli)
    {
        IAS_LOG_ERROR("Retrieving CPF OLI parameters");
        return ERROR;
    }
    cpf_tirs = ias_cpf_get_tirs_parameters(cpf);
    if (!cpf_tirs)
    {
        IAS_LOG_ERROR("Retrieving CPF TIRS parameters");
        return ERROR;
    }
    cpf_offsets = ias_cpf_get_det_offsets(cpf);
    if (!cpf_offsets)
    {
        IAS_LOG_ERROR("Retrieving CPF detector offsets");
        return ERROR;
    }
    cpf_focal_plane = ias_cpf_get_focal_plane(cpf);
    if (!cpf_focal_plane)
    {
        IAS_LOG_ERROR("Retrieving CPF focal plane parameters");
        return ERROR;
    }
    cpf_earth_consts = ias_cpf_get_earth_const(cpf);
    if (!cpf_earth_consts)
    {
        IAS_LOG_ERROR("Retrieving CPF Earth Constants parameters");
        return ERROR;
    }

    /* Save the nominal frame times for the L0R frame time validations.
       Convert milliseconds in CPF to seconds that the L0R frame time units
       will be in. */
    l0r_sensor_data->oli_data.nominal_frame_time
        = cpf_oli->nominal_frame_time * 1.0e-3;
    l0r_sensor_data->tirs_data.nominal_frame_time
        = cpf_tirs->nominal_frame_time * 1.0e-3;

    /* Set the OLI to ACS sensor alignment matrix */
    status 
       = ias_math_invert_matrix(&cpf_attitude->acs_to_oli_rotation_matrix[0][0],
            &sensor2arc[0][0], TRANSFORMATION_MATRIX_DIM);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Inverting OLI to ACS reference alignment matrix");
        return ERROR;
    }
    for (i = 0; i < TRANSFORMATION_MATRIX_DIM; i++)
    {
        for (j = 0; j < TRANSFORMATION_MATRIX_DIM; j++)
        {
            sensor->sensors[IAS_OLI].sensor2acs[i][j] = sensor2arc[i][j];
        }
        /* Set the center of mass to sensor offsets */
        sensor->sensors[IAS_OLI].center_mass2sensor_offset[i]
            = cpf_attitude->cm_to_oli_offsets[i];
    }

    /* Set the TIRS sensor2acs in the model */
    status = ias_math_invert_matrix(&cpf_attitude
                    ->acs_to_tirs_rotation_matrix[0][0],
                    &sensor2arc[0][0], TRANSFORMATION_MATRIX_DIM);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Inverting ACS to TIRS rotation matrix");
        return ERROR;
    }
    for (i = 0; i < TRANSFORMATION_MATRIX_DIM; i++)
    {
        for (j = 0; j < TRANSFORMATION_MATRIX_DIM; j++)
        {
            sensor->sensors[IAS_TIRS].sensor2acs[i][j] = sensor2arc[i][j];
        }
        /* Set the center of mass to sensor offsets */
        sensor->sensors[IAS_TIRS].center_mass2sensor_offset[i]
            = cpf_attitude->cm_to_tirs_offsets[i];
    }

    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        band_number = ias_sat_attr_convert_band_index_to_number(band_index);
        if (band_number == ERROR)
        {
            IAS_LOG_ERROR("Finding band number from band index");
            return ERROR;
        }
        band_attribs = ias_sat_attr_get_band_attributes(band_number);
        if (band_attribs == NULL)
        {
            IAS_LOG_ERROR("Getting band attributes for band number %d",
                    band_number);
            return ERROR;
        }

        /* Set the lines per frame, with PAN having twice the sample lines.
           Also set the settling time, the along and across track IFOVs, the
           nominal integration time, and the line delay flag. */
        if (band_attribs->band_type == IAS_PAN_BAND)
        {
            /* The OLI Pan band */
            sensor->bands[band_index].sampling_char.lines_per_frame
                = FRAME_LINES_PAN;
            /* The CPF stores the settling time in milliseconds, but the
               model stores it in seconds, so convert */
            sensor->bands[band_index].sampling_char.settling_time
                = cpf_oli->detector_settling_time_pan * 1.0e-3;

            sensor->bands[band_index].sampling_char.along_ifov
                = cpf_oli->along_ifov_pan;
            sensor->bands[band_index].sampling_char.across_ifov
                = cpf_oli->across_ifov_pan;
            
            /* Store the integration time from the nominal value in the CPF
               initially. Later, the set_l0r routine may override this value,
               if the value from the L0R is present and valid.
               The CPF stores the integration time in milliseconds, but the
               model stores it in seconds, so convert */
            sensor->bands[band_index].sampling_char.integration_time
                    = cpf_oli->nominal_integration_time_pan * 1.0e-3;

            /* For OLI, time codes are collected at the end of the frame */
            sensor->bands[band_index].sampling_char.time_codes_at_frame_start
                    = 0;

            /* The OLI instrument has a delay in the time stamp included with
               the imagery. Frame N of the imagery includes the time stamp for
               Frame N-1 (i.e. the time stamp is delayed by a single line).
               Set the flag for OLI bands so the find_time routine can adjust
               for this. */
            sensor->bands[band_index].sampling_char.frame_delay = 1;
        }
        else if (band_attribs->sensor_id == IAS_OLI)
        {
            /* All MS bands of OLI */
            sensor->bands[band_index].sampling_char.lines_per_frame
                = FRAME_LINES_MS;
            /* The CPF stores the settling time in milliseconds, but the
               model stores it in seconds, so convert */
            sensor->bands[band_index].sampling_char.settling_time
                = cpf_oli->detector_settling_time_ms * 1.0e-3;

            sensor->bands[band_index].sampling_char.along_ifov
                = cpf_oli->along_ifov_ms;
            sensor->bands[band_index].sampling_char.across_ifov
                = cpf_oli->across_ifov_ms;

            /* Store the integration time from the CPF initially. Later, the
               set_l0r routine may override this value, if the value from the
               L0R is present and valid.
               The CPF stores the integration time in milliseconds, but the
               model stores it in seconds, so convert */
            sensor->bands[band_index].sampling_char.integration_time
                    = cpf_oli->nominal_integration_time_ms * 1.0e-3;

            /* For OLI, time codes are collected at the end of the frame */
            sensor->bands[band_index].sampling_char.time_codes_at_frame_start
                    = 0;

            /* The OLI instrument has a delay in the time stamp included with
               the imagery. Frame N of the imagery includes the time stamp for
               Frame N-1 (i.e. the time stamp is delayed by a single line).
               Set the flag for OLI bands so the find_time routine can adjust
               for this. */
            sensor->bands[band_index].sampling_char.frame_delay = 1;
        }
        else if (band_attribs->sensor_id == IAS_TIRS)
        {
            sensor->bands[band_index].sampling_char.lines_per_frame = 1;

            /* TIRS doesn't have a settling time, so set it to zero */
            sensor->bands[band_index].sampling_char.settling_time = 0.0;

            /* set the along and across track IFOV */
            sensor->bands[band_index].sampling_char.along_ifov
                = cpf_tirs->along_ifov_thermal;
            sensor->bands[band_index].sampling_char.across_ifov
                = cpf_tirs->across_ifov_thermal;

            /* set the nominal integration time from the CPF.  May be
               overridden from the L0R contents later.  The CPF stores the
               integration time in milliseconds, so convert to seconds. */
            sensor->bands[band_index].sampling_char.integration_time
                    = cpf_tirs->nominal_integration_time * 1.0e-3;

            /* For TIRS, time codes are collected at the start of the frame */
            sensor->bands[band_index].sampling_char.time_codes_at_frame_start
                    = 1;

            /* The TIRS instrument does not have the same delay in the time
               stamp as OLI, so set the line delay flag to false. */
            sensor->bands[band_index].sampling_char.frame_delay = 0;
        }
        else
            IAS_LOG_ERROR("Unexpected sensor or band type");

        /* Set the maximum detector delay to a constant value for all bands */
        sensor->bands[band_index].sampling_char.maximum_detector_delay
            = MAX_DET_DELAY;

        for (sca_index = 0; sca_index < sensor->bands[band_index].sca_count;
                sca_index++)
        {
            IAS_SENSOR_SCA_MODEL *cur_sca;
            cur_sca = &sensor->bands[band_index].scas[sca_index];

            /* Set the along and across track detector offsets per detector */
            for (i = 0; i < cur_sca->detectors; i++)
            {
                cur_sca->detector_offsets_along_track[i]
                    = cpf_offsets->along_per_detector[band_index][sca_index][i];

                cur_sca->detector_offsets_across_track[i]
                    = cpf_offsets->across_per_detector[band_index]
                    [sca_index][i];
            }

            /* Set the Legendre coefficients */
            for (i = 0; i < IAS_LOS_LEGENDRE_TERMS; i++)
            {
                cur_sca->sca_coef_x[i] = cpf_focal_plane->along_los_legendre
                    [band_index][sca_index][i];
                cur_sca->sca_coef_y[i] = cpf_focal_plane->across_los_legendre
                    [band_index][sca_index][i];
            }

            /* Set the nominal fill offsets */
            cur_sca->nominal_fill = cpf_focal_plane->nominal_fill_offset
                [band_index][sca_index];
        }
    }

    /* make sure the SSM model is NULL for OLI */
    sensor->sensors[IAS_OLI].ssm_model = NULL;

    /* assume the TIRS sensor is present for now so we can add the CPF info
       to SSM model structure */

    /* make sure the ssm model is NULL since it should be here.  If it
       is set, it is likely a programming error. */
    if (sensor->sensors[IAS_TIRS].ssm_model != NULL)
    {
        IAS_LOG_ERROR("SSM model unexpectedly already allocated in model");
        return ERROR;
    }

    /* allocate the ssm model */
    ssm_model = calloc(1, sizeof(*ssm_model));
    if (!ssm_model)
    {
        IAS_LOG_ERROR("Failed to allocate the SSM model");
        return ERROR;
    }
    sensor->sensors[IAS_TIRS].ssm_model = ssm_model;

    /* set the TIRS SSM alignment matrix in the model */
    set_ssm_alignment_matrix(cpf_tirs, sensor->sensors[IAS_TIRS].ssm_model);

    /* Save the leap seconds data for L0R frame times processing */
    l0r_sensor_data->leap_seconds_data = &cpf_earth_consts->leap_seconds_data;

    /* Only does something if log level set to IAS_LOG_LEVEL_DEBUG */
    if (IAS_LOG_DEBUG_ENABLED())
        log_sensor_cpf_values(sensor);

    return SUCCESS;
}




/////////////////////////////////////////////////////////////////////////////
//added by LYL at 2014/3/6
int ias_sensor_set_cpf_for_MWD
(
    IAS_CPF *cpf,                /* I: CPF structure pointer with values to
                                   set into the sensor structure */
    IAS_SENSOR_MODEL *sensor    /* O: Sensor model structure to populate */
)
{
    int status;                       /* routine return statuses */
    int band_index;                   /* index to band array */
    int i, j;                         /* basic loop counters */
    int band_number;                  /* the band attributes band number */
    int sca_index;                    /* index to SCA array for current band */
    double sensor2arc[TRANSFORMATION_MATRIX_DIM][TRANSFORMATION_MATRIX_DIM];
                                      /* sensor to attitude control reference */
    const IAS_BAND_ATTRIBUTES *band_attribs;
                                      /* band attributes for current band num */
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm_model;
                                      /* cached pointer to the ssm model */
    /* CPF groups needed */
    const struct IAS_CPF_ATTITUDE_PARAMETERS *cpf_attitude;
    const struct IAS_CPF_OLI_PARAMETERS *cpf_oli;
    const struct IAS_CPF_TIRS_PARAMETERS *cpf_tirs;
    const struct IAS_CPF_DETECTOR_OFFSETS *cpf_offsets;
    const struct IAS_CPF_FOCAL_PLANE *cpf_focal_plane;
    const struct IAS_CPF_EARTH_CONSTANTS *cpf_earth_consts;

    /* Get the needed CPF groups */
    cpf_attitude = ias_cpf_get_attitude_params(cpf);
    if (!cpf_attitude)
    {
        IAS_LOG_ERROR("Retrieving CPF attitude parameters");
        return ERROR;
    }
    cpf_oli = ias_cpf_get_oli_parameters(cpf);
    if (!cpf_oli)
    {
        IAS_LOG_ERROR("Retrieving CPF OLI parameters");
        return ERROR;
    }
    cpf_tirs = ias_cpf_get_tirs_parameters(cpf);
    if (!cpf_tirs)
    {
        IAS_LOG_ERROR("Retrieving CPF TIRS parameters");
        return ERROR;
    }
    cpf_offsets = ias_cpf_get_det_offsets(cpf);
    if (!cpf_offsets)
    {
        IAS_LOG_ERROR("Retrieving CPF detector offsets");
        return ERROR;
    }
    cpf_focal_plane = ias_cpf_get_focal_plane(cpf);
    if (!cpf_focal_plane)
    {
        IAS_LOG_ERROR("Retrieving CPF focal plane parameters");
        return ERROR;
    }
    cpf_earth_consts = ias_cpf_get_earth_const(cpf);
    if (!cpf_earth_consts)
    {
        IAS_LOG_ERROR("Retrieving CPF Earth Constants parameters");
        return ERROR;
    }

    /* Set the OLI to ACS sensor alignment matrix */
    status
       = ias_math_invert_matrix(&cpf_attitude->acs_to_oli_rotation_matrix[0][0],
            &sensor2arc[0][0], TRANSFORMATION_MATRIX_DIM);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Inverting OLI to ACS reference alignment matrix");
        return ERROR;
    }
    for (i = 0; i < TRANSFORMATION_MATRIX_DIM; i++)
    {
        for (j = 0; j < TRANSFORMATION_MATRIX_DIM; j++)
        {
            sensor->sensors[IAS_OLI].sensor2acs[i][j] = sensor2arc[i][j];
        }
        /* Set the center of mass to sensor offsets */
        sensor->sensors[IAS_OLI].center_mass2sensor_offset[i]
            = cpf_attitude->cm_to_oli_offsets[i];
    }

    /* Set the TIRS sensor2acs in the model */
    status = ias_math_invert_matrix(&cpf_attitude
                    ->acs_to_tirs_rotation_matrix[0][0],
                    &sensor2arc[0][0], TRANSFORMATION_MATRIX_DIM);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Inverting ACS to TIRS rotation matrix");
        return ERROR;
    }
    for (i = 0; i < TRANSFORMATION_MATRIX_DIM; i++)
    {
        for (j = 0; j < TRANSFORMATION_MATRIX_DIM; j++)
        {
            sensor->sensors[IAS_TIRS].sensor2acs[i][j] = sensor2arc[i][j];
        }
        /* Set the center of mass to sensor offsets */
        sensor->sensors[IAS_TIRS].center_mass2sensor_offset[i]
            = cpf_attitude->cm_to_tirs_offsets[i];
    }

    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        band_number = ias_sat_attr_convert_band_index_to_number(band_index);
        if (band_number == ERROR)
        {
            IAS_LOG_ERROR("Finding band number from band index");
            return ERROR;
        }
        band_attribs = ias_sat_attr_get_band_attributes(band_number);
        if (band_attribs == NULL)
        {
            IAS_LOG_ERROR("Getting band attributes for band number %d",
                    band_number);
            return ERROR;
        }

        /* Set the lines per frame, with PAN having twice the sample lines.
           Also set the settling time, the along and across track IFOVs, the
           nominal integration time, and the line delay flag. */
        if (band_attribs->band_type == IAS_PAN_BAND)
        {
            /* The OLI Pan band */
            sensor->bands[band_index].sampling_char.lines_per_frame
                = FRAME_LINES_PAN;
            /* The CPF stores the settling time in milliseconds, but the
               model stores it in seconds, so convert */
            sensor->bands[band_index].sampling_char.settling_time
                = cpf_oli->detector_settling_time_pan * 1.0e-3;

            sensor->bands[band_index].sampling_char.along_ifov
                = cpf_oli->along_ifov_pan;
            sensor->bands[band_index].sampling_char.across_ifov
                = cpf_oli->across_ifov_pan;

            /* Store the integration time from the nominal value in the CPF
               initially. Later, the set_l0r routine may override this value,
               if the value from the L0R is present and valid.
               The CPF stores the integration time in milliseconds, but the
               model stores it in seconds, so convert */
            sensor->bands[band_index].sampling_char.integration_time
                    = cpf_oli->nominal_integration_time_pan * 1.0e-3;

            /* For OLI, time codes are collected at the end of the frame */
            sensor->bands[band_index].sampling_char.time_codes_at_frame_start
                    = 0;

            /* The OLI instrument has a delay in the time stamp included with
               the imagery. Frame N of the imagery includes the time stamp for
               Frame N-1 (i.e. the time stamp is delayed by a single line).
               Set the flag for OLI bands so the find_time routine can adjust
               for this. */
            sensor->bands[band_index].sampling_char.frame_delay = 1;
        }
        else if (band_attribs->sensor_id == IAS_OLI)
        {
            /* All MS bands of OLI */
            sensor->bands[band_index].sampling_char.lines_per_frame
                = FRAME_LINES_MS;
            /* The CPF stores the settling time in milliseconds, but the
               model stores it in seconds, so convert */
            sensor->bands[band_index].sampling_char.settling_time
                = cpf_oli->detector_settling_time_ms * 1.0e-3;

            sensor->bands[band_index].sampling_char.along_ifov
                = cpf_oli->along_ifov_ms;
            sensor->bands[band_index].sampling_char.across_ifov
                = cpf_oli->across_ifov_ms;

            /* Store the integration time from the CPF initially. Later, the
               set_l0r routine may override this value, if the value from the
               L0R is present and valid.
               The CPF stores the integration time in milliseconds, but the
               model stores it in seconds, so convert */
            sensor->bands[band_index].sampling_char.integration_time
                    = cpf_oli->nominal_integration_time_ms * 1.0e-3;

            /* For OLI, time codes are collected at the end of the frame */
            sensor->bands[band_index].sampling_char.time_codes_at_frame_start
                    = 0;

            /* The OLI instrument has a delay in the time stamp included with
               the imagery. Frame N of the imagery includes the time stamp for
               Frame N-1 (i.e. the time stamp is delayed by a single line).
               Set the flag for OLI bands so the find_time routine can adjust
               for this. */
            sensor->bands[band_index].sampling_char.frame_delay = 1;
        }
        else if (band_attribs->sensor_id == IAS_TIRS)
        {
            sensor->bands[band_index].sampling_char.lines_per_frame = 1;

            /* TIRS doesn't have a settling time, so set it to zero */
            sensor->bands[band_index].sampling_char.settling_time = 0.0;

            /* set the along and across track IFOV */
            sensor->bands[band_index].sampling_char.along_ifov
                = cpf_tirs->along_ifov_thermal;
            sensor->bands[band_index].sampling_char.across_ifov
                = cpf_tirs->across_ifov_thermal;

            /* set the nominal integration time from the CPF.  May be
               overridden from the L0R contents later.  The CPF stores the
               integration time in milliseconds, so convert to seconds. */
            sensor->bands[band_index].sampling_char.integration_time
                    = cpf_tirs->nominal_integration_time * 1.0e-3;

            /* For TIRS, time codes are collected at the start of the frame */
            sensor->bands[band_index].sampling_char.time_codes_at_frame_start
                    = 1;

            /* The TIRS instrument does not have the same delay in the time
               stamp as OLI, so set the line delay flag to false. */
            sensor->bands[band_index].sampling_char.frame_delay = 0;
        }
        else
            IAS_LOG_ERROR("Unexpected sensor or band type");

        /* Set the maximum detector delay to a constant value for all bands */
        sensor->bands[band_index].sampling_char.maximum_detector_delay
            = MAX_DET_DELAY;

        for (sca_index = 0; sca_index < sensor->bands[band_index].sca_count;
                sca_index++)
        {
            IAS_SENSOR_SCA_MODEL *cur_sca;
            cur_sca = &sensor->bands[band_index].scas[sca_index];

            /* Set the along and across track detector offsets per detector */
            for (i = 0; i < cur_sca->detectors; i++)
            {
                cur_sca->detector_offsets_along_track[i]
                    = cpf_offsets->along_per_detector[band_index][sca_index][i];

                cur_sca->detector_offsets_across_track[i]
                    = cpf_offsets->across_per_detector[band_index]
                    [sca_index][i];
            }

            /* Set the Legendre coefficients */
            for (i = 0; i < IAS_LOS_LEGENDRE_TERMS; i++)
            {
                cur_sca->sca_coef_x[i] = cpf_focal_plane->along_los_legendre
                    [band_index][sca_index][i];
                cur_sca->sca_coef_y[i] = cpf_focal_plane->across_los_legendre
                    [band_index][sca_index][i];
            }

            /* Set the nominal fill offsets */
            cur_sca->nominal_fill = cpf_focal_plane->nominal_fill_offset
                [band_index][sca_index];
        }
    }

    /* make sure the SSM model is NULL for OLI */
    sensor->sensors[IAS_OLI].ssm_model = NULL;

    /* assume the TIRS sensor is present for now so we can add the CPF info
       to SSM model structure */

    /* make sure the ssm model is NULL since it should be here.  If it
       is set, it is likely a programming error. */
    if (sensor->sensors[IAS_TIRS].ssm_model != NULL)
    {
        IAS_LOG_ERROR("SSM model unexpectedly already allocated in model");
        return ERROR;
    }

    /* allocate the ssm model */
    ssm_model = calloc(1, sizeof(*ssm_model));
    if (!ssm_model)
    {
        IAS_LOG_ERROR("Failed to allocate the SSM model");
        return ERROR;
    }
    sensor->sensors[IAS_TIRS].ssm_model = ssm_model;

    /* set the TIRS SSM alignment matrix in the model */
    set_ssm_alignment_matrix(cpf_tirs, sensor->sensors[IAS_TIRS].ssm_model);

    /* Save the leap seconds data for L0R frame times processing */
//    l0r_sensor_data->leap_seconds_data = &cpf_earth_consts->leap_seconds_data;

    /* Only does something if log level set to IAS_LOG_LEVEL_DEBUG */
    if (IAS_LOG_DEBUG_ENABLED())
        log_sensor_cpf_values(sensor);

    return SUCCESS;
}
/*************************************************************************
Name: set_ssm_alignment_matrix

Purpose: Helper routine to calculate the SSM alignment matrix from the 
    values in the CPF.

Returns:
    nothing

**************************************************************************/
static void set_ssm_alignment_matrix
(
    const struct IAS_CPF_TIRS_PARAMETERS *cpf_tirs, /* I: CPF TIRS parameters */
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm_model /* O: SSM model to set */
)
{
    double cosr;    /* cosine of telescope roll misalignment */
    double sinr;    /* sine of telescope roll misalignment */
    double cosp;    /* cosine of telescope pitch misalignment */
    double sinp;    /* sine of telescope pitch misalignment */
    double cosy;    /* cosine of telescope yaw misalignment */
    double siny;    /* sine of telescope yaw misalignment */

    cosr = cos(cpf_tirs->ssm_telescope_roll_offset);
    sinr = sin(cpf_tirs->ssm_telescope_roll_offset);
    cosp = cos(cpf_tirs->ssm_telescope_pitch_offset
         + 2.0 * cpf_tirs->ssm_mirror_angle_deviation);
    sinp = sin(cpf_tirs->ssm_telescope_pitch_offset
         + 2.0 * cpf_tirs->ssm_mirror_angle_deviation);
    cosy = cos(cpf_tirs->ssm_telescope_yaw_offset);
    siny = sin(cpf_tirs->ssm_telescope_yaw_offset);

    ssm_model->alignment_matrix[0][0] =  cosr * cosp;
    ssm_model->alignment_matrix[0][1] = -sinr * cosp;
    ssm_model->alignment_matrix[0][2] =  sinp;
    ssm_model->alignment_matrix[1][0] =  cosr * sinp * siny + sinr * cosy;
    ssm_model->alignment_matrix[1][1] =  cosr * cosy - sinr * sinp * siny;
    ssm_model->alignment_matrix[1][2] = -cosp * siny;
    ssm_model->alignment_matrix[2][0] =  sinr * siny - cosr * sinp * cosy;
    ssm_model->alignment_matrix[2][1] =  sinr * sinp * cosy + cosr * siny;
    ssm_model->alignment_matrix[2][2] =  cosp * cosy;
}

/*************************************************************************
Name: log_sensor_cpf_values

Purpose: Prints debugging log data for the CPF source values.

Returns:
    nothing

**************************************************************************/
static void log_sensor_cpf_values
(
    IAS_SENSOR_MODEL *sensor          /* I: Model structure */
)
{
    int band_index;                   /* index to band array */
    int sca_index;                    /* index to SCA array for current band */
    int i;                            /* basic loop counter */

    /*== DEBUG LOGGING ======================================================*/
    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        for (sca_index = 0; sca_index < sensor->bands[band_index].sca_count;
                sca_index++)
        {
            for (i = 0; i < sensor->bands[band_index].scas[sca_index].detectors;
                    i++)
            {
                if (band_index == 0 && sca_index == 0 && i >= 0 && i <= 4)
                {
                    IAS_LOG_DEBUG("Detector %d along track offset: %lf", i,
                            sensor->bands[0].scas[0].
                            detector_offsets_along_track[i]);
                    IAS_LOG_DEBUG("Detector %d across track offset: %lf", i,
                            sensor->bands[0].scas[0].
                            detector_offsets_across_track[i]);
                }
            }
            if (band_index == 0 && sca_index == 0)
            {
                IAS_LOG_DEBUG("Nominal fill: %d",
                        sensor->bands[0].scas[0].nominal_fill);
                IAS_LOG_DEBUG("Along track Legendre coefficients: %lf %lf %lf",
                        sensor->bands[0].scas[0].sca_coef_x[0],
                        sensor->bands[0].scas[0].sca_coef_x[1],
                        sensor->bands[0].scas[0].sca_coef_x[2]);
                IAS_LOG_DEBUG("Across track Legendre coefficients: %lf %lf %lf",
                        sensor->bands[0].scas[0].sca_coef_y[0],
                        sensor->bands[0].scas[0].sca_coef_y[1],
                        sensor->bands[0].scas[0].sca_coef_y[2]);
            }
        }
    }

    IAS_LOG_DEBUG("Sensor to ACS alignment matrix: %lf %lf %lf %lf %lf %lf "
            "%lf %lf %lf",
            sensor->sensors[0].sensor2acs[0][0],
            sensor->sensors[0].sensor2acs[0][1],
            sensor->sensors[0].sensor2acs[0][2],
            sensor->sensors[0].sensor2acs[1][0],
            sensor->sensors[0].sensor2acs[1][1],
            sensor->sensors[0].sensor2acs[1][2],
            sensor->sensors[0].sensor2acs[2][0],
            sensor->sensors[0].sensor2acs[2][1],
            sensor->sensors[0].sensor2acs[2][2]);
    IAS_LOG_DEBUG("Center of mass to sensor offsets: %lf %lf %lf",
            sensor->sensors[0].center_mass2sensor_offset[0],
            sensor->sensors[0].center_mass2sensor_offset[1],
            sensor->sensors[0].center_mass2sensor_offset[2]);

    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        IAS_LOG_DEBUG("Lines per frame for band %d: %d", band_index,
                sensor->bands[band_index].sampling_char.lines_per_frame);
    }
    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        IAS_LOG_DEBUG("Sampling settling time for band %d: %lf", band_index,
                sensor->bands[band_index].sampling_char.settling_time);
    }
    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        IAS_LOG_DEBUG("Along track IFOV for band %d: %lf", band_index,
                sensor->bands[band_index].sampling_char.along_ifov);
        IAS_LOG_DEBUG("Across track IFOV for band %d: %lf", band_index,
                sensor->bands[band_index].sampling_char.across_ifov);
    }
    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        IAS_LOG_DEBUG("Integration time for band %d: %lf", band_index,
                sensor->bands[band_index].sampling_char.integration_time);
    }
    /*== END DEBUG ==========================================================*/
}
