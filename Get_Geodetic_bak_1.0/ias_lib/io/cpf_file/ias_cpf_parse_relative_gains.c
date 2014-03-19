/*************************************************************************

NAME: ias_cpf_parse_relative_gains

PURPOSE: Relative gains common function to pull both oli and tirs rel gains
         from the cpf.  The tirs rel gains will be moved to the common
         relative gains structure when this function returns to the 
         calling function.  
        
RETURN VALUE: SUCCSSS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 1

int ias_cpf_parse_relative_gains
(
    const IAS_CPF *cpf,             /* I: CPF structure */
    int sensor,               /* I: sensor tirs or oli */
    int band_type,            /* I: normal or blind */
    const char *group_name,               /* I: group to retrieve */
    const char *attribute_prefix,         /* I: Rel_Gains, Pre_Rel_Gains,
                                          Post_Rel_Gains */
    struct IAS_CPF_RELATIVE_GAINS *rel_gains /* O: CPF relative gains data */
)
{
    int nbands;                     /* total number bands */
    int nscas;                      /* total number scas */
    int sca_index;                  /* sca loop counter */
    int band_index;                 /* band loop var */
    int band_number;                /* Actual band number */
    int normal_band_index;          /* normal band number converted to index */
    int band_list[IAS_MAX_NBANDS];  /* list of band numbers */
    int status;                     /* Function return value */
    int ndet;                       /* band detector count */
    int count = 0;                  /* number of list buckets */

    IAS_OBJ_DESC *odl_tree;         /* ODL tree */

    /* get oli band info */
    status = ias_sat_attr_get_sensor_band_numbers(sensor, band_type, 0, 
                                                  band_list, IAS_MAX_NBANDS,
                                                  &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting band numbers for sensor id: %d", sensor);
        return ERROR;
    }
   
    /* get sca count */
    nscas = ias_sat_attr_get_sensor_sca_count(sensor);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sca count for sensor id: %d", sensor);
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[nbands * nscas * NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * nscas * NUMBER_ATTRIBUTES];

    /* set the pointers to null */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];
       
        /* get the index equivalent of the normal band number */
        normal_band_index 
            = ias_sat_attr_convert_band_number_to_index(band_list[band_index]); 
        if (normal_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting the band number to an index");
            return ERROR;
        }

        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            rel_gains->per_detector[normal_band_index][sca_index] = NULL;
        }
    }
    
    /* Loop through the bands */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];
 
        /* get the index equivalent of the normal band number */
        normal_band_index 
            = ias_sat_attr_convert_band_number_to_index(band_list[band_index]); 
        if (normal_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting the band number to an index");
            return ERROR;
        }
        /* get number of scas this band */
        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting sca count for band number: %d", 
                 band_number);
            free(rel_gains->per_detector[band_index][sca_index]);
            rel_gains->per_detector[band_index][sca_index] = NULL;
            return ERROR;
        }

        /* get detector count of current band */
        ndet = ias_sat_attr_get_detectors_per_sca(band_number);
        if (ndet == ERROR)
            {
                IAS_LOG_ERROR("Getting detector count for  band number: %d", 
                               band_number);
                free(rel_gains->per_detector[normal_band_index][sca_index]);
                rel_gains->per_detector[normal_band_index][sca_index] = NULL;
                return ERROR;
            }

        for (sca_index = 0; sca_index < nscas; sca_index++)
        {

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "%s_B%02d_SCA%02d", attribute_prefix,
                    band_number, sca_index +1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating CPF attribute name string");
                free(rel_gains->per_detector[normal_band_index][sca_index]);
                rel_gains->per_detector[normal_band_index][sca_index] = NULL;
                return ERROR;
            }


            /* allocate space for Relative Gains in CPF */
            rel_gains->per_detector[normal_band_index][sca_index] 
                                = malloc(ndet * sizeof(double));

            if (rel_gains->per_detector[normal_band_index][sca_index] == NULL)
            {
                IAS_LOG_ERROR("Allocating memory detector relative gains "
                              "group: %s", group_name);
                for (band_index = 0; band_index < nbands; band_index++)
                {
                    for (sca_index = 0; sca_index < nscas; sca_index++)
                    {
                        free(rel_gains
                                ->per_detector[normal_band_index][sca_index]);
                        rel_gains
                            ->per_detector[normal_band_index][sca_index] = NULL;
                    }
                }
                return ERROR;
            }

            /* populate list with relative gains info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                    = rel_gains->per_detector[normal_band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;
         }
    }
    
    /* make a sanity check of number of parameters to retrieve */
    if ((nbands * nscas * NUMBER_ATTRIBUTES) != count)
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
        for (band_index = 0; band_index < nbands; band_index++)
        {
            /* get band number from band index */
            band_number = band_list[band_index];

            /* get the index equivalent of the normal band number */
            normal_band_index = ias_sat_attr_convert_band_number_to_index(
                                                        band_list[band_index]); 
            if (normal_band_index == ERROR)
            {
                IAS_LOG_ERROR("Converting the band number to an index");
                return ERROR;
            }

            for (sca_index = 0; sca_index < nscas; sca_index++)
            {
                free(rel_gains->per_detector[normal_band_index][sca_index]);
                rel_gains->per_detector[normal_band_index][sca_index] = NULL;
            }
        }
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}
