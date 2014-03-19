/******************************************************************************
 NAME:                  ias_bpf_get_model_parameters

 PURPOSE:  Retrieves bias parameter model values from a BPF for the
           specified band number, SCA number, and band spectral type

 RETURNS:  Integer status code of SUCCESS or ERROR

 NOTES:    The current implementation returns 4 detector-specific parameters
           for OLI (pre-acquisition response, post-acquisition response,
           a1 coefficient, c1 coefficient); OLI also returns the a0
           coefficient for each SCA.

           For TIRS, the current implementation returns 2 detector-specific
           parameters-- the pre-acquisition response and the post-acquisition
           response.
******************************************************************************/
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#include "ias_types.h"
#include "ias_logging.h"
#include "ias_satellite_attributes.h"
#include "ias_bpf.h"

int ias_bpf_get_model_parameters
(
    const struct IAS_BPF_BIAS_MODEL *bias_model,
                               /* I: Fully populated band model data
                                  structure */
    IAS_SPECTRAL_TYPE spectral_type,
                               /* I: Expected spectral type for the
                                  specified band */
    int band_number,           /* I: 1-based band number */
    int sca_number,            /* I: 1-based SCA number */
    int num_detectors,         /* I: Total number of detectors in the
                                  current band/SCA */
    int even_odd,              /* I: Even/Odd indicator (used only for
                                  PAN band) */
    double *pre_acquisition_average,
                               /* O: Array of pre-acquisition average
                                  responses in the current band/SCA. */
    double *post_acquisition_average,
                               /* O: Array of post-acquisition average
                                  responses in the current band/SCA. */
    double *a1_coefficient,    /* O: Array of OLI a1_coefficient for
                                  all detectors in the current band/SCA.
                                  Pass in NULL if reading a TIRS BPF.  */
    double *c1_coefficient,    /* O: Array of OLI c1_coefficient for
                                  all detectors in the current band/SCA.
                                  Pass in NULL if reading a TIRS BPF.  */
    double *a0_coefficient     /* O: The OLI VRP model coefficient for the
                                  current band/SCA.  Pass in NULL if reading
                                  a TIRS BPF.  */
)
{
    int band_index = 0;        /* Index of the requested band within
                                  the BPF */
    int det_index = 0;         /* Detector loop counter */


    /* Sanity checks.  It's assumed the spectral type has been validated
       prior to entering this routine--the only values it can take on are
       IAS_SPECTRAL_VNIR, IAS_SPECTRAL_SWIR, IAS_SPECTRAL_PAN, or
       IAS_SPECTRAL_THERMAL.  */
    if (bias_model == NULL)
    {
        IAS_LOG_ERROR("No bias model data are available");
        return ERROR;
    }
    else
    {
        if ((pre_acquisition_average == NULL)
            || (post_acquisition_average == NULL))
        {
            IAS_LOG_ERROR("Invalid pre/post acquisition data buffers");
            return ERROR;
        }

        if (spectral_type != IAS_SPECTRAL_THERMAL)
        {
            if ((a1_coefficient == NULL) || (c1_coefficient == NULL)
                || (a0_coefficient == NULL))
            {
                IAS_LOG_ERROR("Invalid OLI bias model coefficient data "
                    "buffers");
                return ERROR;
            }
        }
    }

    /* Considering the SCA number and detector number as fixed, determine
       the corresponding 0-based offsets that are needed */
    int sca_index = sca_number - 1;

    /* Validate input parameters */
    if ((sca_number < 1) || (sca_number > IAS_MAX_NSCAS))
    {
        IAS_LOG_ERROR("Invalid SCA number %d", sca_number);
        return ERROR;
    }

    /* Get the band index given the specified band number */
    band_index = ias_sat_attr_convert_band_number_to_index(band_number);
    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Invalid band index for specified band number %d",
                      band_number);
        return ERROR;
    }

    /* Make sure the input band spectral type matches what's expected */
    if (spectral_type != bias_model[band_index].spectral_type)
    {
        IAS_LOG_ERROR("BPF structure does not contain the expected "
                      "spectral type information for band number %d",
                      band_number);
        return ERROR;
    }
    else
    {
        /* If it's the PAN band, check the odd/even flag value */
        if ((spectral_type == IAS_SPECTRAL_PAN)
                && (even_odd != IAS_BPF_PAN_EVEN)
                && (even_odd != IAS_BPF_PAN_ODD))
        {
            IAS_LOG_ERROR("Invalid even/odd PAN band flag value %d",
                even_odd);
            return ERROR;
        }
    }

    /* Grab the proper set of coefficients */
    switch (spectral_type)
    {
        case IAS_SPECTRAL_VNIR:
        {
            for (det_index = 0; det_index < num_detectors; det_index++)
            {
                double *det_ptr = bias_model[band_index]
                    .vnir->det_params[sca_index][det_index];

                pre_acquisition_average[det_index]
                        = det_ptr[IAS_BPF_DETECTOR_PRE_AVG];
                post_acquisition_average[det_index]
                        = det_ptr[IAS_BPF_DETECTOR_POST_AVG];
                a1_coefficient[det_index] = det_ptr[IAS_BPF_DETECTOR_A1];
                c1_coefficient[det_index] = det_ptr[IAS_BPF_DETECTOR_C1];
            }

            /* Get the a0 coefficient */
            *a0_coefficient = bias_model[band_index]
                .vnir->a0_coefficient[sca_index];

            break;
        }
        case IAS_SPECTRAL_SWIR:
        {
            for (det_index = 0; det_index < num_detectors; det_index++)
            {
                double *det_ptr = bias_model[band_index]
                    .swir->det_params[sca_index][det_index];

                pre_acquisition_average[det_index]
                        = det_ptr[IAS_BPF_DETECTOR_PRE_AVG];
                post_acquisition_average[det_index]
                        = det_ptr[IAS_BPF_DETECTOR_POST_AVG];
                a1_coefficient[det_index] = det_ptr[IAS_BPF_DETECTOR_A1];
                c1_coefficient[det_index] = det_ptr[IAS_BPF_DETECTOR_C1];
            }

            /* Get the a0 coefficient */
            *a0_coefficient = bias_model[band_index]
                .swir->a0_coefficient[sca_index];

            break;
        }
        case IAS_SPECTRAL_PAN:
        {
            /* Treat the even and odd coefficients separately */
            if (even_odd == IAS_BPF_PAN_EVEN)
            {
                for (det_index = 0; det_index < num_detectors; det_index++)
                {
                    double *det_ptr = bias_model[band_index]
                        .pan->det_params_even[sca_index][det_index];

                    pre_acquisition_average[det_index]
                            = det_ptr[IAS_BPF_DETECTOR_PRE_AVG];
                    post_acquisition_average[det_index]
                            = det_ptr[IAS_BPF_DETECTOR_POST_AVG];
                    a1_coefficient[det_index] = det_ptr[IAS_BPF_DETECTOR_A1];
                    c1_coefficient[det_index] = det_ptr[IAS_BPF_DETECTOR_C1];
                }

                /* Get the a0 model coefficient */
                *a0_coefficient = bias_model[band_index]
                    .pan->a0_coefficient_even[sca_index];
            }
            else
            {
                for (det_index = 0; det_index < num_detectors; det_index++)
                {
                    double *det_ptr = bias_model[band_index]
                        .pan->det_params_odd[sca_index][det_index];

                    pre_acquisition_average[det_index]
                            = det_ptr[IAS_BPF_DETECTOR_PRE_AVG];
                    post_acquisition_average[det_index]
                            = det_ptr[IAS_BPF_DETECTOR_POST_AVG];
                    a1_coefficient[det_index] = det_ptr[IAS_BPF_DETECTOR_A1];
                    c1_coefficient[det_index] = det_ptr[IAS_BPF_DETECTOR_C1];
                }

                /* Get the a0 model coefficient */
                *a0_coefficient = bias_model[band_index]
                    .pan->a0_coefficient_odd[sca_index];
            }

            break;
        }
        case IAS_SPECTRAL_THERMAL:
        {
            for (det_index = 0; det_index < num_detectors; det_index++)
            {
                double *det_ptr = bias_model[band_index]
                    .thermal->det_params[sca_index][det_index];

                pre_acquisition_average[det_index] =
                    det_ptr[IAS_BPF_DETECTOR_PRE_AVG];
                post_acquisition_average[det_index] =
                    det_ptr[IAS_BPF_DETECTOR_POST_AVG];
            }
            break;
        }
        default:
        {
            IAS_LOG_ERROR("Unable to determine type of bias "
                    "parameters for band number %d", band_number);
            return ERROR;
        }
    }

    /* Done */
    return SUCCESS;

}
