/*************************************************************************

NAME: ias_cpf_parse_saturation_level

PURPOSE: Retrieve the saturation level data group from the odl tree and insert
         into the internal cpf structure.
         Includes OLI/TIRS normal, blind and vrp bands.
        
         NOTE: the pointers in this structure are initialized in ias_read_cpf.

RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_BAND_SCA_ATTRIBUTES 4
#define NUMBER_ATTRIBUTES 2

int ias_cpf_parse_saturation_level
(
    const IAS_CPF *cpf,          /* I: CPF structure */
    int sensor,                  /* I: sensor tirs or oli */
    const char *group_name,      /* I: group to retrieve */
    struct IAS_CPF_SATURATION_LEVEL *saturation /* O: cpf saturation data */
)
{
    int band_count;              /* total number bands */
    int band_index;              /* band loop var */
    int band_number;             /* Actual band number */
    int normal_band_index;       /* normal band number convereted to index */
    int parameter_band_number;   /* parameter attribute band number */
    int band_list[IAS_MAX_TOTAL_BANDS]; /* list of band numbers */
    int status;                  /* Function return value */
    int nscas;                   /* total number scas */
    int sca_index;               /* SCA loop var */
    int count = 0;               /* number of list buckets */
    int ndets;                   /* band detector count */
    int number_attributes;       /* calculated number of attributes */
    char band_prefix[5];         /* band parameter name prefix */
    int nbands = 0;              /* low/high radiance saturation band count */
    int band_offset = 0;         /* low/high radiance saturation band offset*/

    IAS_OBJ_DESC *odl_tree;      /* ODL tree */

    const IAS_BAND_ATTRIBUTES *band_attr; /* band attributes structure */

    /* get sca count */
    nscas = ias_sat_attr_get_sensor_sca_count(sensor);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sca count for sensor id: %d", sensor);
        return ERROR;
    }

    /* check band type to get correct band attributes */
    if (sensor == IAS_OLI)
    {
        status = ias_sat_attr_get_any_sensor_band_numbers(sensor, 
                IAS_NORMAL_BAND | IAS_BLIND_BAND | IAS_VRP_BAND, 0, band_list, 
                IAS_MAX_TOTAL_BANDS, &band_count);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Getting OLI band numbers");
            return ERROR;
        }
    }
    else if (sensor == IAS_TIRS)
    {
        status = ias_sat_attr_get_any_sensor_band_numbers(sensor,
                IAS_NORMAL_BAND | IAS_BLIND_BAND | IAS_SECONDARY_BAND, 0,
                band_list, IAS_MAX_TOTAL_BANDS, &band_count);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Getting TIRS band numbers");
            return ERROR;
        }
    }
    else
    {
        IAS_LOG_ERROR("Unknown sensor given: %d", sensor);
        return ERROR;
    }

    /* calculate number of attributes */
    number_attributes = (nscas * band_count * NUMBER_BAND_SCA_ATTRIBUTES) 
                        + NUMBER_ATTRIBUTES;
    
    /* full attribute name */
    char attribute[number_attributes][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[number_attributes];

    /* Loop thru the bands */
    for (band_index = 0; band_index < band_count; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];
        band_attr = ias_sat_attr_get_band_attributes(band_number);
        if (band_attr == NULL)
        {
            IAS_LOG_ERROR("Getting band attributes vrp band number: %d",
                            band_number);
            ias_cpf_free_saturation_level_memory(saturation);
            return ERROR;
        }

        /* see if we are processing a vrp band */
        if(ias_sat_attr_band_classification_matches(band_number, (IAS_VRP_BAND))
            || (ias_sat_attr_band_classification_matches(band_number, 
                        (IAS_VRP_BAND | IAS_BLIND_BAND))))
        {
            /* get the normal band number for the vrp parameters */
            parameter_band_number = band_attr->normal_band_number;

            strncpy(band_prefix, "VRP_", sizeof(band_prefix));
            band_prefix[sizeof(band_prefix) - 1] = '\0';
        }
        else
        {
            strcpy(band_prefix, "");
            parameter_band_number = band_number;
        }

        /* get the index equivalent of the normal band number */
        normal_band_index 
            = ias_sat_attr_convert_band_number_to_index(band_number);
        if (normal_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting band number: %d to an index", 
                            band_number);
            ias_cpf_free_saturation_level_memory(saturation);
            return ERROR;
        }

        /* get detector count for current band */
        ndets = ias_sat_attr_get_detectors_per_sca(band_number);
        if (ndets == ERROR)
        {
            IAS_LOG_ERROR("Getting OLI/TIRS detector count band number: %d",
                           band_number);
            ias_cpf_free_saturation_level_memory(saturation);
            return ERROR;
        }

        /* loop through the scas */
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                   "%s%s_B%02d_SCA%02d", band_prefix,
                   "Analog_Low_Saturation_Level", parameter_band_number, 
                    sca_index +1);
            if ((status < 0) || (status >= sizeof(attribute[count])))
            {
                IAS_LOG_ERROR("Constructing CPF parameter name, group: %s",
                               group_name);
                return ERROR;
            }

            /* allocate space for analog low level data in CPF */
            saturation->analog_low_saturation_level
                [normal_band_index][sca_index] = malloc(ndets * sizeof(int));

            if (saturation->analog_low_saturation_level
                    [normal_band_index][sca_index] == NULL)
            { 
                IAS_LOG_ERROR("Allocating memory analog low "
                              "level saturation parameter: %s", group_name);
                ias_cpf_free_saturation_level_memory(saturation);
                return ERROR;
            }

            /* populate the attribute list */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr = saturation
                 ->analog_low_saturation_level[normal_band_index][sca_index];
            list[count].parm_size = ndets * sizeof(int);
            list[count].parm_type = IAS_ODL_Int;
            list[count].parm_count = ndets;
            count++;

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]),
                   "%s%s_B%02d_SCA%02d", band_prefix,
                   "Analog_High_Saturation_Level", parameter_band_number, 
                    sca_index +1);
            if ((status < 0) || (status >= sizeof(attribute[count])))
            {
                IAS_LOG_ERROR("Constructing CPF parameter name, group: %s",
                               group_name);
                return ERROR;
            }

            /* allocate space for analog high level saturation in CPF */
            saturation->analog_high_saturation_level
                [normal_band_index][sca_index] = malloc(ndets * sizeof(int));

            if (saturation->analog_high_saturation_level
                    [normal_band_index][sca_index] == NULL)
            { 
                IAS_LOG_ERROR("Allocating memory analog high level "
                              "saturation parameter: %s", group_name);
                ias_cpf_free_saturation_level_memory(saturation);
                return ERROR;
            }
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr = saturation
                  ->analog_high_saturation_level[normal_band_index][sca_index];
            list[count].parm_size = ndets * sizeof(int);
            list[count].parm_type = IAS_ODL_Int;
            list[count].parm_count = ndets;
            count++;

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]),
                    "%s%s_B%02d_SCA%02d", band_prefix,
                    "Digital_Low_Saturation_Level", parameter_band_number, 
                     sca_index +1);
            if ((status < 0) || (status >= sizeof(attribute[count])))
            {
                IAS_LOG_ERROR("Constructing CPF parameter name, group: %s",
                               group_name);
                return ERROR;
            }

            /* allocate space for Digital low level saturation in CPF */
            saturation->digital_low_saturation_level
                [normal_band_index][sca_index] = malloc(ndets * sizeof(int));

            if (saturation->digital_low_saturation_level
                    [normal_band_index][sca_index] == NULL)
            { 
                IAS_LOG_ERROR("Allocating memory digital low "
                              "level saturation parameter: %s", group_name);
                ias_cpf_free_saturation_level_memory(saturation);
                return ERROR;
            }
            /* populate list with digital low level saturation info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr = saturation
                  ->digital_low_saturation_level[normal_band_index][sca_index];
            list[count].parm_size = ndets * sizeof(int);
            list[count].parm_type = IAS_ODL_Int;
            list[count].parm_count = ndets;
            count++;

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]),
                    "%s%s_B%02d_SCA%02d", band_prefix,
                    "Digital_High_Saturation_Level", parameter_band_number, 
                     sca_index +1);
            if ((status < 0) || (status >= sizeof(attribute[count])))
            {
                IAS_LOG_ERROR("Constructing CPF parameter name, group: %s",
                               group_name);
                return ERROR;
            }

            /* allocate space for digital high level saturation in CPF */
            saturation->digital_high_saturation_level
                [normal_band_index][sca_index] = malloc(ndets * sizeof(int));

            if (saturation->digital_high_saturation_level
                    [normal_band_index][sca_index] == NULL)
            {
                IAS_LOG_ERROR("Allocating memory digital high "
                              "saturation level parameter: %s", group_name);
                ias_cpf_free_saturation_level_memory(saturation);
                return ERROR;
            }

            /* populate list with digital high level saturation info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr = saturation
                 ->digital_high_saturation_level[normal_band_index][sca_index];
            list[count].parm_size = ndets * sizeof(int);
            list[count].parm_type = IAS_ODL_Int;
            list[count].parm_count = ndets;
            count++;
        }
    }

    /* get the band count to retrieve the low/high radiance 
       saturation elements*/
    if (sensor == IAS_OLI)
    {
        nbands = IAS_OLI_MAX_NBANDS;
        band_offset = 0;
    }
    else if (sensor == IAS_TIRS)
    {
        nbands = IAS_TIRS_MAX_NBANDS;

        /* get the index of the first band to set as the offset as to where 
           to start inserting the tirs data into the arrays */
        band_number = band_list[0];
        band_offset = ias_sat_attr_convert_band_number_to_index(band_number);
        if (band_offset == ERROR)
        {
            IAS_LOG_ERROR("Converting band number: %d to index", 
                            band_number);
            ias_cpf_free_saturation_level_memory(saturation);
            return ERROR;
        }
    }
    else
    {
        IAS_LOG_ERROR("Unknown sensor given: %d", sensor);
        return ERROR;
    }

    /* build the attribute list */
    list[count].group_name = group_name;
    list[count].attribute = "Low_Radiance_Saturation";
    list[count].parm_ptr = &saturation->low_radiance_saturation[band_offset];
    list[count].parm_size = nbands * sizeof(double);
    list[count].parm_type = IAS_ODL_Double;
    list[count].parm_count = nbands;
    count++;
   
    list[count].group_name = group_name;
    list[count].attribute = "High_Radiance_Saturation";
    list[count].parm_ptr = &saturation->high_radiance_saturation[band_offset];
    list[count].parm_size = nbands * sizeof(double);
    list[count].parm_type = IAS_ODL_Double;
    list[count].parm_count = nbands;
    count++;

    /* make a sanity check of number of parameters to retrieve */
    if (number_attributes != count)
    {
        IAS_LOG_ERROR("Number of parameters does not match number to retrieve");
        return ERROR;
    }

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        ias_cpf_free_saturation_level_memory(saturation);
        return ERROR;
    }

    /* free memory */
    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}

void ias_cpf_free_saturation_level_memory
(
    struct IAS_CPF_SATURATION_LEVEL *saturation /* cpf saturation struct */
)
{
    int band_index;                         /* band loop control */
    int sca_index;                          /* sca loop control */

    for (band_index = 0; band_index < IAS_MAX_TOTAL_BANDS; band_index++)
    {
        for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
        {
            free(saturation
             ->analog_low_saturation_level[band_index][sca_index]);
            saturation
             ->analog_low_saturation_level[band_index][sca_index] = NULL;
            free(saturation
             ->analog_high_saturation_level[band_index][sca_index]);
            saturation
             ->analog_high_saturation_level[band_index][sca_index] = NULL;
            free(saturation
             ->digital_low_saturation_level[band_index][sca_index]); 
            saturation
             ->digital_low_saturation_level[band_index][sca_index] = NULL;
            free(saturation
             ->digital_high_saturation_level[band_index][sca_index]); 
            saturation
             ->digital_high_saturation_level[band_index][sca_index] = NULL;
        }
    }
}
