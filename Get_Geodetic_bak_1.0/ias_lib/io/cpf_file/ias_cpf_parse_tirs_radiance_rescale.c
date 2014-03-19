/*************************************************************************

NAME: ias_cpf_parse_tirs_radiance_rescale

PURPOSE: Retrieve the radiance_rescale parameters data group from the odl 
         tree and insert into the internal cpf strucure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

int ias_cpf_parse_tirs_radiance_rescale
(
    const IAS_CPF *cpf,                     /* I: CPF structure */
    struct IAS_CPF_RADIANCE_RESCALE *radiance_rescale 
                                            /* O: CPF radiance rescale data */
)
{
    int status;                    /* Status of return from function */
    int count = 0;                 /* Number of items in CPF_LIST_TYPE */
    int nbands;                    /* number of tirs normal bands */
    int band_index;                /* band loop counter */
    int band_list[IAS_MAX_NBANDS]; /* list of band numbers */
    int normal_band_index;         /* normal band number converted to index */
    double gain[IAS_MAX_NBANDS];   /* Tirs gain values from cpf */
    double bias[IAS_MAX_NBANDS];   /* Tirs bias values from cpf */

    char group_name[] = "TIRS_RADIANCE_RESCALE"; 
                                   /* Group to retrieve from the CPF */

    IAS_OBJ_DESC *odl_tree;        /* ODL tree */

    /* get tirs band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_TIRS, IAS_NORMAL_BAND, 
                                                  0, band_list,
                                                  IAS_MAX_NBANDS, &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting tirs sat band attributes");
        return ERROR;
    }
    ODL_LIST_TYPE list[] = 
    {
        {group_name, "Radiance_Additive_Factor", bias, sizeof(bias), 
         IAS_ODL_Double, nbands},

        {group_name, "Radiance_Multiplicative_Factor", gain, sizeof(gain), 
         IAS_ODL_Double, nbands}
    };

    /* Calculate the number of attributes to retrieve */
    count = sizeof(list) / sizeof(ODL_LIST_TYPE);

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }

    /* move the tirs data to the common structure elements */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get the index equivalent of the normal band number */
        normal_band_index 
            = ias_sat_attr_convert_band_number_to_index(band_list[band_index]);
        if (normal_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting the band number to an index");
            return ERROR;
        }

        radiance_rescale->bias[normal_band_index] =    
                bias[band_index];
        radiance_rescale->gain[normal_band_index] =    
                gain[band_index];
    }

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}
