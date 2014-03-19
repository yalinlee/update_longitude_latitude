/*************************************************************************

NAME: ias_cpf_parse_oli_radiance_rescale

PURPOSE: Retrieve the oli radiance_rescale parameters data group from the odl 
         tree and insert into the internal cpf strucure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

int ias_cpf_parse_oli_radiance_rescale
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    struct IAS_CPF_RADIANCE_RESCALE *radiance_rescale 
                                        /* O: CPF radiance rescale data */
)
{
    int status;                    /* Status of return from function */
    int count = 0;                 /* Number of items in CPF_LIST_TYPE */
    int nbands;                    /* number of normal bands */
    int band_list[IAS_MAX_NBANDS]; /* list of band numbers */
    char group_name[] = "OLI_RADIANCE_RESCALE"; 
                                   /* Group to retrieve from the CPF */
    IAS_OBJ_DESC *odl_tree;        /* ODL tree */

    /* get oli band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, IAS_NORMAL_BAND, 
                                                  0, band_list,
                                                  IAS_MAX_NBANDS, &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting sat band attributes");
        return ERROR;
    }

    ODL_LIST_TYPE list[] = 
    {
        {group_name, "Reflectance_Additive_Factor", radiance_rescale->bias,
         sizeof(radiance_rescale->bias), 
         IAS_ODL_Double, nbands},

        {group_name, "Reflectance_Multiplicative_Factor", 
         radiance_rescale->gain, sizeof(radiance_rescale->gain), 
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

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}
