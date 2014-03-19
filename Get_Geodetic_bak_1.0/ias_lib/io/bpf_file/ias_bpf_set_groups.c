/**************************************************************************
 NAME:               ias_bpf_set_groups

 PURPOSE:  Public interface routines to allow updating individual
           members of BPF groups

***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ias_bpf.h"
#include "bpf_local_defines.h"
#include "ias_satellite_attributes.h"
#include "ias_logging.h"


/*------------------------------------------------------------------------
 NAME:              ias_bpf_set_file_attributes_file_name

 PURPOSE:  Writes new/updated BPF file name to the FILE_ATTRIBUTES group

 RETURNS:  Nothing
--------------------------------------------------------------------------*/
void ias_bpf_set_file_attributes_file_name
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    const char *bpf_file_name          /* I: Name of BPF file to update */
)
{
    strncpy(bpf->file_attributes.file_name, bpf_file_name,
        sizeof(bpf->file_attributes.file_name));
}


/*------------------------------------------------------------------------
 NAME:              ias_bpf_set_file_attributes_file_source

 PURPOSE:  Writes new/updated BPF file source to the FILE_ATTRIBUTES group

 RETURNS:  Nothing
--------------------------------------------------------------------------*/
void ias_bpf_set_file_attributes_file_source
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    const char *bpf_file_source        /* I: BPF source file name to update */
)
{
    strncpy(bpf->file_attributes.file_source, bpf_file_source,
        sizeof(bpf->file_attributes.file_source));
}



/*------------------------------------------------------------------------
 NAME:         ias_bpf_set_file_attributes_effective_date_begin

 PURPOSE:  Writes new/updated BPF effective begin date to the
           FILE_ATTRIBUTES group

 RETURNS:  Nothing
--------------------------------------------------------------------------*/
void ias_bpf_set_file_attributes_effective_date_begin
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    const char *effective_date_begin   /* I: Beginning effective date */
)
{
    strncpy(bpf->file_attributes.effective_date_begin, effective_date_begin,
        sizeof(bpf->file_attributes.effective_date_begin));
}



/*------------------------------------------------------------------------
 NAME:         ias_bpf_set_file_attributes_effective_date_end

 PURPOSE:  Writes new/updated BPF effective end date to the
           FILE_ATTRIBUTES group

 RETURNS:  Nothing
--------------------------------------------------------------------------*/
void ias_bpf_set_file_attributes_effective_date_end
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    const char *effective_date_end     /* I: Ending effective date */
)
{
    strncpy(bpf->file_attributes.effective_date_end,
        effective_date_end, sizeof(bpf->file_attributes.effective_date_end));
}



/*------------------------------------------------------------------------
 NAME:         ias_bpf_set_file_attributes_spacecraft_name

 PURPOSE:  Writes new/updated BPF spacecraft name to the
           FILE_ATTRIBUTES group

 RETURNS:  Nothing
--------------------------------------------------------------------------*/
void ias_bpf_set_file_attributes_spacecraft_name
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    const char *bpf_spacecraft_name    /* I: Spacecraft Name */
)
{
    strncpy(bpf->file_attributes.spacecraft_name, bpf_spacecraft_name,
        sizeof(bpf->file_attributes.spacecraft_name));
}



/*------------------------------------------------------------------------
 NAME:         ias_bpf_set_file_attributes_sensor_name

 PURPOSE:  Writes new/updated BPF sensor name to the
           FILE_ATTRIBUTES group

 RETURNS:  Nothing
--------------------------------------------------------------------------*/
void ias_bpf_set_file_attributes_sensor_name
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    const char *bpf_sensor_name        /* I: Sensor name */
)
{
    strncpy(bpf->file_attributes.sensor_name, bpf_sensor_name,
        sizeof(bpf->file_attributes.sensor_name));
}



/*------------------------------------------------------------------------
 NAME:            ias_bpf_set_file_attributes_baseline_date

 PURPOSE:  Writes new/updated BPF baseline date to the FILE_ATTRIBUTES group

 RETURNS:  Nothing
--------------------------------------------------------------------------*/
void ias_bpf_set_file_attributes_baseline_date
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    const char *bpf_baseline_date      /* I: Name of BPF file to update */
)
{
    strncpy(bpf->file_attributes.baseline_date, bpf_baseline_date,
        sizeof(bpf->file_attributes.baseline_date));
}



/*------------------------------------------------------------------------
 NAME:              ias_bpf_set_file_attributes_description

 PURPOSE:  Writes new/updated BPF description to the FILE_ATTRIBUTES group

 RETURNS:  Nothing
--------------------------------------------------------------------------*/
void ias_bpf_set_file_attributes_description
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    const char *bpf_description        /* I: Name of BPF file to update */
)
{
    strncpy(bpf->file_attributes.description, bpf_description,
        sizeof(bpf->file_attributes.description));
}



/*------------------------------------------------------------------------
 NAME:              ias_bpf_set_file_attributes_version

 PURPOSE:  Writes new/updated BPF version to the FILE_ATTRIBUTES group

 RETURNS:  Nothing
--------------------------------------------------------------------------*/
void ias_bpf_set_file_attributes_version
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    int version                        /* I: Name of BPF file to update */
)
{
    bpf->file_attributes.version = version;
}



/*------------------------------------------------------------------------
 NAME:      ias_bpf_set_orbit_parameters_begin_orbit_number

 PURPOSE:  Writes new/updated beginning orbit number to the
           ORBIT_PARAMETERS group

 RETURNS:  Nothing
--------------------------------------------------------------------------*/
void ias_bpf_set_orbit_parameters_begin_orbit_number
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    int begin_orbit_number             /* I: Name of BPF file to update */
)
{
    bpf->orbit_parameters.begin_orbit_number = begin_orbit_number;
}



/*------------------------------------------------------------------------
 NAME:      ias_bpf_set_bias_model_band_number

 PURPOSE:  Sets the BPF band number

 RETURNS:  Integer status code of SUCCESS or ERROR
--------------------------------------------------------------------------*/
int ias_bpf_set_bias_model_band_number
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    int band_number                    /* I: Band number */
)
{
    int band_index;                    /* 0-based band index */


    /* Make sure the band number is a valid OLI/TIRS imaging band number. */
    if (!ias_sat_attr_band_classification_matches(band_number,
        IAS_NORMAL_BAND))
    {
        IAS_LOG_ERROR("Band number %d is not a 'normal' imaging band",
            band_number);
        return ERROR;
    }

    /* Get the 0-based band index corresponding to the band number. */
    band_index = ias_sat_attr_convert_band_number_to_index(band_number);
    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Converting band number %d to corresponding index",
            band_number);
        return ERROR;
    }


    /* Set the band number */
    bpf->bias_model[band_index].band_number = band_number;

    /* Done */
    return SUCCESS;
}



/*------------------------------------------------------------------------
 NAME:      ias_bpf_set_bias_model_spectral_type

 PURPOSE:  Writes the spectral type for the specified band number
           and allocates space for the correct type of bias model structure

 RETURNS:  SUCCESS or ERROR
--------------------------------------------------------------------------*/
int ias_bpf_set_bias_model_spectral_type
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    int band_number,                   /* I: Band number */
    IAS_SPECTRAL_TYPE spectral_type    /* I: Spectral type for the current
                                          band */
)
{
    int band_index;                    /* 0-based index into band arrays */


    /* Convert the band number to the corresponding 0-based index */
    band_index = ias_sat_attr_convert_band_number_to_index(band_number);
    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Converting band number %d to corresponding index",
            band_number);
        return ERROR;
    }

    /* Sanity check to ensure the correct band number for the current
       bias_model array element. */
    if (band_number != bpf->bias_model[band_index].band_number)
    {
        IAS_LOG_ERROR("Input band number %d does not correspond to "
            "current bias model band number %d", band_number,
            bpf->bias_model[band_index].band_number);
        return ERROR;
    }

    /* Set the spectral type */
    bpf->bias_model[band_index].spectral_type = spectral_type;

    /* Depending on the spectral type, allocate either the PAN model
       block, the VNIR model block, the SWIR model block, or the
       THERMAL model block.  */
    switch (spectral_type)
    {
        case IAS_SPECTRAL_VNIR:
        {
            bpf->bias_model[band_index].vnir =
                calloc(1, sizeof(struct IAS_BPF_VNIR_BIAS_MODEL));
            if (bpf->bias_model[band_index].vnir == NULL)
            {
                IAS_LOG_ERROR("Allocating memory for VNIR bias parameters");
                return ERROR;
            }
            break;
        }
        case IAS_SPECTRAL_SWIR:
        {
            bpf->bias_model[band_index].swir =
                calloc(1, sizeof(struct IAS_BPF_SWIR_BIAS_MODEL));
            if (bpf->bias_model[band_index].swir == NULL)
            {
                IAS_LOG_ERROR("Allocating memory for SWIR bias parameters");
                return ERROR;
            }
            break;
        }
        case IAS_SPECTRAL_PAN:
        {
            bpf->bias_model[band_index].pan =
                calloc(1, sizeof(struct IAS_BPF_PAN_BIAS_MODEL));
            if (bpf->bias_model[band_index].pan == NULL)
            {
                IAS_LOG_ERROR("Allocating memory for PAN bias parameters");
                return ERROR;
            }
            break;
        }
        case IAS_SPECTRAL_THERMAL:
        {
            bpf->bias_model[band_index].thermal =
                calloc(1, sizeof(struct IAS_BPF_THERMAL_BIAS_MODEL));
            if (bpf->bias_model[band_index].thermal == NULL)
            {
                IAS_LOG_ERROR("Allocating memory for THERMAL bias "
                    "parameters");
                return ERROR;
            }
            break;
        }
        default:
        {
            IAS_LOG_ERROR("Unable to determine type of bias parameters for "
                "band %d", bpf->bias_model[band_index].band_number);
            return ERROR;
        }
    }

    /* Done */
    return SUCCESS;
}



/*------------------------------------------------------------------------
 NAME:      ias_bpf_set_a0_coefficient

 PURPOSE:  Sets the a0 model coefficient parameter value for the
           specified band and SCA

 RETURNS:  Integer status code of SUCCESS or ERROR
--------------------------------------------------------------------------*/
int ias_bpf_set_bias_model_a0_coefficient
(
    IAS_BPF *bpf,                    /* I/O:  BPF data structure to be
                                        updated */
    int band_number,                 /* I: 1-based band number */
    int sca_index,                   /* I: 0-based SCA index */
    int oddeven_flag,                /* I: PAN band state flag */
    double a0_coeff                  /* I: A0 model coefficient value */
)
{
    int band_index = 0;              /* 0-based band index */
    int number_of_scas;


    /* Convert the band number to the corresponding 0-based index */
    band_index = ias_sat_attr_convert_band_number_to_index(band_number);
    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Converting band number %d to corresponding index",
            band_number);
        return ERROR;
    }
    if (band_number != bpf->bias_model[band_index].band_number)
    {
        IAS_LOG_ERROR("Input band number %d does not correspond to "
            "current bias model band number %d", band_number,
            bpf->bias_model[band_index].band_number);
        return ERROR;
    }

    /* Get the number of SCAs for each band and make sure the SCA index
       is in the proper range.  */
    number_of_scas = ias_sat_attr_get_scas_per_band(band_number);
    if (number_of_scas == ERROR)
    {
        IAS_LOG_ERROR("Retrieving number of SCAs for band %d", band_number);
        return ERROR;
    }
    if ((sca_index < 0) || (sca_index >= number_of_scas))
    {
        IAS_LOG_ERROR("Invalid SCA array index value %d for band number %d ",
            sca_index, band_number);
        return ERROR;
    }

    /* Set the A0_Coefficient parameter for the OLI bands.  */
    switch (bpf->bias_model[band_index].spectral_type)
    {
        case IAS_SPECTRAL_PAN:
        {
            if (oddeven_flag == IAS_BPF_PAN_EVEN)
            {
                bpf->bias_model[band_index]
                    .pan->a0_coefficient_even[sca_index] = a0_coeff;
            }
            else
            {
                bpf->bias_model[band_index]
                    .pan->a0_coefficient_odd[sca_index] = a0_coeff;
            }
            break;
        }
        case IAS_SPECTRAL_SWIR:
        {
            bpf->bias_model[band_index].swir->a0_coefficient[sca_index] =
               a0_coeff;
            break;
        }
        case IAS_SPECTRAL_VNIR:
        {
            bpf->bias_model[band_index].vnir->a0_coefficient[sca_index] =
                a0_coeff;
            break;
        }
        default:
        {
            IAS_LOG_ERROR("Invalid spectral type %d for band number %d",
                bpf->bias_model[band_index].spectral_type, band_number);
            return ERROR;
        }
    }

    /* Done */
    return SUCCESS;
}



/*------------------------------------------------------------------------
 NAME:      ias_bpf_set_bias_model_detector_coefficients

 PURPOSE:  Sets detector-specific parameter values for the specified
           OLI / TIRS band and SCA

 RETURNS:  Integer status code of SUCCESS or ERROR
--------------------------------------------------------------------------*/
int ias_bpf_set_bias_model_detector_coefficients
(
    IAS_BPF *bpf,                      /* I/O: BPF data structure
                                          to update */
    int band_number,                   /* I: Band number */
    int sca_index,                     /* I: 0-based SCA index */
    int det_index,                     /* I: 0-based detector index */
    int oddeven_flag,                  /* I: PAN band state flag */
    double *params,                    /* I: Array of detector-specific
                                          parameters to set */
    int num_params                     /* I: Number of parameters to set */
)
{
    int band_index = 0;                /* 0-based index into band arrays */
    int number_of_scas;                /* Number of SCAs per band */
    int number_of_detectors;           /* Number of detectors per SCA */
    int param_count = 0;               /* Parameter loop counter */


    /* Convert the band number to the corresponding 0-based index */
    band_index = ias_sat_attr_convert_band_number_to_index(band_number);
    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Converting band number %d to corresponding index",
                      band_number);
        return ERROR;
    }
    if (band_number != bpf->bias_model[band_index].band_number)
    {
        IAS_LOG_ERROR("Input band number %d does not correspond to "
            "current bias_model band number %d", band_number,
            bpf->bias_model[band_index].band_number);
        return ERROR;
    }

    /* Get the number of SCAs and number of detectors per SCA for each
       band, then determine whether the SCA index and detector index
       are within their proper ranges.  */
    number_of_scas = ias_sat_attr_get_scas_per_band(band_number);
    number_of_detectors = ias_sat_attr_get_detectors_per_sca(band_number);
    if ((number_of_scas == ERROR) || (number_of_detectors == ERROR))
    {
        IAS_LOG_ERROR("Retrieving number of SCAs and/or number of "
            "detectors per SCA for band %d", band_number);
        return ERROR;
    }
    if ((sca_index < 0) || (sca_index >= number_of_scas)
        || (det_index < 0) || (det_index >= number_of_detectors))
    {
        IAS_LOG_ERROR("Invalid SCA and/or detector array index values %d, "
            "%d for band %d", sca_index, det_index, band_number);
        return ERROR;
    }

    /* Ensure the number of parameters to set is at least 1. */
    if (num_params < 1)
    {
        IAS_LOG_ERROR("Must be at least one parameter value to set");
        return ERROR;
    }

    /* Set each detector-specific parameter in its proper order */
    if (bpf->bias_model[band_index].spectral_type == IAS_SPECTRAL_PAN)
    {
        if (oddeven_flag == IAS_BPF_PAN_EVEN)
        {
            for (param_count = 0; param_count < num_params; param_count++)
            {
                bpf->bias_model[band_index]
                    .pan->det_params_even[sca_index][det_index][param_count]
                        = params[param_count];
            }
        }
        else
        {
            for (param_count = 0; param_count < num_params; param_count++)
            {
                bpf->bias_model[band_index]
                    .pan->det_params_odd[sca_index][det_index][param_count]
                        = params[param_count];
            }
        }
    }
    else if (bpf->bias_model[band_index].spectral_type == IAS_SPECTRAL_SWIR)
    {
        for (param_count = 0; param_count < num_params; param_count++)
        {
            bpf->bias_model[band_index]
                .swir->det_params[sca_index][det_index][param_count] =
                     params[param_count];
        }
    }
    else if (bpf->bias_model[band_index].spectral_type == IAS_SPECTRAL_VNIR)
    {
        for (param_count = 0; param_count < num_params; param_count++)
        {
            bpf->bias_model[band_index]
                .vnir->det_params[sca_index][det_index][param_count] =
                    params[param_count];
        }
    }
    else if (bpf->bias_model[band_index].spectral_type
        == IAS_SPECTRAL_THERMAL)
    {
        for (param_count = 0; param_count < num_params; param_count++)
        {
            bpf->bias_model[band_index]
                .thermal->det_params[sca_index][det_index][param_count] =
                params[param_count];
        }
    }

    /* Done */
    return SUCCESS;
}



/**************************************************************************
 NAME:     ias_bpf_get_sensor_id

 PURPOSE:  Returns the sensor ID number based on the sensor name in the
           BPF file attributes information.

 RETURNS:  Sensor ID number if successful; IAS_INVALID_SENSOR_ID value if
           there's an error

 NOTES:    This routine is currently shared between the BPF File IO and
           BPF database libraries.  It should also be callable from any
           any application as well.
***************************************************************************/
IAS_SENSOR_ID ias_bpf_get_sensor_id
(
    IAS_BPF *bpf                      /* I: BPF data structure */
)
{
    const struct IAS_BPF_FILE_ATTRIBUTES *file_attributes = NULL;
    int is_oli_bpf = 0;
    int is_tirs_bpf = 0;

    /* Get the file attributes. For this routine to work, this substructure
       must be populated.  */
    file_attributes = ias_bpf_get_file_attributes(bpf);
    if (file_attributes == NULL)
    {
        IAS_LOG_ERROR("Retrieving BPF file attributes");
        return IAS_INVALID_SENSOR_ID;
    }

    /* Check the sensor name to determine whether we're dealing with
       an OLI or TIRS BPF.  At this point, a valid sensor name must be in
       the file attributes information -- if it isn't, the sensor ID number
       can't be determined, and that's an error.

       For OLI, "valid" sensor names are:
       - "Operational Land Imager"
       - "OLI"

       For TIRS, "valid sensor names are:
       - "Thermal Infrared Sensor"
       - "TIRS"

       These names are case-sensitive.  */
    is_oli_bpf = ((strcmp(file_attributes->sensor_name, "OLI") == 0)
        || (strcmp(file_attributes->sensor_name,
            "Operational Land Imager") == 0));
    is_tirs_bpf = ((strcmp(file_attributes->sensor_name, "TIRS") == 0)
        || (strcmp(file_attributes->sensor_name,
            "Thermal Infrared Sensor") == 0));

    /* The BPF is either an OLI BPF or a TIRS BPF.  If neither of the flags
       are true, the sensor name is invalid.  */
    if (!(is_oli_bpf || is_tirs_bpf))
    {
        IAS_LOG_ERROR("Invalid sensor name '%s', cannot determine sensor "
            "ID", bpf->file_attributes.sensor_name);
        return IAS_INVALID_SENSOR_ID;
    }

    if (is_oli_bpf)
        return IAS_OLI;
    else
        return IAS_TIRS;
}
