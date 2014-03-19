/*************************************************************************

NAME: ias_cpf_parse_reflect_conv

PURPOSE: Retrieve the reflect_conv parameters data group from the odl 
         tree and insert into the internal cpf strucure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

int ias_cpf_parse_reflect_conv
(
    const IAS_CPF *cpf,             /* I: CPF structure */
    struct IAS_CPF_REFLECTANCE_CONVERSION *reflect_conv 
                                    /* O: CPF reflectance conversion data */
)
{
    int status;                      /* Status of return from function */
    int count = 0;                   /* Number of items in CPF_LIST_TYPE */
    int band_list[IAS_MAX_NBANDS];   /* array of band numbers */
    int nbands;                      /* band count returned */
    char group_name[] = "REFLECTANCE_CONVERSION"; 
                                     /* Group to retrieve from the CPF */
    IAS_OBJ_DESC *odl_tree;          /* ODL tree */

    status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, IAS_NORMAL_BAND, 
                                                  0, band_list, IAS_MAX_NBANDS,
                                                  &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting oli sat band attributes");
        return ERROR;
    }

    ODL_LIST_TYPE list[] = 
    {
        {group_name, "Reflect_Conv_Coeff", &reflect_conv->reflect_conv_coeff,
         sizeof(reflect_conv->reflect_conv_coeff), IAS_ODL_Double, nbands }
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
