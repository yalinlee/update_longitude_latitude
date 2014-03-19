/***************************************************************************
 NAME:      ias_rlut_build_linearization_params_table_description

 PURPOSE:   Routines implementing construction of the RLUT linearization
            parameter table

 ROUTINES:  ias_rlut_cleanup_table_description
            ias_rlut_build_linearization_params_table_description

****************************************************************************/
#include "hdf5.h"
#include "hdf5_hl.h"
#include "ias_logging.h"
#include "ias_rlut.h"
#include "ias_rlut_private.h"
#include "ias_const.h"


/*************************************************************************
 NAME:     ias_rlut_cleanup_table_description

 PURPOSE:  Public routine that closes open fields

 RETURNS:  Nothing
**************************************************************************/
void ias_rlut_cleanup_table_description
(
    hid_t *fields_to_close,             /* I: IDs to close */
    int num_fields                      /* I: Number of fields to close */
)
{
    int field_count;                 /* Loop counter */

    for (field_count = 0; field_count < num_fields; field_count++)
    {
        if (fields_to_close[field_count] >= 0)
        {
            if (H5Tclose(fields_to_close[field_count]) < 0)
                IAS_LOG_WARNING("closing HDF field %d", (field_count + 1));
        }
    }
}   /* END ias_rlut_cleanup_table_description */



/*************************************************************************
 NAME:     ias_rlut_build_linearization_params_table_description

 PURPOSE:  Builds the table description for the RLUT linearization
           parameters

 RETURNS:  Integer status code of SUCCESS or ERROR

*************************************************************************/
int ias_rlut_build_linearization_params_table_description
(
    size_t offsets[IAS_RLUT_PARAM_NFIELDS],          /* O: Offsets into data
                                                        structure */
    const char *field_names[IAS_RLUT_PARAM_NFIELDS], /* O: Field names */
    hid_t field_types[IAS_RLUT_PARAM_NFIELDS],       /* O: Field data types */
    hid_t fields_to_close[IAS_RLUT_PARAM_NFIELDS],   /* O: List of fields to
                                                        be closed */
    size_t field_sizes[IAS_RLUT_PARAM_NFIELDS]       /* O: Field sizes */
)
{
    IAS_RLUT_LINEARIZATION_PARAMS lin_params; /* Data structure reference */
    int field_count;                          /* Generic field counter */
    int return_status = SUCCESS;


    /* Initialize */
    for (field_count = 0; field_count < IAS_RLUT_PARAM_NFIELDS; field_count++)
    {
        fields_to_close[field_count] = -1;
    }

    /* Define the table, starting with the low DN cutoff threshold */
    field_count = 0;
    offsets[field_count] = HOFFSET(IAS_RLUT_LINEARIZATION_PARAMS,
        cutoff_threshold_low);
    field_names[field_count] = "Low Cutoff Threshold";
    field_types[field_count] = H5T_NATIVE_DOUBLE;
    field_sizes[field_count] = sizeof(lin_params.cutoff_threshold_low);

    /* High DN cutoff threshold */
    offsets[++field_count] = HOFFSET(IAS_RLUT_LINEARIZATION_PARAMS,
        cutoff_threshold_high);
    field_names[field_count] = "High Cutoff Threshold";
    field_types[field_count] = H5T_NATIVE_DOUBLE;
    field_sizes[field_count] = sizeof(lin_params.cutoff_threshold_high);

    /* Low range remapping coefficients */
    offsets[++field_count] = HOFFSET(IAS_RLUT_LINEARIZATION_PARAMS,
        remap_coeff0_low);
    field_names[field_count] = "Remap Coefficient 0 Low";
    field_types[field_count] = H5T_NATIVE_DOUBLE;
    field_sizes[field_count] = sizeof(lin_params.remap_coeff0_low);

    offsets[++field_count] = HOFFSET(IAS_RLUT_LINEARIZATION_PARAMS,
        remap_coeff1_low);
    field_names[field_count] = "Remap Coefficient 1 Low";
    field_types[field_count] = H5T_NATIVE_DOUBLE;
    field_sizes[field_count] = sizeof(lin_params.remap_coeff1_low);

    offsets[++field_count] = HOFFSET(IAS_RLUT_LINEARIZATION_PARAMS,
        remap_coeff2_low);
    field_names[field_count] = "Remap Coefficient 2 Low";
    field_types[field_count] = H5T_NATIVE_DOUBLE;
    field_sizes[field_count] = sizeof(lin_params.remap_coeff2_low);

    /* Mid range remapping coefficients */
    offsets[++field_count] = HOFFSET(IAS_RLUT_LINEARIZATION_PARAMS,
        remap_coeff0_mid);
    field_names[field_count] = "Remap Coefficient 0 Mid";
    field_types[field_count] = H5T_NATIVE_DOUBLE;
    field_sizes[field_count] = sizeof(lin_params.remap_coeff0_mid);

    offsets[++field_count] = HOFFSET(IAS_RLUT_LINEARIZATION_PARAMS,
        remap_coeff1_mid);
    field_names[field_count] = "Remap Coefficient 1 Mid";
    field_types[field_count] = H5T_NATIVE_DOUBLE;
    field_sizes[field_count] = sizeof(lin_params.remap_coeff1_mid);

    offsets[++field_count] = HOFFSET(IAS_RLUT_LINEARIZATION_PARAMS,
        remap_coeff2_mid);
    field_names[field_count] = "Remap Coefficient 2 Mid";
    field_types[field_count] = H5T_NATIVE_DOUBLE;
    field_sizes[field_count] = sizeof(lin_params.remap_coeff2_mid);

    /* High range remapping coefficients */
    offsets[++field_count] = HOFFSET(IAS_RLUT_LINEARIZATION_PARAMS,
        remap_coeff0_high);
    field_names[field_count] = "Remap Coefficient 0 High";
    field_types[field_count] = H5T_NATIVE_DOUBLE;
    field_sizes[field_count] = sizeof(lin_params.remap_coeff0_high);

    offsets[++field_count] = HOFFSET(IAS_RLUT_LINEARIZATION_PARAMS,
        remap_coeff1_high);
    field_names[field_count] = "Remap Coefficient 1 High";
    field_types[field_count] = H5T_NATIVE_DOUBLE;
    field_sizes[field_count] = sizeof(lin_params.remap_coeff1_high);

    offsets[++field_count] = HOFFSET(IAS_RLUT_LINEARIZATION_PARAMS,
        remap_coeff2_high);
    field_names[field_count] = "Remap Coefficient 2 High";
    field_types[field_count] = H5T_NATIVE_DOUBLE;
    field_sizes[field_count] = sizeof(lin_params.remap_coeff2_high);

    /* Increment the final field count */
    field_count++;

    /* Check for errors */
    if (field_count < IAS_RLUT_PARAM_NFIELDS)
    {
        IAS_LOG_ERROR("Too few fields found");
        ias_rlut_cleanup_table_description(fields_to_close,
            IAS_RLUT_PARAM_NFIELDS);
        return_status = ERROR;
    }
    else if (field_count > IAS_RLUT_PARAM_NFIELDS)
    {
        IAS_LOG_ERROR("Too many fields found -- stack probably corrupted");
        ias_rlut_cleanup_table_description(fields_to_close,
            IAS_RLUT_PARAM_NFIELDS);
        return_status = ERROR;
    }

    return return_status;
}   /*  END ias_rlut_build_linearization_param_table_description */
