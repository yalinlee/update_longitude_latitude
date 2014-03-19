/***************************************************************************
 NAME:                ias_bpf_parse_bias_model

 PURPOSE:  Retrieve the bias model group data from the ODL tree and
           inserts it into the internal BPF data structure

 RETURNS:  Integer status code of SUCCESS or ERROR

 NOTES:    It is STRONGLY RECOMMENDED that this function be called only
           once (i.e. EITHER to parse an OLI BPF OR parse a TIRS BPF).
           If someone tries to parse bias model parameters for BOTH OLI
           and TIRS into the same BPF data structure... "caveat emptor"
           (in other words, let the user beware).
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ias_bpf.h"
#include "bpf_local_defines.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "ias_satellite_attributes.h"

#define IAS_BPF_MAX_GROUPNAME_LENGTH 50
#define IAS_BPF_ATTRIBUTE_STRLEN 50


static void initialize_list_attribute_info
(
    ODL_LIST_TYPE *list_attribute,   /* I/O: Individual list attribute */
    char *group_name,                /* I: Attribute group name */
    char *attribute,                 /* I: Attribute string */
    int param_count                  /* I: Number of parameters */
)
{
    /* Initialize the common attribute information */
    list_attribute->group_name = group_name;
    list_attribute->attribute = attribute;
    list_attribute->parm_count = param_count;
    list_attribute->parm_type = IAS_ODL_Double;
    list_attribute->parm_size  = param_count * sizeof(double);
}


/*************************************************************************
 NAME:     free_bias_model_memory

 PURPOSE:  Frees all bias model pointers allocated when a parsing
           error has been detected.

 RETURNS:  Nothing
**************************************************************************/
static void free_bias_model_memory
(
    struct IAS_BPF_BIAS_MODEL *bias_model     /* I: Bias model data
                                                 structure */
)
{
    int band_index;    /* band loop counter */

    /* This will attempt to free the internal pointers for ALL bands,
       whether allocated or not.  If the internal pointers for a band
       have not been allocated, nothing happens--passing a NULL pointer
       into free() does nothing.  Once the pointers are freed, (re)set
       them to NULL.  */
    for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
    {
        free(bias_model[band_index].vnir);
        free(bias_model[band_index].swir);
        free(bias_model[band_index].pan);
        free(bias_model[band_index].thermal);

        bias_model[band_index].vnir = NULL;
        bias_model[band_index].swir = NULL;
        bias_model[band_index].pan  = NULL;
        bias_model[band_index].thermal = NULL;
    }
}


int ias_bpf_parse_bias_model
(
    IAS_BPF *bpf,                            /* I: BPF data structure */
    struct IAS_BPF_BIAS_MODEL *bias_model    /* O: Populated bias_model
                                                data structure */
)
{
    IAS_SENSOR_ID sensor_id;

    int number_of_bands;             /* Number of bands */
    int number_of_scas;              /* Number of SCAs in each band */
    int number_of_detectors;         /* Number of detectors in an SCA */
    int index;                       /* Band loop counter */
    int band_index;                  /* Index to current band */
    int sca_index;                   /* SCA loop counter */
    int det_index;                   /* Detector loop counter */
    int band_number;                 /* Actual band number */
    int sca_number;                  /* Actual SCA number */
    int band_list[IAS_MAX_NBANDS];   /* band list */
    int status;                      /* Function return status */
    int number_of_attributes = 0;    /* Total number of ODL attributes to
                                        process */
    int attribute_count = 0;         /* Attribute counter */

    char group_name[IAS_BPF_ATTRIBUTE_STRLEN];
                                   /* Group name */

    char **attribute = NULL;       /* Attribute string array */

    IAS_OBJ_DESC *odl_tree = NULL; /* ODL "tree" containing BIAS_MODEL
                                      information */

    ODL_LIST_TYPE *list = NULL;    /* ODL parameter list */


    /* Get the correct sensor ID number from the sensor name in the BPF file
       attributes. */
    sensor_id = ias_bpf_get_sensor_id(bpf);
    if (sensor_id == IAS_INVALID_SENSOR_ID)
    {
        IAS_LOG_ERROR("Retrieving valid sensor ID number from BPF "
            "sensor name");
        return ERROR;
    }

    /* Get the band list for the specified sensor. */
    status = ias_sat_attr_get_sensor_band_numbers(sensor_id,
        IAS_NORMAL_BAND, 0, band_list, IAS_MAX_NBANDS, &number_of_bands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Obtaining sensor-specific band list information");
        return ERROR;
    }

    /* Get the number of SCAs for each band in the specified sensor. */
    number_of_scas = ias_sat_attr_get_sensor_sca_count(sensor_id);
    if (number_of_scas == ERROR)
    {
        IAS_LOG_ERROR("Obtaining sensor-specific SCA count");
        return ERROR;
    }

    /* Get the maximum number of detectors for the specified sensor.
       This will equal the number of detector-specific attributes to be
       processed. */
    number_of_attributes = 
        ias_sat_attr_get_sensor_max_normal_detectors(sensor_id);
    if (number_of_attributes == ERROR)
    {
        IAS_LOG_ERROR("Invalid max. number of attributes %d",
            number_of_attributes);
        return ERROR;
    }

    /* If we're parsing an OLI BPF, we'll need an extra attribute for the
       a0_coefficient parameter.  In the BPF, these are written as one value
       for each OLI band/SCA combination */
    if (sensor_id == IAS_OLI)
        number_of_attributes++;

    /* Allocate ODL list and attribute string buffers large enough to
       process all the data. */
    list = malloc(number_of_attributes * sizeof(ODL_LIST_TYPE));
    if (list == NULL)
    {
        IAS_LOG_ERROR("Allocating ODL list data buffer");
        return ERROR;
    }

    attribute = (char **)ias_misc_allocate_2d_array(number_of_attributes,
        IAS_BPF_ATTRIBUTE_STRLEN, sizeof(char));
    if (attribute == (char **)NULL)
    {
        IAS_LOG_ERROR("Allocating attribute string data buffer");
        free(list);
        return ERROR;
    }

    /* Initialize the bias model array to a 'clean' state.  All internal
       pointers will be (re)set to NULL.  */
    memset(bias_model, 0, sizeof(*bias_model));

/*-------------------------------------------------------------------------
                   BIAS_MODEL PROCESSING
---------------------------------------------------------------------------*/
    /* Loop through the bands for the specific sensor. */
    for (index = 0; index < number_of_bands; index++)
    {
        /* Save the band number and get its corresponding index. */;
        band_number = band_list[index];
        band_index = ias_sat_attr_convert_band_number_to_index(band_number);
        if (band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting band number %d to corresponding "
                "index value", band_number);
            free(list);

            /* Clean up memory allocated for previous bands. */
            ias_misc_free_2d_array((void **)attribute);
            free_bias_model_memory(bias_model);
            return ERROR;
        }

        /* Populate the current band's bias model information, starting with
           the band number information. */
        bias_model[band_index].band_number = band_number;

        /* Get the spectral type for the current band. */
        bias_model[band_index].spectral_type =
            ias_sat_attr_get_spectral_type_from_band_number(band_number);

        if (bias_model[band_index].spectral_type == IAS_UNKNOWN_SPECTRAL_TYPE)
        {
            IAS_LOG_ERROR("Retrieving spectral type for band number %d",
                band_number);
            free(list);
            ias_misc_free_2d_array((void **)attribute);

            /* Clean up the memory allocated for previous bands */
            free_bias_model_memory(bias_model);
            return ERROR;
        }

        /* Get the number of detectors in an SCA for the current band */
        number_of_detectors = ias_sat_attr_get_detectors_per_sca(band_number);
        if (number_of_detectors == ERROR)
        {
            IAS_LOG_ERROR("Retrieving number of SCA detectors for band "
                "number %d", band_number);
            free(list);
            ias_misc_free_2d_array((void **)attribute);
            free_bias_model_memory(bias_model);
            return ERROR;
        }

        /* Process each band according to its spectral type. */
        if (bias_model[band_index].spectral_type == IAS_SPECTRAL_VNIR)
        {
            /* Allocate the VNIR data block */
            bias_model[band_index].vnir =
                calloc(1, sizeof(struct IAS_BPF_VNIR_BIAS_MODEL));

            if (bias_model[band_index].vnir == NULL)
            {
                IAS_LOG_ERROR("Allocating VNIR band bias model "
                    "data block for band number %d", band_number);
                ias_misc_free_2d_array((void **)attribute);
                free(list);
                free_bias_model_memory(bias_model);
                return ERROR;
            }

            /* Process all SCAs in the band */
            for (sca_index = 0; sca_index < number_of_scas; sca_index++)
            {
                sca_number = sca_index + 1;

                /* Clear the ODL list buffer */
                memset(list, 0, sizeof(ODL_LIST_TYPE));

                sprintf(group_name, "BIAS_MODEL_B%02d_SCA%02d",
                    band_number, sca_number);

                /* (Re)set the attribute count for this SCA */
                attribute_count = 0;

                /* Set up the model parameter array for each detector */
                for (det_index = 0; det_index < number_of_detectors;
                     det_index++)
                {
                    sprintf(attribute[attribute_count], "D%03d",
                        (det_index + 1));

                    initialize_list_attribute_info(&list[attribute_count],
                        group_name, attribute[attribute_count],
                        IAS_BPF_VNIR_VALUES);

                    list[attribute_count].parm_ptr = &(bias_model[band_index]
                        .vnir->det_params[sca_index][det_index][0]);

                    attribute_count++;
                }

                /* Set up the VRP model coefficient parameter */
                sprintf(attribute[attribute_count], "A0_Coefficient");

                initialize_list_attribute_info(&list[attribute_count],
                    group_name, attribute[attribute_count], 1);

                list[attribute_count].parm_ptr = &(bias_model[band_index]
                    .vnir->a0_coefficient[sca_index]);

                attribute_count++;

                /* Pull the group data from the cache and put it in the
                   ODL tree */
                GET_BPF_GROUP_FROM_CACHE(bpf, group_name, odl_tree);

                /* Populate the list from the ODL tree */
                status = ias_odl_get_field_list(odl_tree, list,
                    attribute_count);

                if (status != SUCCESS)
                {
                    IAS_LOG_ERROR("Cannot retrieve BIAS_MODEL_B%02d_"
                        "SCA%02d data", band_number, sca_number);
                    DROP_BPF_ODL_TREE(odl_tree);
                    free(list);
                    ias_misc_free_2d_array((void **)attribute);
                    free_bias_model_memory(bias_model);
                    return ERROR;
                }

                /* Free the ODL tree memory */
                DROP_BPF_ODL_TREE(odl_tree);
            }
        }
        else if (bias_model[band_index].spectral_type == IAS_SPECTRAL_SWIR)
        {
            /* Allocate the SWIR band data block */
            bias_model[band_index].swir =
                calloc(1, sizeof(struct IAS_BPF_SWIR_BIAS_MODEL));

            if (bias_model[band_index].swir == NULL)
            {
                IAS_LOG_ERROR("Allocating SWIR band bias model "
                              "data block for band number %d", band_number);
                ias_misc_free_2d_array((void **)attribute);
                free(list);
                free_bias_model_memory(bias_model);
                return ERROR;
            }

            /* Process all SCAs in the band */
            for (sca_index = 0; sca_index < number_of_scas; sca_index++)
            {
                sca_number = sca_index + 1;

                /* Clear the ODL list buffer */
                memset(list, 0, sizeof(ODL_LIST_TYPE));

                sprintf(group_name, "BIAS_MODEL_B%02d_SCA%02d",
                    band_number, sca_number);

                /* (Re)set the attribute count for this SCA */
                attribute_count = 0;

                /* Set up the model parameter array for each detector */
                for (det_index = 0; det_index < number_of_detectors;
                     det_index++)
                {
                    sprintf(attribute[attribute_count], "D%03d",
                        (det_index + 1));

                    initialize_list_attribute_info(&list[attribute_count],
                        group_name, attribute[attribute_count],
                        IAS_BPF_SWIR_VALUES);

                    list[attribute_count].parm_ptr = &(bias_model[band_index]
                        .swir->det_params[sca_index][det_index][0]);

                    attribute_count++;
                }

                /* Set up the VRP model coefficient parameter */
                sprintf(attribute[attribute_count], "A0_Coefficient");

                initialize_list_attribute_info(&list[attribute_count],
                    group_name, attribute[attribute_count], 1);

                list[attribute_count].parm_ptr = &(bias_model[band_index]
                    .swir->a0_coefficient[sca_index]);

                attribute_count++;

                /* Pull the group data from the cache and put it in the
                   ODL tree */
                GET_BPF_GROUP_FROM_CACHE(bpf, group_name, odl_tree);

                /* Populate the list from the ODL tree */
                status = ias_odl_get_field_list(odl_tree, list,
                    attribute_count);

                if (status != SUCCESS)
                {
                    IAS_LOG_ERROR("Cannot retrieve BIAS_MODEL_B%02d_"
                        "SCA%02d data", band_number, sca_number);
                    DROP_BPF_ODL_TREE(odl_tree);
                    free(list);
                    ias_misc_free_2d_array((void **)attribute);
                    free_bias_model_memory(bias_model);
                    return ERROR;
                }

                /* Free the ODL tree memory */
                DROP_BPF_ODL_TREE(odl_tree);
            }
        }
        else if (bias_model[band_index].spectral_type == IAS_SPECTRAL_PAN)
        {
            /* Allocate the PAN band data block */
            bias_model[band_index].pan =
                calloc(1, sizeof(struct IAS_BPF_PAN_BIAS_MODEL));

            if (bias_model[band_index].pan == NULL)
            {
                IAS_LOG_ERROR("Allocating PAN band bias model "
                              "data block in band %d", band_number);
                ias_misc_free_2d_array((void **)attribute);
                free(list);
                free_bias_model_memory(bias_model);
                return ERROR;
            }

            /* Process each SCA in the band */
            for (sca_index = 0; sca_index < number_of_scas; sca_index++)
            {
                sca_number = (sca_index + 1);

                /* Clear the ODL list buffer */
                memset(list, 0, sizeof(ODL_LIST_TYPE));

                /* Do the "odd" group first */
                sprintf(group_name, "BIAS_MODEL_ODD_B08_SCA%02d",
                    sca_number);

                /* (Re)set the attribute count to 0 */
                attribute_count = 0;

                /* Set up the model parameter array for each detector */
                for (det_index = 0; det_index < number_of_detectors;
                     det_index++)
                {
                    /* Build the odd parameter name */
                    sprintf(attribute[attribute_count], "D%03d",
                        (det_index + 1));

                    initialize_list_attribute_info(&list[attribute_count],
                        group_name, attribute[attribute_count],
                        IAS_BPF_PAN_VALUES);

                    list[attribute_count].parm_ptr = &(bias_model[band_index]
                        .pan->det_params_odd[sca_index][det_index][0]);

                    attribute_count++;
                }

                /* Set up the VRP model coefficient parameter */
                sprintf(attribute[attribute_count], "A0_Coefficient");

                initialize_list_attribute_info(&list[attribute_count],
                    group_name, attribute[attribute_count], 1);

                list[attribute_count].parm_ptr = &(bias_model[band_index]
                    .pan->a0_coefficient_odd[sca_index]);

                attribute_count++;

                /* Pull the group data from the cache and put it
                   in the ODL tree */
                GET_BPF_GROUP_FROM_CACHE(bpf, group_name, odl_tree);

                /* Populate the list from the ODL tree */
                status = ias_odl_get_field_list(odl_tree, list,
                    attribute_count);
                if (status != SUCCESS)
                {
                    IAS_LOG_ERROR("Cannot retrieve BIAS_MODEL_ODD_B08_"
                        "SCA%02d data", sca_number);
                    DROP_BPF_ODL_TREE(odl_tree);
                    free(list);
                    ias_misc_free_2d_array((void **)attribute);
                    free_bias_model_memory(bias_model);
                    return ERROR;
                }

                /* Free the ODL tree memory */
                DROP_BPF_ODL_TREE(odl_tree);

                /* Clear the ODL list buffer */
                memset(list, 0, sizeof(ODL_LIST_TYPE));

                /* Now the even group */
                sprintf(group_name, "BIAS_MODEL_EVEN_B08_SCA%02d",
                    sca_number);

                /* (Re)set the attribute count to 0 */
                attribute_count = 0;

                /* Set up the model parameter array for each detector */
                for (det_index = 0; det_index < number_of_detectors;
                     det_index++)
                {
                    /* Build the even group name */
                    sprintf(attribute[attribute_count], "D%03d",
                        (det_index + 1));

                    initialize_list_attribute_info(&list[attribute_count],
                        group_name, attribute[attribute_count],
                        IAS_BPF_PAN_VALUES);

                    list[attribute_count].parm_ptr = &(bias_model[band_index]
                        .pan->det_params_even[sca_index][det_index][0]);

                    attribute_count++;
                }

                /* Set up the VRP model coefficient parameter */
                sprintf(attribute[attribute_count], "A0_Coefficient");

                initialize_list_attribute_info(&list[attribute_count],
                    group_name, attribute[attribute_count], 1);

                list[attribute_count].parm_ptr = &(bias_model[band_index]
                    .pan->a0_coefficient_even[sca_index]);

                attribute_count++;

                /* Pull the group data from the cache and put it
                   in the ODL tree */
                GET_BPF_GROUP_FROM_CACHE(bpf, group_name, odl_tree);

                /* Populate the list from the ODL tree */
                status = ias_odl_get_field_list(odl_tree, list,
                        attribute_count);
                if (status != SUCCESS)
                {
                    IAS_LOG_ERROR("Cannot retrieve BIAS_MODEL_EVEN_B08_"
                        "SCA%02d data", sca_number);
                    DROP_BPF_ODL_TREE(odl_tree);
                    free(list);
                    ias_misc_free_2d_array((void **)attribute);
                    free_bias_model_memory(bias_model);
                    return ERROR;
                }

                /* Free the ODL tree memory */
                DROP_BPF_ODL_TREE(odl_tree);
            }
        }
        else if (bias_model[band_index].spectral_type
            == IAS_SPECTRAL_THERMAL)
        {
            /* Allocate the THERMAL band data block */
            bias_model[band_index].thermal =
                calloc(1, sizeof(struct IAS_BPF_THERMAL_BIAS_MODEL));

            if (bias_model[band_index].thermal == NULL)
            {
                IAS_LOG_ERROR("Allocating THERMAL band bias model "
                              "data block for band number %d", band_number);
                ias_misc_free_2d_array((void **)attribute);
                free(list);
                free_bias_model_memory(bias_model);
                return ERROR;
            }

            /* Process all SCAs in the band */
            for (sca_index = 0; sca_index < number_of_scas; sca_index++)
            {
                sca_number = sca_index + 1;

                /* Clear the ODL list buffer */
                memset(list, 0, sizeof(ODL_LIST_TYPE));

                sprintf(group_name, "BIAS_MODEL_B%02d_SCA%02d",
                    band_number, sca_number);

                /* (Re)set the attribute count for this SCA */
                attribute_count = 0;

                /* Set up the model parameter array for each detector */
                for (det_index = 0; det_index < number_of_detectors;
                     det_index++)
                {
                    sprintf(attribute[attribute_count], "D%03d",
                        (det_index + 1));

                    initialize_list_attribute_info(&list[attribute_count],
                        group_name, attribute[attribute_count],
                        IAS_BPF_THERMAL_VALUES);

                    list[attribute_count].parm_ptr = &(bias_model[band_index]
                        .thermal->det_params[sca_index][det_index][0]);

                    attribute_count++;
                }

                /* Pull the group data from the cache and put it in the
                   ODL tree */
                GET_BPF_GROUP_FROM_CACHE(bpf, group_name, odl_tree);

                /* Populate the list from the ODL tree */
                status = ias_odl_get_field_list(odl_tree, list,
                    attribute_count);
                if (status != SUCCESS)
                {
                    IAS_LOG_ERROR("Cannot retrieve BIAS_MODEL_B%02d_"
                        "SCA%02d data", band_number, sca_number);
                    DROP_BPF_ODL_TREE(odl_tree);
                    free(list);
                    ias_misc_free_2d_array((void **)attribute);
                    free_bias_model_memory(bias_model);
                    return ERROR;
                }

                /* Free the ODL tree memory */
                DROP_BPF_ODL_TREE(odl_tree);
            }
        }
    }

    /* Done */
    if (ias_misc_free_2d_array((void **)attribute) != SUCCESS)
        IAS_LOG_WARNING("Freeing attribute string data buffer");
    free(list);

    return SUCCESS;
}
