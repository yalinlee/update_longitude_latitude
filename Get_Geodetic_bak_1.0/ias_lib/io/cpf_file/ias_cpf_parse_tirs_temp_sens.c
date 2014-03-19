/*************************************************************************

NAME: ias_cpf_parse_tirs_temp_sens

PURPOSE: Retrieve the TIRS tempature sensitivity data group from the odl tree 
         and insert into the internal cpf structure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 2
#define NUMBER_BAND_ATTRIBUTES 1

int ias_cpf_parse_tirs_temp_sens
(
    const IAS_CPF *cpf,                        /* I: CPF structure */
    struct IAS_CPF_TEMP_SENSITIVITY *tirs_temp_sens 
                                               /* O: CPF det offsets data */
)
{
    int nbands;                     /* total number bands */
    int band_index;                 /* band loop var */
    int normal_band_index = 0;      /* normal band number converted to index */
    int band_number;                /* Actual band number */
    int band_list[IAS_MAX_NBANDS];  /* list of band numbers */
    int status;                     /* Function return value */
    int nscas;                      /* total number scas */
    int sca_index;                  /* SCA loop var */
    int count = 0;                  /* number of list buckets */
    int ndet;                       /* band detector count */
    char group_name[] = "TIRS_TEMP_SENSITIVITY"; /* Name of group to retrieve */

    IAS_OBJ_DESC *odl_tree;              /* ODL tree */
    
    /* get TIRS normal band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_TIRS, IAS_NORMAL_BAND, 
                                                  0, band_list,
                                                  IAS_MAX_NBANDS, &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting sat band attributes");
        return ERROR;
    }

    /* get sca count */
    nscas = ias_sat_attr_get_sensor_sca_count(IAS_TIRS);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sat sca count");
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[nbands * nscas * NUMBER_BAND_ATTRIBUTES
                   + NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * nscas * NUMBER_BAND_ATTRIBUTES
                       + NUMBER_ATTRIBUTES];

    /* Retrieve the detector level values */
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
            for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
            {
                for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                {
                    free(tirs_temp_sens
                        ->temp_sensitivity_coeff[band_index][sca_index]);
                    tirs_temp_sens->temp_sensitivity_coeff
                        [band_index][sca_index] = NULL;
                }
            }
            return ERROR;
        }

        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting sca count for band number: %d", band_number);
            for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
            {
                for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                {
                    free(tirs_temp_sens
                        ->temp_sensitivity_coeff[band_index][sca_index]);
                    tirs_temp_sens->temp_sensitivity_coeff
                        [band_index][sca_index] = NULL;
                }
            }
            return ERROR;
        }

        /* get detector count of current band */
        ndet = ias_sat_attr_get_detectors_per_sca(band_number);
        if (ndet == ERROR)
        {
            IAS_LOG_ERROR("Getting detector count for  band number: %d", 
                           band_number);
            for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
            {
                for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                {
                    free(tirs_temp_sens
                        ->temp_sensitivity_coeff[band_index][sca_index]);
                    tirs_temp_sens->temp_sensitivity_coeff
                        [band_index][sca_index] = NULL;
                }
            }
            return ERROR;
        }

        /* loop through the scas */
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                     "%s_B%02d_SCA%02d", "Temp_Sensitivity_Coeff", 
                     band_number, sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Temp_Sensitivity_Coeff attribute "
                              "string");
                for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
                {
                    for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                    {
                        free(tirs_temp_sens->temp_sensitivity_coeff
                            [band_index][sca_index]);
                        tirs_temp_sens->temp_sensitivity_coeff
                            [band_index][sca_index] = NULL;
                    }
                }
                return ERROR;
            }

            /* allocate space for Temperature Sensitivity Coeff in cpf */
            tirs_temp_sens->temp_sensitivity_coeff[normal_band_index][sca_index]
                                              = malloc(ndet * sizeof(double));
            if (tirs_temp_sens->
                temp_sensitivity_coeff[normal_band_index][sca_index] == NULL)
            {
                /* need to free memory here */
                IAS_LOG_ERROR("Allocating memory for Temp Sensitivity"
                              " Coeffs group: %s", group_name);
                for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
                {
                    for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                    {
                        free(tirs_temp_sens->temp_sensitivity_coeff
                            [band_index][sca_index]);
                        tirs_temp_sens->temp_sensitivity_coeff
                            [band_index][sca_index] = NULL;
                    }
                }
                return ERROR;
            }

            /* populate list with temperature sensitivity coeffs info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr = tirs_temp_sens
                   ->temp_sensitivity_coeff[normal_band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;

        }
    }

    /* Retrieve reference temp from odl tree */
    list[count].group_name = group_name;
    list[count].attribute = "TIRS_Reference_Temp";
    list[count].parm_ptr = (void *) &tirs_temp_sens->tirs_reference_temp;
    list[count].parm_size = sizeof(double);
    list[count].parm_type = IAS_ODL_Double;
    list[count].parm_count = 1;
    count++;

    list[count].group_name = group_name;
    list[count].attribute = "TIRS_Thermistor_Flag";
    list[count].parm_ptr = (void *) &tirs_temp_sens->tirs_thermistor_flag;
    list[count].parm_size = sizeof(int) * 4;
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = 4;
    count++;

   GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* make a sanity check of number of parameters to retrieve */
    if ((nbands * nscas * NUMBER_BAND_ATTRIBUTES + NUMBER_ATTRIBUTES) != count)
    {
        IAS_LOG_ERROR("Number of parameters does not match number to retrieve");
        DROP_ODL_TREE(odl_tree);
        for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
        {
            for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
            {
                free(tirs_temp_sens
                    ->temp_sensitivity_coeff[band_index][sca_index]);
                tirs_temp_sens->temp_sensitivity_coeff[band_index][sca_index]
                    = NULL;
            }
        }
        return ERROR;
    }

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
        {
            for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
            {
                free(tirs_temp_sens
                    ->temp_sensitivity_coeff[band_index][sca_index]);
                tirs_temp_sens->temp_sensitivity_coeff[band_index][sca_index]
                    = NULL;
            }
        }
        return ERROR;
    }

    /* free allocated memory */
    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}
