/*************************************************************************
NAME: ias_cpf_parse_nonuniformity

PURPOSE: Retrieve the oli and tirs nonuniformity data group from the 
         odl tree and insert into the internal cpf structure.
        
NOTE:    Pointers are initialized to NULL in ias_cpf_read

RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 2

int ias_cpf_parse_nonuniformity
(
    const IAS_CPF *cpf,                         /* I: CPF structure */
    int sensor,                                 /* I: sensor tirs or oli */
    const char *group_name,                     /* I: group to retrieve */
    struct IAS_CPF_NONUNIFORMITY *nonuniformity /* O: CPF det offsets data */
)
{
    int nbands;                    /* total number bands */
    int band_index;                /* band loop var */
    int band_number;               /* Actual band number */
    int normal_band_index;         /* normal band number converted to index */
    int band_list[IAS_MAX_NBANDS]; /* list of band numbers */
    int status;                    /* Function return value */
    int nscas;                     /* total number scas */
    int sca_index;                 /* SCA loop var */
    int count = 0;                 /* number of list buckets */
    int ndets;                     /* band detector count */

    IAS_OBJ_DESC *odl_tree;        /* ODL tree */

    /* get oli band info */
    status = ias_sat_attr_get_sensor_band_numbers(sensor, IAS_NORMAL_BAND, 
                                                  0, band_list, 
                                                  IAS_MAX_NBANDS, &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting sat band attributes for sensor id: %d", sensor);
        return ERROR;
    }

    /* get sca count */
    nscas = ias_sat_attr_get_sensor_sca_count(sensor);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sat sca count for sensor id: %d", sensor);
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[nbands * nscas * NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * nscas * NUMBER_ATTRIBUTES];

    /* Loop through the bands */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];
        
        /* get the index equivalent of the normal band number */
        normal_band_index 
            = ias_sat_attr_convert_band_number_to_index(band_number); 
        if (normal_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting the band number to an index");
            ias_cpf_free_nonuniformity_memory(nonuniformity);
            return ERROR;
        }

        /* get detector count of current band */
        ndets = ias_sat_attr_get_detectors_per_sca(band_number);
        if (ndets == ERROR)
        {
            IAS_LOG_ERROR("Getting band detector count for band: %d", 
                            band_number);
            ias_cpf_free_nonuniformity_memory(nonuniformity);
            return ERROR;
        }

        /* loop through scas */
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            if (sensor == IAS_OLI)
            {
                /* Add the band and sca information to the CPF attribute name
                   for the OLI sensor */
                status = snprintf(attribute[count], sizeof(attribute[count]),  
                    "%s_B%02d_SCA%02d", "Solar_Diffuser_Scale_Primary", 
                    band_number, sca_index + 1);
                if (status < 0 || status >= sizeof(attribute[count]))
                { 
                    IAS_LOG_ERROR("Creating Solar_Diffuser_Scale_Primary "
                                  "attribute string");
                    ias_cpf_free_nonuniformity_memory(nonuniformity);
                    return ERROR;
                }

            }
            else
            {
                /* Add the band and sca information to the CPF attribute name
                   for the TIRS sensor */
                status = snprintf(attribute[count], sizeof(attribute[count]),
                    "%s_B%02d_SCA%02d", "Blackbody_Scale",
                    band_number, sca_index + 1);
                if (status < 0 || status >= sizeof(attribute[count]))
                { 
                    IAS_LOG_ERROR("Creating Blackbody_Scale attribute string");
                    ias_cpf_free_nonuniformity_memory(nonuniformity);
                    return ERROR;
                }
            }

            /* allocate space for the primary scale factor from cpf */
            nonuniformity->scale_factor_1[normal_band_index][sca_index] 
                = malloc(ndets * sizeof(double));

            if (nonuniformity->scale_factor_1[normal_band_index][sca_index] 
                == NULL)
            {
                IAS_LOG_ERROR("Allocating memory solar diffuser "
                              "scale primary group: %s",
                              group_name);
                ias_cpf_free_nonuniformity_memory(nonuniformity);
                return ERROR;
            }

            /* populate list with nonuniformity scale factors */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr = nonuniformity
                ->scale_factor_1[normal_band_index][sca_index];
            list[count].parm_size = ndets * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndets;
            count++;

            if (sensor == IAS_OLI)
            {
                /* Add the band and sca information to the CPF attribute name */
                status = snprintf(attribute[count], sizeof(attribute[count]),  
                    "%s_B%02d_SCA%02d", "Solar_Diffuser_Scale_Pristine", 
                    band_number, sca_index + 1);
                if (status < 0 || status >= sizeof(attribute[count]))
                { 
                    IAS_LOG_ERROR("Creating Solar_Diffuser_Scale_Pristine "
                                  "attribute string");
                    ias_cpf_free_nonuniformity_memory(nonuniformity);
                    return ERROR;
                }

                /* allocate space for the pristine scale factor from cpf */
                nonuniformity->scale_factor_2[normal_band_index][sca_index] 
                    = malloc(ndets * sizeof(double));

                if (nonuniformity->scale_factor_2[normal_band_index][sca_index] 
                    == NULL)
                {
                    IAS_LOG_ERROR("Allocating memory solar diffuser "
                                  "scale pristine factors group: %s",
                                  group_name);
                    ias_cpf_free_nonuniformity_memory(nonuniformity);
                    return ERROR;
                }

                /* populate list with nonuniformity scale factors */
                list[count].group_name = group_name;
                list[count].attribute = attribute[count];
                list[count].parm_ptr = nonuniformity
                    ->scale_factor_2[normal_band_index][sca_index];
                list[count].parm_size = ndets * sizeof(double);
                list[count].parm_type = IAS_ODL_Double;
                list[count].parm_count = ndets;
                count++;
            }
         }
    }

    if (sensor == IAS_OLI)
    {
        /* make a sanity check of number of parameters to retrieve for OLI */
        if ((nbands * nscas * NUMBER_ATTRIBUTES) != count)
        {
            IAS_LOG_ERROR("Number of parameters does not match number to "
                      "retrieve for this sensor id: %d", sensor);
            ias_cpf_free_nonuniformity_memory(nonuniformity);
            return ERROR;
        }
    }
    else
    {
        /* make a sanity check of number of parameters to retrieve for TIRS */
        if ((nbands * nscas) != count)
        {
            IAS_LOG_ERROR("Number of parameters does not match number to "
                      "retrieve for this sensor id: %d", sensor);
            ias_cpf_free_nonuniformity_memory(nonuniformity);
            return ERROR;
        }
    }

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        ias_cpf_free_nonuniformity_memory(nonuniformity);
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}

void ias_cpf_free_nonuniformity_memory
(
    struct IAS_CPF_NONUNIFORMITY *nonuniformity /* cpf struct to free */
)
{
    int band_index;                 /* band loop control */
    int sca_index;                  /* sca loop control */

    for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
    {
        for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
        {
            free(nonuniformity->scale_factor_1[band_index][sca_index]);
            nonuniformity->scale_factor_1[band_index][sca_index] = NULL;
            free(nonuniformity->scale_factor_2[band_index][sca_index]);
            nonuniformity->scale_factor_2[band_index][sca_index] = NULL;
        }
    }
}
