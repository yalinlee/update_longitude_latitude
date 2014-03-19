/*************************************************************************

NAME: ias_cpf_parse_abs_gains

PURPOSE: Retrieve the absolute gains data group from the odl tree and 
         insert into the internal cpf structure.  Will include TIRS and
         OLI data.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 1

int ias_cpf_parse_oli_abs_gains
(
    const IAS_CPF *cpf,                      /* I: CPF structure */
    struct IAS_CPF_ABSOLUTE_GAINS *abs_gains /* O: CPF absolute gains data */
)
{
    int nbands;                          /* total number bands */
    int band_index;                      /* band loop var */
    int band_number;                     /* Actual band number */
    int band_list[IAS_MAX_NBANDS];       /* list of band numbers */
    int status;                          /* Function return value */
    int nscas;                           /* total number scas */
    int count = 0;                       /* number of list buckets */
    char group_name[] = "OLI_ABSOLUTE_GAINS"; /* group to retrieve */

    IAS_OBJ_DESC *odl_tree;              /* ODL tree */

    /* get oli band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, IAS_NORMAL_BAND, 
                                                  0, band_list, IAS_MAX_NBANDS,
                                                  &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting sat band numbers");
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[nbands * NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * NUMBER_ATTRIBUTES];

    /* set the pointers to null */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        abs_gains->gain[band_index] = NULL;
    }

    /* Loop through the bands */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];

        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Get Sat Attributes sca count returned NULL");
            for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
            {
                free(abs_gains->gain[band_index]);
                abs_gains->gain[band_index] = NULL;
            }
            return ERROR;
        } 

        /* Add the band and sca information to the CPF attribute name */
        status = snprintf(attribute[count],  sizeof(attribute[count]),
            "Gain_B%02d", band_number);
        if (status < 0 || status >= sizeof(attribute[count]))
        { 
            IAS_LOG_ERROR("Creating Gain attribute string");
            for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
            {
                free(abs_gains->gain[band_index]);
                abs_gains->gain[band_index] = NULL;
            }
            return ERROR;
        }

        /* allocate space for gain for current oli band */
        abs_gains->gain[band_index] = malloc(nscas * sizeof(double));
        if (abs_gains->gain[band_index] == NULL)
        {
            IAS_LOG_ERROR("Allocating memory group: %s band: %d", 
                            group_name, band_number);
            for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
            {
                free(abs_gains->gain[band_index]);
                abs_gains->gain[band_index] = NULL;
            }
            return ERROR;
        }

        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr = abs_gains->gain[band_index];
        list[count].parm_size = nscas * sizeof(double);
        list[count].parm_type = IAS_ODL_Double;
        list[count].parm_count = nscas;
        count++;
    }

    /* make a sanity check of number of parameters to retrieve */
    if (nbands * NUMBER_ATTRIBUTES != count)
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
        for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
        {
             free(abs_gains->gain[band_index]);
             abs_gains->gain[band_index] = NULL;
        }
        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }
    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
} /* end of parse oli abs gains */
