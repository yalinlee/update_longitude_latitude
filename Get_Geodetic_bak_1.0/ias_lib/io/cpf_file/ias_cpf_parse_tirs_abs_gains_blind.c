/****************************************************************************

NAME:   ias_cpf_parse_tirs_abs_gains_blind

PURPOSE: Retrieve the tirs absolute gains data group from  the odl tree and
         insert into the internal abs gains cpf structure.

RETURNS: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

# define NUMBER_ATTRIBUTES 1

int ias_cpf_parse_tirs_abs_gains_blind
(
    const IAS_CPF *cpf,                         /* I: CPF structure */
    struct IAS_CPF_ABSOLUTE_GAINS *tirs_abs_gains_blind
                                                /* O: tirs abs gains data */
)
{
    int nbands;                          /* total tirs bands */
    int band_index;                      /* band loop counter */
    int band_number;                     /* actual band number */
    int band_list[IAS_MAX_NBANDS];       /* list of band numbers */
    int nscas;                           /* total number scas */
    int status;                          /* Status of return from function */
    int count = 0;                       /* Number of items in CPF_LIST_TYPE */
    char group_name[] = "TIRS_ABSOLUTE_GAINS_BLIND"; /* group to retrieve */

    IAS_OBJ_DESC *odl_tree;              /* ODL tree */

    /* get tirs band count */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_TIRS, IAS_BLIND_BAND, 
                                                  IAS_SECONDARY_BAND, 
                                                  band_list, IAS_MAX_NBANDS, 
                                                  &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Get sensor band numbers");
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[nbands][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands];

    /* set the pointers to null */
    for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
    {
        tirs_abs_gains_blind->gain[band_index] = NULL;
    }

    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band list */
        band_number = band_list[band_index];

        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting sat attributes");
            free(tirs_abs_gains_blind->gain[band_index]);
            tirs_abs_gains_blind->gain[band_index] = NULL;
            return ERROR;
        }

        /* Add the band and sca information to the CPF attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]), 
                 "Gain_B%02d", band_number);
        if (status < 0 || status >= sizeof(attribute[count]))
        { 
            IAS_LOG_ERROR("Creating Gain attribute string");
            free(tirs_abs_gains_blind->gain[band_index]);
            tirs_abs_gains_blind->gain[band_index] = NULL;
            return ERROR;
        }

        /* allocate space for gain for current band */
        tirs_abs_gains_blind->gain[band_index] = malloc(nscas * sizeof(double));
        if (tirs_abs_gains_blind->gain[band_index] == NULL)
        {
            IAS_LOG_ERROR("Allocating memory for group: %s, band: %d", 
                                                    group_name, band_number);
            for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
            {
                free(tirs_abs_gains_blind->gain[band_index]);
                tirs_abs_gains_blind->gain[band_index] = NULL;
            }
            return ERROR;
        }

        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr = tirs_abs_gains_blind->gain[band_index];
        list[count].parm_size = nscas * sizeof(double);
        list[count].parm_type = IAS_ODL_Double;
        list[count].parm_count = nscas;
        count++;
    }
    
    /* make a sanity check of number of parameters to retrieve */
    if ((NUMBER_ATTRIBUTES) != count)
    {
        IAS_LOG_ERROR("Number of parameters does not match number to retrieve");
        return ERROR;
    }

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting tirs absolute gains blind group from CPF");
        for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
        {
            free(tirs_abs_gains_blind->gain[band_index]);
            tirs_abs_gains_blind->gain[band_index] = NULL ;
        }
        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
} /* end of ias_cpf_parse_tirs_abs_gains */
