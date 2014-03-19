/*************************************************************************

NAME: ias_cpf_parse_avg_bias

PURPOSE: Retrieve the average bias data group from the odl tree and insert
         into the internal cpf structure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 10

/* local free memory routine */
void free_avg_bias_memory
(
    int nbands,                                 /* band loop control */
    int nscas,                                  /* sca loop control */
    struct IAS_CPF_AVERAGE_BIAS *oli_avg_bias   /* CPF average bias data */
);

int ias_cpf_parse_oli_avg_bias
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_AVERAGE_BIAS *oli_avg_bias /* O: CPF average bias data */
)
{
    int status;                         /* Function return value */
    int nbands;                         /* total number bands */
    int band_index;                     /* Band loop var */
    int sca_index;                      /* SCA loop var */
    int nscas;                          /* Total number scas */
    int band_list[IAS_MAX_NBANDS];      /* List of band numbers */
    int count = 0;                      /* number of list buckets */
    int ndet;                           /* band detector count */

    char group_name[] = "AVERAGE_BIAS"; /* Name of group to retrieve */
    IAS_OBJ_DESC *odl_tree;             /* ODL tree */

    /* get sca count */
    nscas = ias_sat_attr_get_sensor_sca_count(IAS_OLI);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sat sca count");
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[nscas * NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nscas * NUMBER_ATTRIBUTES];

    /* set the pointers to null */
    for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
    {
        for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
        {
            oli_avg_bias->bias_vnir[band_index][sca_index] = NULL;
            oli_avg_bias->bias_swir[band_index][sca_index] = NULL;
        }
    }

    /* set the pan band pointers to NULL */
    for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
    {
        oli_avg_bias->bias_even_pan[sca_index] = NULL;
        oli_avg_bias->bias_odd_pan[sca_index] = NULL;
    }

    /* get oli band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, IAS_NORMAL_BAND, 
                                                  0, band_list, IAS_MAX_NBANDS,
                                                  &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting sat band attributes");
        return ERROR;
    }
   
    /* Loop through the bands */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        int band_number = band_list[band_index];
        IAS_BAND_TYPE band_type =
            ias_sat_attr_get_band_type_from_band_number(band_number);
        if (band_type == IAS_UNKNOWN_BAND_TYPE)
        {
            IAS_LOG_ERROR("Determining band type for band number %d",
                band_number);
            free_avg_bias_memory(nbands, nscas, oli_avg_bias);
            return ERROR;
        }

        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting sca count for band number: %d", 
                 band_number);
            free_avg_bias_memory(nbands, nscas, oli_avg_bias);
            return ERROR;
        }
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            /* get the detector count of current band */
            ndet = ias_sat_attr_get_detectors_per_sca(band_number);
            if(ndet == ERROR)
            {
                IAS_LOG_ERROR("Getting detector count for band number: %d", 
                                band_number);
                free_avg_bias_memory(nbands, nscas, oli_avg_bias);
                return ERROR;
            }

            /* Construct the parameter name based on the type of the
               current band */
            if (band_type == IAS_PAN_BAND)
            {
                /* Add band and sca information to the CPF attribute name */
                status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Bias_Odd_B%02d_SCA%02d", band_index + 1, sca_index + 1);
                if (status < 0 || status >= sizeof(attribute[count]))
                { 
                    IAS_LOG_ERROR("Creating Bias_Odd attribute string");
                    free_avg_bias_memory(nbands, nscas, oli_avg_bias);
                    return ERROR;
                }

                /* allocate space for array in CPF */
                oli_avg_bias->bias_odd_pan[sca_index] 
                                = malloc(ndet * sizeof(double));
                if (oli_avg_bias->bias_odd_pan[sca_index] == NULL)
                {
                    IAS_LOG_ERROR("Allocating memory pan band "
                                  "odd frame group: %s", group_name);
                    free_avg_bias_memory(nbands, nscas, oli_avg_bias);
                    return ERROR;
                }

                /* populate list with pre bias pan odd frame info */
                list[count].group_name = group_name;
                list[count].attribute = attribute[count];
                list[count].parm_ptr = oli_avg_bias->bias_odd_pan[sca_index];
                list[count].parm_size = ndet * sizeof(double);
                list[count].parm_type = IAS_ODL_Double;
                list[count].parm_count = ndet;
                count++;

                /* Add band and sca information to the CPF attribute name */
                status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Bias_Even_B%02d_SCA%02d", band_index + 1, sca_index + 1);
                if (status < 0 || status >= sizeof(attribute[count]))
                { 
                    IAS_LOG_ERROR("Creating Bias_Even attribute string");
                    free_avg_bias_memory(nbands, nscas, oli_avg_bias);
                    return ERROR;
                }

                /* allocate space for array in CPF */
                oli_avg_bias->bias_even_pan[sca_index] 
                                = malloc(ndet * sizeof(double));
                if (oli_avg_bias->bias_even_pan[sca_index] == NULL)
                {
                    IAS_LOG_ERROR("Allocating memory pan band "
                                  "even frame group: %s", group_name);
                    free_avg_bias_memory(nbands, nscas, oli_avg_bias);
                    return ERROR;
                }

                /* populate list with pre bias pan even frame info */
                list[count].group_name = group_name;
                list[count].attribute = attribute[count];
                list[count].parm_ptr = oli_avg_bias->bias_even_pan[sca_index];
                list[count].parm_size = ndet * sizeof(double);
                list[count].parm_type = IAS_ODL_Double;
                list[count].parm_count = ndet;
                count++;

            } /* end of pan band processing */
            else if (band_type == IAS_SWIR1_BAND /* do the swir bands 6,7,9 */
                  || band_type == IAS_SWIR2_BAND 
                  || band_type == IAS_CIRRUS_BAND)
            {
                /* Add band and sca information to the CPF attribute name */
                status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Bias_SWIR_B%02d_SCA%02d", band_index + 1, sca_index + 1);
                if (status < 0 || status >= sizeof(attribute[count]))
                { 
                    IAS_LOG_ERROR("Creating Bias_SWIR attribute string");
                    free_avg_bias_memory(nbands, nscas, oli_avg_bias);
                    return ERROR;
                }
    
                /* allocate space for array in CPF */
                oli_avg_bias->bias_swir[band_index][sca_index] 
                                = malloc(ndet * sizeof(double));
                if (oli_avg_bias->bias_swir[band_index][sca_index] == NULL)
                {
                    /* need to free memory here */
                    IAS_LOG_ERROR("Allocating memory swir bias group: %s",
                                    group_name);
                    free_avg_bias_memory(nbands, nscas, oli_avg_bias);
                    return ERROR;
                }
    
                /* populate list with pre bias info */
                list[count].group_name = group_name;
                list[count].attribute = attribute[count];
                list[count].parm_ptr 
                            = oli_avg_bias->bias_swir[band_index][sca_index];
                list[count].parm_size = ndet * sizeof(double);
                list[count].parm_type = IAS_ODL_Double;
                list[count].parm_count = ndet;
                count++;

            } /* end of swir band check */
            else  /* do the vnir bands 1,2,3,4,5 */
            {
                status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Bias_VNIR_B%02d_SCA%02d", band_index + 1, sca_index + 1);
                if (status < 0 || status >= sizeof(attribute[count]))
                { 
                    IAS_LOG_ERROR("Creating Bias_SWIR attribute string");
                    free_avg_bias_memory(nbands, nscas, oli_avg_bias);
                    return ERROR;
                }
    
                /* allocate space for array in CPF */
                oli_avg_bias->bias_vnir[band_index][sca_index] 
                                = malloc(ndet * sizeof(double));
                if (oli_avg_bias->bias_vnir[band_index][sca_index] == NULL)
                {
                    /* need to free memory here */
                    IAS_LOG_ERROR("Allocating memory vnir bias group: %s",
                                    group_name);
                    free_avg_bias_memory(nbands, nscas, oli_avg_bias);
                    return ERROR;
                }
    
                /* populate list with pre bias info */
                list[count].group_name = group_name;
                list[count].attribute = attribute[count];
                list[count].parm_ptr 
                            = oli_avg_bias->bias_vnir[band_index][sca_index];
                list[count].parm_size = ndet * sizeof(double);
                list[count].parm_type = IAS_ODL_Double;
                list[count].parm_count = ndet;
                count++;

            }  /* end if vnir bands */
         } /* sca for loop */
    } /* band for loop */
    
    /* make a sanity check of number of parameters to retrieve */
    if ((nscas * NUMBER_ATTRIBUTES) != count)
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
        free_avg_bias_memory(nbands, nscas, oli_avg_bias);
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}

void free_avg_bias_memory
(
    int nbands,                                 /* band loop control */
    int nscas,                                  /* sca loop control */
    struct IAS_CPF_AVERAGE_BIAS *oli_avg_bias   /* CPF average bias data */
)
{
    int band_index;                             /* band loop control */
    int sca_index;                              /* sca loop control */

    for (band_index = 0; band_index < nbands; band_index++)
    {
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            free(oli_avg_bias->bias_vnir[band_index][sca_index]);
            oli_avg_bias->bias_vnir[band_index][sca_index] = NULL;
            free(oli_avg_bias->bias_swir[band_index][sca_index]);
            oli_avg_bias->bias_swir[band_index][sca_index] = NULL;
        }
    }
    for (sca_index = 0; sca_index < nscas; sca_index++)
    {
        free(oli_avg_bias->bias_even_pan[sca_index]);
        oli_avg_bias->bias_even_pan[sca_index] = NULL;
        free(oli_avg_bias->bias_odd_pan[sca_index]);
        oli_avg_bias->bias_odd_pan[sca_index] = NULL;
    }
}
