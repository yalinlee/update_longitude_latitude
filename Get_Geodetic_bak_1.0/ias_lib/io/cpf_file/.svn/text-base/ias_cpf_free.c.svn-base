/*************************************************************************

NAME: ias_cpf_free

PURPOSE: Free all allocated memory.
        
RETURN VALUE: None

******************************************************************************/

#include <stdlib.h>
#include "ias_satellite_attributes.h"
#include "ias_cpf.h"
#include "local_defines.h"

/* Define a macro to free a pointer's memory and then set the pointer to NULL */
#define FREE_AND_NULL(a) \
do { free(a); \
    a = NULL; } \
while (0)
/* The do.. while(0) block ensures that an unbracketed if statement compiles:
    if (condition)
        FREE_AND_NULL(ptr);
    else
    {
        statement(s);
    }
    Without the do..while(0) block, the above code won't compile, since we have 
two statements in our macro.  Similarily,
    if (condition)
        FREE_AND_NULL(ptr);
will compile, but leads to a very, subtle run-time problem, as only the free is
covered by the if statement.  So, the do..while(0) solves these issues. */

void ias_cpf_free
(
    IAS_CPF *cpf
)
{
    int index;                  /* loop counter */
    int sca_index;                  /* sca loop counter */
    int band_index;                 /* band loop counter */
    int band_number;                /* actural band number */
    int nbands;                     /* number of bands */
    int band_list[IAS_MAX_NBANDS];  /* list of band numbers */
    int nscas = 0;                  /* number of scas */
    int status;                     /* function return status */

    status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, IAS_NORMAL_BAND,
                                                  0, band_list, IAS_MAX_NBANDS,
                                                  &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting OLI band numbers");
        return;
    }

    /* free up the pan band average bias */
    for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
    {
        FREE_AND_NULL(cpf->oli_avg_bias.bias_odd_pan[sca_index]);
        FREE_AND_NULL(cpf->oli_avg_bias.bias_even_pan[sca_index]);
    }

    /* free up groups shared by OLI and TIRS */
    for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
    {
        FREE_AND_NULL(cpf->abs_gains.gain[band_index]);
        FREE_AND_NULL(cpf->tirs_abs_gains_blind.gain[band_index]);
        /* sca loop */
        for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
        {
            FREE_AND_NULL(cpf->nonuniformity.scale_factor_1
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->nonuniformity.scale_factor_2
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->post_rel_gains.per_detector
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->pre_rel_gains.per_detector
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->temp_sens.temp_sensitivity_coeff
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->rel_gains.per_detector[band_index][sca_index]);
            FREE_AND_NULL(cpf->detector_offsets.along_per_detector
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->detector_offsets.across_per_detector
                [band_index][sca_index]);
        }
    }

    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* free the band level elements */
        band_number = band_list[band_index];

        /* get number of scas */
        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting sca count for band: %d", band_number);
            return;
        }

        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            FREE_AND_NULL(cpf->oli_avg_bias.bias_vnir[band_index][sca_index]);
            FREE_AND_NULL(cpf->oli_avg_bias.bias_swir[band_index][sca_index]);
            FREE_AND_NULL(cpf->lamp_rad.effective_rad_backup
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->lamp_rad.effective_rad_pristine
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->lamp_rad.effective_rad_working
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->diffuser_rad.diff_rad_prim
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->diffuser_rad.diff_rad_pris
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->diffuser_rad.diff_bidir_refl_prim
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->diffuser_rad.diff_bidir_refl_pris
                [band_index][sca_index]);
        }
    }

    /* free those elements that use total bands, normal, vrp and blind */
    for (band_index = 0; band_index < IAS_MAX_TOTAL_BANDS; band_index++)
    {
        for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
        {
            FREE_AND_NULL(cpf->detector_noise.per_detector
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->saturation.analog_low_saturation_level
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->saturation.analog_high_saturation_level
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->saturation.digital_low_saturation_level
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->saturation.digital_high_saturation_level
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->detector_status.out_of_spec
                [band_index][sca_index]);
            FREE_AND_NULL(cpf->detector_status.inoperable
                [band_index][sca_index]);
        }
    }

    /* Get the TIRS band attributs */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_TIRS, IAS_NORMAL_BAND,
                                                  0, band_list, IAS_MAX_NBANDS,
                                                  &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting sat TIRS band numbers");
        return;
    }

    /* get sca count for tirs sensor */
    nscas = ias_sat_attr_get_sensor_sca_count(IAS_TIRS);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sat sca count");
        return;
    }

    /* free up the tirs allocations */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            FREE_AND_NULL(cpf->tirs_det_response_blind
                .baseline_dark_response[band_index][sca_index]);
            FREE_AND_NULL(cpf->tirs_det_response_blind
                .background_response[band_index][sca_index]);
            FREE_AND_NULL(cpf->tirs_det_response_blind
                .gain_offsets[band_index][sca_index]);
            FREE_AND_NULL(cpf->tirs_det_response
                .baseline_dark_response[band_index][sca_index]);
            FREE_AND_NULL(cpf->tirs_det_response
                .background_response[band_index][sca_index]);
            FREE_AND_NULL(cpf->tirs_det_response
                .gain_offsets[band_index][sca_index]);
        }
    }

    /* get tirs sca count */
    nscas = ias_sat_attr_get_sensor_sca_count(IAS_TIRS);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sat sca count");
        return;
    }

    /* free tirs blind band allocations */
    /* NOTE: The tirs blind band is the only band loaded in this structure
       the band number and to make use of existing rel gains parse function
       another instance of the rel gains structure was declared, therefore,
       requiring the band element of the array.
     */
    for (sca_index = 0; sca_index < nscas; sca_index++)
    {
        FREE_AND_NULL(cpf->tirs_rel_gains_blind.per_detector[0][sca_index]);
        FREE_AND_NULL(cpf->tirs_pre_rel_gains_blind.per_detector[0][sca_index]);
        FREE_AND_NULL(cpf->tirs_post_rel_gains_blind.per_detector
            [0][sca_index]);
        FREE_AND_NULL(cpf->tirs_det_status_blind.inoperable[0][sca_index]);
        FREE_AND_NULL(cpf->tirs_det_status_blind.out_of_spec[0][sca_index]);
    }

    /* free non band and sca level elements */
    FREE_AND_NULL(cpf->earth.leap_seconds_data.leap_years);
    FREE_AND_NULL(cpf->earth.leap_seconds_data.leap_months);
    FREE_AND_NULL(cpf->earth.leap_seconds_data.leap_days);
    FREE_AND_NULL(cpf->earth.leap_seconds_data.num_leap_seconds);

    /* free cloud cover assessment elements */
    if (cpf->cc_assessment.run_if_thermal != NULL)
    {
        FREE_AND_NULL(cpf->cc_assessment.run_if_thermal);
    }
    if (cpf->cc_assessment.cca_class_type != NULL)
    {
        for (index = 0; index < cpf->cc_assessment.number_of_classes; index++)
        {
            FREE_AND_NULL(cpf->cc_assessment.cca_class_type[index]);
        }
        FREE_AND_NULL(cpf->cc_assessment.cca_class_type);
    }
    if (cpf->cc_assessment.algorithm_names != NULL)
    {
        for (index = 0;index < cpf->cc_assessment.number_of_algorithms;index++)
        {
            FREE_AND_NULL(cpf->cc_assessment.algorithm_names[index]);
            FREE_AND_NULL(cpf->cc_assessment.weights[index]);
        }
        FREE_AND_NULL(cpf->cc_assessment.algorithm_names);
        FREE_AND_NULL(cpf->cc_assessment.weights);
    }
    /* free the buffer with the file contents */
    FREE_AND_NULL(cpf->raw_file_buffer);

    /* free the CPF structure */
    free(cpf);
}
