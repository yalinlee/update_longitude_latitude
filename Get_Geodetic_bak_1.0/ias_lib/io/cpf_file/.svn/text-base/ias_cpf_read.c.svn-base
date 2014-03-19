/*************************************************************************

NAME: ias_cpf_read

PURPOSE: Open the cpf file on the file system, allocate cpf memory,
         populate odl tree, and define the helper functions to populate
         the desired structure with cpf data.
        
RETURN VALUE:
        Type = CPF *

Value               Description
--------------------------------------------------------------------------
Success             Pointer to CPF struct
NULL                Failure

NOTE:  The structure elements of the cpf are statically declared in the 
        ias_cpf.h header file.  All are declared with the defines
        IAS_MAX_NBANDS and IAS_MAX_NSCAS.  All pointers are set to NULL
        in the OLI parse routines.  This will include all structure 
        elements shared between OLI and TIRS.  The shared elements are
        combined in this script via pointer minipulation after both sets 
        are retrieved from the CPF file.  TIRS band/sca combinations are 
        added on as bands 10 & 11.

******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "ias_odl.h"
#include "ias_cpf.h"
#include "local_defines.h"
#include "ias_logging.h"

struct IAS_CPF *ias_cpf_read
(
    const char *filename      /* I: name of the CPF file to read */
)
{
    IAS_CPF *cpf;           /* pointer to cpf structure */

    /* allocate memory for the CPF structure */
    cpf = malloc(sizeof(*cpf));
    if (!cpf)
    {
        IAS_LOG_ERROR("Allocating memory for the CPF structure");
        return NULL;
    }
    memset(cpf, 0, sizeof(*cpf));

    /* read the CPF file into memory */
    if (ias_cpf_cache_file(filename, cpf) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading CPF file from %s", filename);
        free(cpf);
        return NULL;
    }

    return cpf;
}

/*****************************************************************************
NAME:  ias_cpf_get_histogram_char

PURPOSE: Helper function to call for the load of the histogram characterization
         group from the odl tree to the histogram characterization structure

RETURNS: Pointer to the histogram characterization structure

******************************************************************************/
const struct IAS_CPF_HISTOGRAM_CHAR *ias_cpf_get_histogram_char
(
    IAS_CPF *cpf
)
{
    if (!cpf->histogram_char_loaded)
    {
        int status;

        status = ias_cpf_parse_histogram_characterization(cpf, 
                                    &cpf->histogram_char);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF histogram characterization group");
            return NULL;
        }
        cpf->histogram_char_loaded = 1;
    }
    return &cpf->histogram_char;
}

/*****************************************************************************
NAME:  ias_cpf_get_los_model_correction

PURPOSE: Helper function to call for the load of the los model correction
         group from the odl tree to the los model correction structure

RETURNS: Pointer to the los model correction structure

******************************************************************************/
const struct IAS_CPF_LOS_MODEL_CORRECTION *ias_cpf_get_los_model_correction
(
    IAS_CPF *cpf
)
{
    if (!cpf->los_model_correction_loaded)
    {
        int status;

        status = ias_cpf_parse_los_model_correction(cpf, 
                                    &cpf->los_model_correction);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF OLI los model correction data");
            return NULL;
        }
        cpf->los_model_correction_loaded = 1;
    }
    return &cpf->los_model_correction;
}

/*****************************************************************************
NAME:  ias_cpf_get_radiance_rescale

PURPOSE: Helper function to call for the load of the radiance rescale
         group from the odl tree to the radiance rescale structure

RETURNS: Pointer to the radiance rescale structure

******************************************************************************/
const struct IAS_CPF_RADIANCE_RESCALE *ias_cpf_get_radiance_rescale
(
    IAS_CPF *cpf
)
{
    if (!cpf->radiance_rescale_loaded)
    {
        int status;

        status = ias_cpf_parse_oli_radiance_rescale(cpf, 
                                    &cpf->radiance_rescale);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF OLI radiance rescale data");
            return NULL;
        }
    
        status = ias_cpf_parse_tirs_radiance_rescale(cpf, 
                                &cpf->radiance_rescale);
        if (status != SUCCESS)
        {
           IAS_LOG_ERROR("Reading CPF TIRS radiance rescale data");
           return NULL;
        }
        cpf->radiance_rescale_loaded = 1;
    }
    return &cpf->radiance_rescale;
}

/*****************************************************************************
NAME:  ias_cpf_get_thermal_constants

PURPOSE: Helper function to call for the load of the TIRS thermal constants
         group from the odl tree to the TIRS thermal constants structure

RETURNS: Pointer to the thermal constants structure

******************************************************************************/
const struct IAS_CPF_TIRS_THERMAL_CONSTANTS *ias_cpf_get_tirs_thermal_constants
(
    IAS_CPF *cpf
)
{
    if (!cpf->tirs_thermal_constants_loaded)
    {
        int status;

        status = ias_cpf_parse_tirs_thermal_constants(cpf, 
                                &cpf->tirs_thermal_constants);
        if (status != SUCCESS)
        {
           IAS_LOG_ERROR("Reading CPF TIRS thermal constant data");
           return NULL;
        }
        cpf->tirs_thermal_constants_loaded = 1;
    }
    return &cpf->tirs_thermal_constants;
}

/*****************************************************************************
NAME:  ias_cpf_get_tirs_det_status_blind

PURPOSE: Helper function to call for the load of the blind detector status group
         from the odl tree to the blind detector status structure

RETURNS: Pointer to the blind detector status structure

******************************************************************************/
const struct IAS_CPF_DETECTOR_STATUS *ias_cpf_get_tirs_det_status_blind
(
    IAS_CPF *cpf
)
{
    if (!cpf->tirs_det_status_blind_loaded)
    {
        int status;
    
        status = ias_cpf_parse_tirs_det_status_blind(cpf, 
                                &cpf->tirs_det_status_blind);
        if (status != SUCCESS)
        {
           IAS_LOG_ERROR("Reading CPF TIRS det status blind data");
            return NULL;
        }
        cpf->tirs_det_status_blind_loaded = 1;
    }
    return &cpf->tirs_det_status_blind;
}

/*****************************************************************************
NAME:  ias_cpf_get_tirs_det_response_blind

PURPOSE: Helper function to call for the load of the tirs detector response
         blind band group from the odl tree to the det response structure

RETURNS: Pointer to the det response structure blind band 

******************************************************************************/
const struct IAS_CPF_TIRS_DETECTOR_RESPONSE *ias_cpf_get_tirs_det_response_blind
(
    IAS_CPF *cpf
)
{
    if (!cpf->tirs_det_response_blind_loaded)
    {
        int status;
    
        status = ias_cpf_parse_tirs_detector_response_blind(cpf, 
                                            &cpf->tirs_det_response_blind);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF det response blind data");
            return NULL;
        }
        cpf->tirs_det_response_blind_loaded = 1;
    }
    return &cpf->tirs_det_response_blind;
}

/*****************************************************************************
NAME:  ias_cpf_get_tirs_det_response

PURPOSE: Helper function to call for the load of the tirs detector response
         group from the odl tree to the det response structure

RETURNS: Pointer to the det response structure

******************************************************************************/
const struct IAS_CPF_TIRS_DETECTOR_RESPONSE *ias_cpf_get_tirs_det_response
(
    IAS_CPF *cpf
)
{
    if (!cpf->tirs_det_response_loaded)
    {
        int status;
    
        status = ias_cpf_parse_tirs_detector_response(cpf, 
                                            &cpf->tirs_det_response);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF detector response data");
            return NULL;
        }
        cpf->tirs_det_response_loaded = 1;
    }
    return &cpf->tirs_det_response;
}

/*****************************************************************************
NAME:  ias_cpf_get_tirs_align_cal

PURPOSE: Helper function to call for the load of the tirs align cal group
         from the odl tree to the tirs_align_cal structure

RETURNS: Pointer to the tirs_align_cal structure

******************************************************************************/
const struct IAS_CPF_TIRS_ALIGN_CAL *ias_cpf_get_tirs_align_cal
(
    IAS_CPF *cpf
)
{
    if (!cpf->tirs_align_cal_loaded)
    {
        int status;
    
        status = ias_cpf_parse_tirs_align_cal(cpf, &cpf->tirs_align_cal);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF TIRS align cal data");
            return NULL;
        }
        cpf->tirs_align_cal_loaded = 1;
    }
    return &cpf->tirs_align_cal;
}

/*****************************************************************************
NAME:  ias_cpf_get_oli_avg_bias

PURPOSE: Helper function to call for the load of the average bias group
         from the odl tree to the average bias structure

RETURNS: Pointer to the average bias structure

******************************************************************************/
const struct IAS_CPF_AVERAGE_BIAS *ias_cpf_get_oli_avg_bias
(
    IAS_CPF *cpf
)
{
    if (!cpf->avg_bias_loaded)
    {
        int status;
    
        status = ias_cpf_parse_oli_avg_bias(cpf, &cpf->oli_avg_bias);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF average bias data");
            return NULL;
        }
        cpf->avg_bias_loaded = 1;
    }
    return &cpf->oli_avg_bias;
}

/*****************************************************************************
NAME:  ias_cpf_get_tirs_abs_gains_blind

PURPOSE: Helper function to call for the load of the tirs abs gains blind group
         from the odl tree to the tirs abs gains blind structure

RETURNS: Pointer to the tirs abs gains blind structure

******************************************************************************/
const struct IAS_CPF_ABSOLUTE_GAINS *ias_cpf_get_tirs_abs_gains_blind
(
    IAS_CPF *cpf
)
{
    if (!cpf->tirs_abs_gains_blind_loaded)
    {
        int status;
    
        status = ias_cpf_parse_tirs_abs_gains_blind(cpf, 
                                    &cpf->tirs_abs_gains_blind);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF TIRS abs gains blind data");
            return NULL;
        }
        cpf->tirs_abs_gains_blind_loaded = 1;
    }
    return &cpf->tirs_abs_gains_blind;
}

/*****************************************************************************
NAME:  ias_cpf_get_lunar_irradiance

PURPOSE: Helper function to call for the load of the lunar irradiance group
         from the odl tree to the lunar irradiance structure

RETURNS: Pointer to the lunar irradiance structure

******************************************************************************/
const struct IAS_CPF_LUNAR_IRRADIANCE *ias_cpf_get_lunar_irradiance
(
    IAS_CPF *cpf
)
{
    if (!cpf->lunar_irrad_loaded)
    {
        int status;
    
        status = ias_cpf_parse_lunar_irradiance(cpf, &cpf->lunar_irrad);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF lunar irradiance");
            return NULL;
        }
        cpf->lunar_irrad_loaded = 1;
    }
    return &cpf->lunar_irrad;
}

/*****************************************************************************
NAME:  ias_cpf_get_lamp_radiance

PURPOSE: Helper function to call for the load of the lamp radiance group
         from the odl tree to the lamp radiance structure

RETURNS: Pointer to the lamp radiance structure

******************************************************************************/
const struct IAS_CPF_LAMP_RADIANCE *ias_cpf_get_lamp_radiance
(
    IAS_CPF *cpf
)
{
    if (!cpf->lamp_rad_loaded)
    {
        int status;
    
        status = ias_cpf_parse_lamp_radiance(cpf, &cpf->lamp_rad);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF lamp radiance data");
            return NULL;
        }
        cpf->lamp_rad_loaded = 1;
    }
    return &cpf->lamp_rad;
}

/*****************************************************************************
NAME:  ias_cpf_get_impulse_noise

PURPOSE: Helper function to call for the load of the OLI and TIRS 
         impulse noise group from the odl tree to the impulse noise structure,
         includes normal, vrp, and blind bands

RETURNS: Pointer to the impulse noise structure

******************************************************************************/
const struct IAS_CPF_IMPULSE_NOISE *ias_cpf_get_impulse_noise
(
    IAS_CPF *cpf
)
{
    if (!cpf->impulse_noise_loaded)
    {
        int status;
    
        status = ias_cpf_parse_impulse_noise(cpf, &cpf->impulse_noise);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF impulse_noise_ data");
            return NULL;
        }
        cpf->impulse_noise_loaded = 1;
    }
    return &cpf->impulse_noise;
}

/*****************************************************************************
NAME:  ias_cpf_get_temp_sensitivity

PURPOSE: Helper function to call for the load of the temperature sensitivity 
         group from the odl tree to the temp sensitivity structure

RETURNS: Pointer to the temp sensitivity structure

******************************************************************************/
const struct IAS_CPF_TEMP_SENSITIVITY *ias_cpf_get_temp_sensitivity
(
    IAS_CPF *cpf
)
{
    if (!cpf->temp_sens_loaded)
    {
        int status;                     /* function return status */
        int band_index;                 /* band loop counter */
        int sca_index;                  /* sca loop counter */

        /* set the pointers to null */
        for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
        {
            for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
            {
                cpf->temp_sens.temp_sensitivity_coeff[band_index][sca_index]
                    = NULL;
            }
        }

        status = ias_cpf_parse_oli_temp_sens(cpf, &cpf->temp_sens);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF OLI temperature sensitivity data");
            return NULL;
        }

        status = ias_cpf_parse_tirs_temp_sens(cpf, &cpf->temp_sens);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF TIRS temperature sensitivity data");

            /* loop through the bands and scas to free them (although they are
               likely already freed) */
            for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
            {
                for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                {
                    free(cpf->temp_sens
                         .temp_sensitivity_coeff[band_index][sca_index]);
                    cpf->temp_sens
                         .temp_sensitivity_coeff[band_index][sca_index] = NULL;
                }
            }
            return NULL;
        }

        cpf->temp_sens_loaded = 1;
    }

    return &cpf->temp_sens;
}

/*****************************************************************************
NAME:  ias_cpf_get_reflect_conv

PURPOSE: Helper function to call for the load of the reflectance conversion 
         group from the odl tree to the _ structure

RETURNS: Pointer to the reflectance conversion structure

******************************************************************************/
const struct IAS_CPF_REFLECTANCE_CONVERSION *ias_cpf_get_reflect_conv
(
    IAS_CPF *cpf
)
{
    if (!cpf->reflect_conv_loaded)
    {
        int status;
    
        status = ias_cpf_parse_reflect_conv(cpf, &cpf->reflect_conv);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF reflectance conversion data");
            return NULL;
        }
        cpf->reflect_conv_loaded = 1;
    }
    return &cpf->reflect_conv;
}

/*****************************************************************************
NAME:  ias_cpf_get_saturation_level

PURPOSE: Helper function to call for the load of the saturation level group
         from the odl tree to the saturation level structure.  i
         Includes:
            oli & tirs normal bands
            oli & tirs blind bands 
            oli vrp bands.

RETURNS: Pointer to the saturation level structure
******************************************************************************/
const struct IAS_CPF_SATURATION_LEVEL *ias_cpf_get_saturation_level
(
    IAS_CPF *cpf
)
{
    if (!cpf->saturation_loaded)
    {
        int status;                         /* function return status */
        int band_index;                     /* band loop counter */
        int sca_index;                      /* sca loop counter */

        /* initialize the pointers in the sturcture to NULL here */
        for (band_index = 0; band_index < IAS_MAX_TOTAL_BANDS; band_index++)
        {
            for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
            {
                cpf->saturation.analog_low_saturation_level
                        [band_index][sca_index] = NULL;
                cpf->saturation.analog_high_saturation_level
                        [band_index][sca_index] = NULL;
                cpf->saturation.digital_low_saturation_level
                        [band_index][sca_index] = NULL;
                cpf->saturation.digital_high_saturation_level
                        [band_index][sca_index] = NULL;
            }
        }

        /* get the oli normal band data */
        status = ias_cpf_parse_saturation_level(cpf, IAS_OLI,
                                               "OLI_SATURATION_LEVEL",
                                               &cpf->saturation);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading OLI CPF saturation data");
            ias_cpf_free_saturation_level_memory(&cpf->saturation);    
            return NULL;
        }

        /* get the tirs normal band data */
        status = ias_cpf_parse_saturation_level(cpf, IAS_TIRS,
                                               "TIRS_SATURATION_LEVEL",
                                               &cpf->saturation);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading TIRS CPF normal band saturation data");
            /* free oli allocations */
            ias_cpf_free_saturation_level_memory(&cpf->saturation);    
            return NULL;
        }

        cpf->saturation_loaded = 1;
    }
    return &cpf->saturation;
}

/*****************************************************************************
NAME:  ias_cpf_get_relative_gains

PURPOSE: Helper function to call for the load of the relative gains group
         from the odl tree to the relative gains structure

RETURNS: Pointer to the relative gains structure

******************************************************************************/
const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_relative_gains
(
    IAS_CPF *cpf
)
{
    if (!cpf->rel_gains_loaded)
    {
        int status;                     /* function return status */
        int nbands;                     /* total number bands */
        int band_index;                 /* band loop counter */
        int sca_index;                  /* sca loop counter */
        int band_list[IAS_MAX_NBANDS];  /* array of band numbers */

        status = ias_cpf_parse_relative_gains(cpf, IAS_OLI, IAS_NORMAL_BAND,
                                              "OLI_RELATIVE_GAINS",
                                              "Rel_Gains", &cpf->rel_gains);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF OLI relative gains data");
            return NULL;
        }

        /* read the tirs rel gains */
        status = ias_cpf_parse_relative_gains(cpf, IAS_TIRS, 
                                              IAS_NORMAL_BAND,
                                              "TIRS_RELATIVE_GAINS",
                                              "Rel_Gains", &cpf->rel_gains);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF TIRS relative gains data");
            status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, 
                                                  IAS_NORMAL_BAND,
                                                  0, band_list, IAS_MAX_NBANDS,
                                                  &nbands);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Getting OLI band numbers");
                return NULL;
            }
            
            /* loop through the oli bands and scas */
            for (band_index = 0; band_index < nbands; band_index++)
            {
                for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                {
                    free(cpf->rel_gains.per_detector[band_index][sca_index]);
                    cpf->rel_gains.per_detector[band_index][sca_index] = NULL;
                }
            }
            return NULL;
        }

        /* set the loaded flag */
        cpf->rel_gains_loaded = 1;
    }
    return &cpf->rel_gains;
}

/*****************************************************************************
NAME:  ias_cpf_get_prelaunch_relative_gains

PURPOSE: Helper function to call for the load of the prelaunch relative gains
         group from the odl tree to the prelaunch relative gains structure

RETURNS: Pointer to the prelaunch relative gains structure

******************************************************************************/
const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_prelaunch_relative_gains
(
    IAS_CPF *cpf
)
{
    if (!cpf->pre_rel_gains_loaded)
    {
        int status;                     /* function return status */
        int nbands;                     /* total number bands */
        int band_index;                 /* band loop counter */
        int sca_index;                  /* sca loop counter */
        int band_list[IAS_MAX_NBANDS];  /* array of band numbers */

        status = ias_cpf_parse_relative_gains(cpf, IAS_OLI, IAS_NORMAL_BAND,
                                                  "OLI_PRE_RELATIVE_GAINS",
                                                  "Pre_Rel_Gains", 
                                                  &cpf->pre_rel_gains);

        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF pre launch rel gains data");
            return NULL;
        }

        /* read the tirs data */
        status = ias_cpf_parse_relative_gains(cpf, IAS_TIRS, 
                                                  IAS_NORMAL_BAND,
                                                  "TIRS_PRE_RELATIVE_GAINS",
                                                  "Pre_Rel_Gains", 
                                                  &cpf->pre_rel_gains);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading TIRS pre launch rel gains");
            /* free oli allocations */
            status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, 
                                                  IAS_NORMAL_BAND,
                                                  0, band_list, IAS_MAX_NBANDS,
                                                  &nbands);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Getting OLI band numbers");
                return NULL;
            }
            /* loop through the oli bands and scas */
            for (band_index = 0; band_index < nbands; band_index++)
            {
                for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                {
                    free(cpf->pre_rel_gains
                         .per_detector[band_index][sca_index]);
                    cpf->pre_rel_gains
                        .per_detector[band_index][sca_index] = NULL;
                }
            }
            return NULL;
        }

        cpf->pre_rel_gains_loaded = 1;
    }
    return &cpf->pre_rel_gains;
}

/*****************************************************************************
NAME:  ias_cpf_get_postlaunch_relative_gains

PURPOSE: Helper function to call for the load of the postlaunch relative gains
         group from the odl tree to the postlaunch relative gains structure

RETURNS: Pointer to the postlaunch relative gains structure

******************************************************************************/
const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_postlaunch_relative_gains
(
    IAS_CPF *cpf
)
{
    if (!cpf->post_rel_gains_loaded)
    {
        int status;                     /* function return status */
        int nbands;                     /* total number bands */
        int band_index;                 /* band loop counter */
        int sca_index;                  /* sca loop counter */
        int band_list[IAS_MAX_NBANDS];  /* array of band numbers */
    
        /* read the oli data */
        status = ias_cpf_parse_relative_gains(cpf, IAS_OLI, IAS_NORMAL_BAND,
                                                  "OLI_POST_RELATIVE_GAINS",
                                                  "Post_Rel_Gains", 
                                                  &cpf->post_rel_gains);

        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading OLI post launch rel gains");
            return NULL;
        }

        /* read the tirs data */
        status = ias_cpf_parse_relative_gains(cpf, IAS_TIRS, 
                                                  IAS_NORMAL_BAND,
                                                  "TIRS_POST_RELATIVE_GAINS",
                                                  "Post_Rel_Gains", 
                                                  &cpf->post_rel_gains);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading TIRS post launch rel gains");
            /* free oli allocations */
            status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, 
                                                  IAS_NORMAL_BAND,
                                                  0, band_list, IAS_MAX_NBANDS,
                                                  &nbands);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Getting OLI band numbers");
                return NULL;
            }
            /* loop through the oli bands and scas */
            for (band_index = 0; band_index < nbands; band_index++)
            {
                for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                {
                    free(cpf->post_rel_gains
                            .per_detector[band_index][sca_index]);
                    cpf->post_rel_gains
                            .per_detector[band_index][sca_index] = NULL;
                }
            }
            return NULL;
        }

        /* set the loaded flag */
        cpf->post_rel_gains_loaded = 1;
    }
    return &cpf->post_rel_gains;
}

/*****************************************************************************
NAME:  ias_cpf_get_tirs_rel_gains_blind

PURPOSE: Helper function to call for the load of the tirs relative gains 
         blind band group from the odl tree to the cpf structure

         NOTE: there is only one blind band and to utilize previously 
         declared rel gains structure we are using a band loop in the
         ias_cpf_parse_tirs_rel_gains_blind band function.

RETURNS: Pointer to the tirs rel gains blind band structure

******************************************************************************/
const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_tirs_rel_gains_blind
(
    IAS_CPF *cpf
)
{
    if (!cpf->tirs_rel_gains_blind_loaded)
    {
        int status;

        /* read the tirs rel gains blind band */
        status = ias_cpf_parse_tirs_rel_gains_blind(cpf,
                                                  "TIRS_REL_GAINS_BLIND",
                                                  "Rel_Gains_Blind", 
                                                  &cpf->tirs_rel_gains_blind);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR(
                "Reading CPF TIRS rel gains blind band data");
            return NULL;
        }

        /* set the loaded flag */
        cpf->tirs_rel_gains_blind_loaded = 1;
    }
    return &cpf->tirs_rel_gains_blind;
}

/*****************************************************************************
NAME:  ias_cpf_get_tirs_pre_rel_gains_blind

PURPOSE: Helper function to call for the load of the tirs relative gains 
         blind band group from the odl tree to the cpf structure

RETURNS: Pointer to the tirs rel gains blind band structure

******************************************************************************/
const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_tirs_pre_rel_gains_blind
(
    IAS_CPF *cpf
)
{
    if (!cpf->tirs_pre_rel_gains_blind_loaded)
    {
        int status;

        /* read the tirs rel gains */
        status = ias_cpf_parse_tirs_rel_gains_blind(cpf,
                                              "TIRS_PRE_REL_GAINS_BLIND",
                                              "Pre_Rel_Gains_Blind", 
                                              &cpf->tirs_pre_rel_gains_blind);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR(
                "Reading CPF TIRS pre rel gains blind band data");
            return NULL;
        }

        /* set the loaded flag */
        cpf->tirs_pre_rel_gains_blind_loaded = 1;
    }
    return &cpf->tirs_pre_rel_gains_blind;
}

/*****************************************************************************
NAME:  ias_cpf_get_tirs_pre_rel_gains_blind

PURPOSE: Helper function to call for the load of the tirs relative gains 
         blind band group from the odl tree to the cpf structure

RETURNS: Pointer to the tirs rel gains blind band structure

******************************************************************************/
const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_tirs_post_rel_gains_blind
(
    IAS_CPF *cpf
)
{
    if (!cpf->tirs_post_rel_gains_blind_loaded)
    {
        int status;

        /* read the tirs rel gains */
        status = ias_cpf_parse_tirs_rel_gains_blind(cpf,
                                             "TIRS_POST_REL_GAINS_BLIND",
                                             "Post_Rel_Gains_Blind", 
                                             &cpf->tirs_post_rel_gains_blind);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR(
                "Reading CPF TIRS post rel gains blind band data");
            return NULL;
        }

        /* set the loaded flag */
        cpf->tirs_post_rel_gains_blind_loaded = 1;
    }
    return &cpf->tirs_post_rel_gains_blind;
}

/*****************************************************************************
NAME:  ias_cpf_get_i2i_assessment

PURPOSE: Helper function to call for the load of the OLI & TIRS 
         i2i assessment group from the odl tree to the i2i assessment structure

RETURNS: Pointer to the i2i assessment structure

******************************************************************************/
const struct IAS_CPF_I2I_ASSESSMENT *ias_cpf_get_i2i_assessment
(
    IAS_CPF *cpf
)
{
    if (!cpf->i2i_assess_loaded)
    {
        int status;
    
        status = ias_cpf_parse_i2i_assessment(cpf, &cpf->i2i_assess);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF i2i assessment data");
            return NULL;
        }
        cpf->i2i_assess_loaded = 1;
    }
    return &cpf->i2i_assess;
}

/*****************************************************************************
NAME:  ias_cpf_get_geo_system

PURPOSE: Helper function to call for the load of the geo system group
         from the odl tree to the geo system structure

RETURNS: Pointer to the geo system structure

******************************************************************************/

const struct IAS_CPF_GEO_SYSTEM *ias_cpf_get_geo_system
(
    IAS_CPF *cpf
)
{
    if (!cpf->geo_sys_loaded)
    {
        int status;
    
        status = ias_cpf_parse_geo_system(cpf, &cpf->geo_sys);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF geo system data");
            return NULL;
        }
        cpf->geo_sys_loaded = 1;
    }
    return &cpf->geo_sys;
}

/*****************************************************************************
NAME:  ias_cpf_get_gcp_correlation

PURPOSE: Helper function to call for the load of the gcp correlation group
         from the odl tree to the gcp correlation structure

RETURNS: Pointer to the gcp correlation structure

******************************************************************************/
const struct IAS_CPF_GCP_CORRELATION *ias_cpf_get_gcp_correlation
(
    IAS_CPF *cpf
)
{
    if (!cpf->gcp_corr_loaded)
    {
        int status;

        status = ias_cpf_parse_gcp_correlation(cpf, &cpf->gcp_corr);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF gcp correlation data");
            return NULL;
        }
        cpf->gcp_corr_loaded = 1;
    }

    return &cpf->gcp_corr;
}

/*****************************************************************************
NAME:  ias_cpf_get_focal_plane_cal

PURPOSE: Helper function to call for the load of the focal plane cal group
         from the odl tree to the focal plane cal structure

RETURNS: Pointer to the focal plane cal structure

******************************************************************************/
const struct IAS_CPF_FOCAL_PLANE_CAL *ias_cpf_get_focal_plane_cal
(
    IAS_CPF *cpf
)
{
    if (!cpf->fp_cal_loaded)
    {
        int status;

        /* read the correct part of CPF file in as an ODL tree */
        status = ias_cpf_parse_focal_plane_cal(cpf, &cpf->fp_cal);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF focal plane cal data");
            return NULL;
        }
        cpf->fp_cal_loaded = 1;
    }

    return &cpf->fp_cal;
}

/*****************************************************************************
NAME:  ias_cpf_get_nonuniformity

PURPOSE: Helper function to call for the load of the nonuniformity group
         from the odl tree to the nonuniformity structure

RETURNS: Pointer to the nonuniformity structure

******************************************************************************/
const struct IAS_CPF_NONUNIFORMITY *ias_cpf_get_nonuniformity
(
    IAS_CPF *cpf
)
{
    if (!cpf->nonuniformity_loaded)
    {
        int status;                     /* function return status */
        int band_index;                 /* band loop control */
        int sca_index;                  /* sca loop control */

        /* initialize the struct elements here */
        for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
        {
            for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
            {
                cpf->nonuniformity.scale_factor_1
                            [band_index][sca_index] = NULL;
                cpf->nonuniformity.scale_factor_2
                            [band_index][sca_index] = NULL;
            }
        }
 
        /* get the oli group */
        status = ias_cpf_parse_nonuniformity(cpf, IAS_OLI, "OLI_NONUNIFORMITY",
                                            &cpf->nonuniformity);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF OLI nonuniformity data");
            ias_cpf_free_nonuniformity_memory(&cpf->nonuniformity);
            return NULL;
        }

        /* get the tirs group */
        status 
            = ias_cpf_parse_nonuniformity(cpf, IAS_TIRS, "TIRS_NONUNIFORMITY",
                                          &cpf->nonuniformity);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF TIRS nonuniformity data");
            ias_cpf_free_nonuniformity_memory(&cpf->nonuniformity);
            return NULL;
        }
        cpf->nonuniformity_loaded = 1;
    }
    
    return &cpf->nonuniformity;
}

/*****************************************************************************
NAME:  ias_cpf_get_focal_plane

PURPOSE: Helper function to call for the load of the oli & tirs focal plane
         groups from the odl tree to the focal_plane structure

RETURNS: Pointer to the focal_plane structure

******************************************************************************/
const struct IAS_CPF_FOCAL_PLANE *ias_cpf_get_focal_plane
(
    IAS_CPF *cpf
)
{
    if (!cpf->focal_plane_loaded)
    {
        int status;

        /* read the oli data */
        status = ias_cpf_parse_oli_focal_plane(cpf, &cpf->focal_plane);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF OLI focal_plane data");
            return NULL;
        }

        /* read the tirs data */
        status = ias_cpf_parse_tirs_focal_plane(cpf, &cpf->focal_plane);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF TIRS focal_plane data");
            return NULL;
        }

        /* set the loaded flag */
        cpf->focal_plane_loaded = 1;
    }
    
    return &cpf->focal_plane;
}

/*****************************************************************************
NAME:  ias_cpf_get_diffuser_rad

PURPOSE: Helper function to call for the load of the diffuser radiance group
         from the odl tree to the diffuser radiance structure

RETURNS: Pointer to the diffuser radiance structure

******************************************************************************/

const struct IAS_CPF_DIFFUSER_RADIANCE *ias_cpf_get_diffuser_rad
(
    IAS_CPF *cpf
)
{
    if (!cpf->diffuser_rad_loaded)
    {
        int status;
        
        status = ias_cpf_parse_diffuser_rad(cpf, &cpf->diffuser_rad);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF diffuser radiance data");
            return NULL;
        }
        cpf->diffuser_rad_loaded = 1;
    }
    
    return &cpf->diffuser_rad;
}

/*****************************************************************************
NAME:  ias_cpf_get_det_status

PURPOSE: Helper function to call for the load of the detector status group
         from the odl tree to the detector status structure

RETURNS: Pointer to the detector status structure

*****************************************************************************/
const struct IAS_CPF_DETECTOR_STATUS *ias_cpf_get_det_status
(
    IAS_CPF *cpf
)
{
    if (!cpf->detector_status_loaded)
    {
        int status;                     /* function return status */
        int nbands;                     /* total number bands */
        int band_index;                 /* band loop counter */
        int sca_index;                  /* sca loop counter */
        int band_list[IAS_MAX_NBANDS];  /* array of band numbers */

        status = ias_cpf_parse_oli_det_status(cpf, &cpf->detector_status);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF detector status data");
            return NULL;
        }
        status = ias_cpf_parse_tirs_det_status(cpf, &cpf->detector_status);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF TIRS detector status data");
            /* free oli allocations */
            status = ias_sat_attr_get_any_sensor_band_numbers(IAS_OLI,
                IAS_NORMAL_BAND | IAS_BLIND_BAND | IAS_VRP_BAND, 0, band_list,
                IAS_MAX_TOTAL_BANDS, &nbands);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Getting OLI band numbers");
                return NULL;
            }
            /* loop through the oli bands and scas */
            for (band_index = 0; band_index < nbands; band_index++)
            {
                for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                {
                    free(cpf->detector_status
                            .out_of_spec[band_index][sca_index]);
                    cpf->detector_status
                            .out_of_spec[band_index][sca_index] = NULL;
                    free(cpf->detector_status
                            .inoperable[band_index][sca_index]);
                    cpf->detector_status
                            .inoperable[band_index][sca_index] = NULL;
                }
            }
            return NULL;
        }

        cpf->detector_status_loaded = 1;
    }
    
    return &cpf->detector_status;
}

/*****************************************************************************
NAME:  ias_cpf_get_det_noise

PURPOSE: Helper function to call for the load of the detector noise group
         OLI and TIRS from the odl tree to the detector noise structure
         Will include normal, blind and vrp bands.

RETURNS: Pointer to the detector noise structure

*****************************************************************************/
const struct IAS_CPF_DETECTOR_NOISE *ias_cpf_get_det_noise
(
    IAS_CPF *cpf
)
{
    if (!cpf->detector_noise_loaded)
    {
        int status;                     /* function return status */
        int band_index;                 /* band loop counter */
        int sca_index;                  /* sca loop counter */

        /* initialize the pointers in the structure to NULL here */
        for (band_index = 0; band_index < IAS_MAX_TOTAL_BANDS; band_index++)
        {
            for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
            {
                cpf->detector_noise
                   .per_detector[band_index][sca_index] = NULL;
            }
        }

        /* read the oli det noise */
        status = ias_cpf_parse_oli_det_noise(cpf, &cpf->detector_noise);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF OLI detector noise data");
            return NULL;
        }

        /* read the tirs det noise */
        status = ias_cpf_parse_tirs_det_noise(cpf, &cpf->detector_noise);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF TIRS detector noise data");

            /* free oli allocations, we use IAS_MAX_TOTAL_BANDS because this
               includes normal, vrp and blind bands */
            for (band_index = 0; band_index < IAS_MAX_TOTAL_BANDS; band_index++)
            {
                for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                {
                    free(cpf->detector_noise
                            .per_detector[band_index][sca_index]);
                    cpf->detector_noise
                        .per_detector[band_index][sca_index] = NULL;
                }
            }
            return NULL;
        }

        cpf->detector_noise_loaded = 1;
    }
    
    return &cpf->detector_noise;
}

/*****************************************************************************
NAME:  ias_cpf_get_b2b_assessment

PURPOSE: Helper function to call for the load of the b2b assessment group
         from the odl tree to the b2b assessment structure

RETURNS: Pointer to the b2b assessment structure

******************************************************************************/
const struct IAS_CPF_B2B_ASSESSMENT *ias_cpf_get_b2b_assessment
(
    IAS_CPF *cpf
)
{
    if (!cpf->b2b_assess_loaded)
    {
        int status;                         /* function return value */

        status = ias_cpf_parse_b2b_assessment(cpf, &cpf->b2b_assess);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF OLI b2b_assessment data");
            return NULL;
        }

        /* set the loaded flag */
        cpf->b2b_assess_loaded = 1;
    }
    
    return &cpf->b2b_assess;
}

/*****************************************************************************
NAME:  ias_cpf_get_attitude_params

PURPOSE: Helper function to call for the load of the attitude parameters group
         from the odl tree to the cpf attitude parameters structure

RETURNS: Pointer to the _ structure

******************************************************************************/
const struct IAS_CPF_ATTITUDE_PARAMETERS *ias_cpf_get_attitude_params
(
    IAS_CPF *cpf
)
{
    if (!cpf->attitude_loaded)
    {
        int status;
        
        status = ias_cpf_parse_attitude_params(cpf, &cpf->attitude);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF attitude params data");
            return NULL;
        }
        cpf->attitude_loaded = 1;
    }

    return &cpf->attitude;
} /* end of ias cpf get attitude params */

/*****************************************************************************
NAME:  ias_cpf_get_ancil_conv

PURPOSE: Helper function to call for the load of the ancil conv group
         from the odl tree to the ancil conv structure

RETURNS: Pointer to the ancil conversion structure

******************************************************************************/
const struct IAS_CPF_ANCILLARY_ENG_CONV *ias_cpf_get_ancil_conv
(
    IAS_CPF *cpf
)
{
    if (!cpf->ancil_eng_conv_loaded)
    {
        int status;
        
        status = ias_cpf_parse_ancil_conv(cpf, &cpf->ancil_eng_conv);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF ancil eng conv data");
            return NULL;
        }
        cpf->ancil_eng_conv_loaded = 1;
    }
    
    return &cpf->ancil_eng_conv;
}

/*****************************************************************************
NAME:  ias_cpf_get_ancil_qa_thresholds

PURPOSE: Helper function to call for the load of the ancillary data group
         from the odl tree to the ancil data structure

RETURNS: Pointer to the ancillary data structure

******************************************************************************/
const struct IAS_CPF_ANCILLARY_QA_THRESHOLDS *ias_cpf_get_ancil_qa_thresholds
(
    IAS_CPF *cpf
)
{
    if (!cpf->ancil_qa_thresh_loaded)
    {
        int status;
        
        status = ias_cpf_parse_ancil_qa_thresholds(cpf, 
                                                   &cpf->ancil_qa_thresholds);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF ancillary qa thresholds ");
            return NULL;
        }
        cpf->ancil_qa_thresh_loaded = 1;
    }
    
    return &cpf->ancil_qa_thresholds;
}

/*****************************************************************************
NAME:  ias_cpf_get_abs_gains

PURPOSE: Helper function to call for the load of the absolute gains data
         from the odl tree to the abs gains structure
         
RETURNS: Pointer to abs gains data structure.

******************************************************************************/
const struct IAS_CPF_ABSOLUTE_GAINS *ias_cpf_get_abs_gains
(
    IAS_CPF *cpf
)
{
    if (!cpf->abs_gains_loaded)
    {
        int status;                         /* function return value */

        /* get oli abs gains and stow in abs_gains struct first */
        status = ias_cpf_parse_oli_abs_gains(cpf, &cpf->abs_gains);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading OLI abs gains");
            return NULL;
        }

        /* get tirs abs gains and stow in tirs abs_gains struct */
        status = ias_cpf_parse_tirs_abs_gains(cpf, &cpf->abs_gains);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading TIRS abs gains");
            return NULL;
        }
        
        /* set the loaded flag */
        cpf->abs_gains_loaded = 1;
    }
    return &cpf->abs_gains;

} /* end of ias cpf get abs gains */

/*****************************************************************************
NAME:  ias_cpf_get_earth_const

PURPOSE: Helper function to call for the load of the earth constant data
         from the odl tree to the cpf earth constants structure.

RETURNS: Pointer to earth constants data structure.

******************************************************************************/
const struct IAS_CPF_EARTH_CONSTANTS *ias_cpf_get_earth_const
(
    IAS_CPF *cpf
)
{
    if (!cpf->earth_loaded)
    {
        int status;                 /* function return value */

        status = ias_cpf_parse_earth_const(cpf, &cpf->earth);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF earth constants");
            return NULL;
        }
        cpf->earth_loaded = 1;
    }
    return &cpf->earth;

}  /* end of get earth constants */

/*****************************************************************************

NAME: ias_cpf_get_det_offsets

PURPOSE: Returns a pointer to the det offsets structure

RETURNS:  The det offsets pointer, or NULL if not initialized.

******************************************************************************/
const struct IAS_CPF_DETECTOR_OFFSETS *ias_cpf_get_det_offsets
(
    IAS_CPF *cpf
)
{
    if (!cpf->detector_offsets_loaded)
    {
        int status;                     /* function return status */
        int nbands;                     /* total number bands */
        int band_index;                 /* band loop counter */
        int sca_index;                  /* sca loop counter */
        int band_list[IAS_MAX_NBANDS];  /* array of band numbers */

        status = ias_cpf_parse_oli_det_offsets(cpf, &cpf->detector_offsets);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF detector offsets");
            return NULL;
        }

        status = ias_cpf_parse_tirs_det_offsets(cpf, &cpf->detector_offsets);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF TIRS detector offsets");

            /* free oli allocations */
            status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, 
                                                  IAS_NORMAL_BAND,
                                                  0, band_list, IAS_MAX_NBANDS,
                                                  &nbands);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Getting OLI band numbers");
                return NULL;
            }
            /* loop through the oli bands and scas */
            for (band_index = 0; band_index < nbands; band_index++)
            {
                for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                {
                    free(cpf->detector_offsets
                            .along_per_detector[band_index][sca_index]);
                    cpf->detector_offsets
                            .along_per_detector[band_index][sca_index] = NULL;
                    free(cpf->detector_offsets
                            .across_per_detector[band_index][sca_index]);
                    cpf->detector_offsets
                            .across_per_detector[band_index][sca_index] = NULL;
                }
            }
            return NULL;
        }

        cpf->detector_offsets_loaded = 1;
    }

    return &cpf->detector_offsets;

}

/*****************************************************************************

NAME: ias_cpf_get_file_attributes

PURPOSE: Returns a pointer to the file attributes structure 

RETURNS:  The file attributes pointer, or NULL if not initialized.

******************************************************************************/
const struct IAS_CPF_FILE_ATTRIBUTES *ias_cpf_get_file_attributes
(
    IAS_CPF *cpf
)
{
    if (!cpf->file_attribs_loaded)
    {
        int status;                 /* function return value */

        status = ias_cpf_parse_file_attributes(cpf, &cpf->file_attribs);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF file attributes");
            return NULL;
        }
        cpf->file_attribs_loaded = 1;
    }
    return &cpf->file_attribs;

}  /* end of get file atts */

/*****************************************************************************

NAME: ias_cpf_get_orbit

PURPOSE: Returns a pointer to the orbit parameters

RETURNS:  The orbit parameters pointer, or NULL if not initialized.

******************************************************************************/
const struct IAS_CPF_ORBIT_PARAMETERS *ias_cpf_get_orbit
(
    IAS_CPF *cpf
)
{
    if (!cpf->orbit_loaded)
    {
        int status;                 /* function return value */

        status = ias_cpf_parse_orbit_parameters(cpf, &cpf->orbit);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF orbit parameters");
            return NULL;
        }
        cpf->orbit_loaded = 1;
    }
    return &cpf->orbit;

}

/*****************************************************************************

NAME: ias_cpf_get_oli_parameters

PURPOSE: Returns a pointer to the OLI parameters

RETURNS:  The sensor parameters pointer, or NULL if not initialized.

******************************************************************************/
const struct IAS_CPF_OLI_PARAMETERS *ias_cpf_get_oli_parameters
(
    IAS_CPF *cpf
)
{
    if (!cpf->oli_parameter_loaded)
    {
        int status;                          /* function return value */

        /* place the oli params in the oli parameter struct */
        status = ias_cpf_parse_oli_parameters(cpf, &cpf->oli_parameters);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF OLI parameters");
            return NULL;
        }

        /* set the loaded flag */
        cpf->oli_parameter_loaded = 1;
    }
    return &cpf->oli_parameters;
}

/*****************************************************************************

NAME: ias_cpf_get_tirs_parameters

PURPOSE: Returns a pointer to the TIRS parameters

RETURNS:  The tirs parameters pointer, or NULL if not initialized.

******************************************************************************/
const struct IAS_CPF_TIRS_PARAMETERS *ias_cpf_get_tirs_parameters
(
    IAS_CPF *cpf
)
{
    if (!cpf->tirs_parameter_loaded)
    {
        int status;                          /* function return value */

        /* place the tirs params in the oli parameter struct */
        status = ias_cpf_parse_tirs_parameters(cpf, &cpf->tirs_parameters);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF TIRS parameters");
            return NULL;
        }

        /* set the loaded flag */
        cpf->tirs_parameter_loaded = 1;
    }
    return &cpf->tirs_parameters;
}
/*****************************************************************************

NAME: ias_cpf_get_oli_sca_parameters

PURPOSE: Returns a pointer to the SCA parameters

RETURNS:  The SCA parameters pointer, or NULL if not initialized.

******************************************************************************/
const struct IAS_CPF_SCA_PARAMETERS *ias_cpf_get_sca_parameters
(
    IAS_CPF *cpf
)
{
    if (!cpf->sca_parm_loaded)
    {
        int status;                      /* function return value */
        int band_index;                  /* band loop counter */
        int normal_band_index;           /* index of normal band number */
        int nbands;                      /* sensor total band count */
        int nscas;                       /* sensor total sca count */
        int sca_index;                   /* sca loop counter */
        int band_list[IAS_MAX_NBANDS];   /* array of sensor band numbers */

        IAS_CPF_SCA_PARAMETERS tirs_sca; /* struct to get the tirs data */
        
        /* get sca count */
        nscas = ias_sat_attr_get_sensor_sca_count(IAS_TIRS);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting TIRS sca count");
            return NULL;
        }


        status = ias_cpf_parse_sca_parameters(cpf, IAS_OLI, 
                                              "OLI_SCA_PARAMETERS",
                                               &cpf->sca_parms);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF OLI sca parameters");
            return NULL;
        }
        status = ias_cpf_parse_sca_parameters(cpf, IAS_TIRS, 
                                              "TIRS_SCA_PARAMETERS",
                                               &tirs_sca);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF TIRS sca parameters");
            return NULL;
        }
        /* free oli allocations */
        status = ias_sat_attr_get_sensor_band_numbers(IAS_TIRS, 
                                              IAS_NORMAL_BAND,
                                              0, band_list, IAS_MAX_NBANDS,
                                              &nbands);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Getting TIRS band numbers");
            return NULL;
        }
        
        /* move the tirs data to the main struct */
        for (band_index = 0; band_index < nbands; band_index++)
        {
            /* get the index equivalent of the normal band number */
            normal_band_index = ias_sat_attr_convert_band_number_to_index(
                                                        band_list[band_index]); 
            if (normal_band_index == ERROR)
            {
                IAS_LOG_ERROR("Converting the band number to an index");
                return NULL;
            }

            /* move the tirs data to the main structure, there is one less 
               discontinuity ratio than there is scas so subtract one from
               the loop control var */
            for (sca_index = 0; sca_index < (nscas -1); sca_index++)
            {
                cpf
                  ->sca_parms.discontinuity_ratio[normal_band_index][sca_index]
                  = tirs_sca.discontinuity_ratio[band_index][sca_index];
            }
            cpf->sca_parms.stripe_cutoff[normal_band_index] 
                          = tirs_sca.stripe_cutoff[band_index]; 
            cpf->sca_parms.max_valid_correlation_shift[normal_band_index]
                          = tirs_sca.max_valid_correlation_shift[band_index];
            cpf->sca_parms.min_valid_neighbor_segments[normal_band_index]
                          = tirs_sca.min_valid_neighbor_segments[band_index];
            cpf->sca_parms.sca_overlap_threshold[normal_band_index]
                          = tirs_sca.sca_overlap_threshold[band_index];
        }

        cpf->sca_parm_loaded = 1;
    }

    return &cpf->sca_parms;
}

/*****************************************************************************

NAME: ias_cpf_get_ut1_times

PURPOSE: Returns a pointer to the UT1 time parameters

RETURNS:  The UT1 time parameters pointer, or NULL if not initialized.

******************************************************************************/
const struct IAS_CPF_UT1_TIME_PARAMETERS *ias_cpf_get_ut1_times
(
    IAS_CPF *cpf
)
{
    if (!cpf->ut1_times_loaded)
    {
        int status;                 /* function return value */

        status = ias_cpf_parse_ut1_times(cpf, &cpf->ut1_times);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF UT1 time parameters");
            return NULL;
        }
        cpf->ut1_times_loaded = 1;
    }
    return &cpf->ut1_times;

}

/*****************************************************************************

NAME: ias_cpf_get_cloud_cover_assessment

PURPOSE: Returns a pointer to the cloud cover assessment parameters

RETURNS:  The cloud cover assessment parameters pointer, or NULL if not
          initialized.

******************************************************************************/
const struct IAS_CPF_CLOUD_COVER_ASSESSMENT *ias_cpf_get_cloud_cover_assessment
(
    IAS_CPF *cpf
)
{
    if (!cpf->cc_assessment_loaded)
    {
        int status;                 /* function return value */

        status = ias_cpf_parse_cloud_cover_assessment(cpf, &cpf->cc_assessment);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading CPF cloud cover assessment parameters");
            return NULL;
        }
        cpf->cc_assessment_loaded = 1;
    }
    return &cpf->cc_assessment;

}

