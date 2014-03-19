/*************************************************************************

NAME: ias_cpf_parse_tirs_thermal_constants

PURPOSE: Retrieve the tirs_thermal_constants parameters data group from the odl 
         tree and insert into the internal cpf structure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

int ias_cpf_parse_tirs_thermal_constants
(
    const IAS_CPF *cpf,                  /* I: CPF structure */
    struct IAS_CPF_TIRS_THERMAL_CONSTANTS *tirs_thermal_constants 
                                         /* O: CPF thermail constants data */
)
{
    int status;                    /* Status of return from function */
    int count = 0;                 /* Number of items in CPF_LIST_TYPE */

    char group_name[] = "TIRS_THERMAL_CONSTANTS"; 
                                   /* Group to retrieve from the CPF */

    IAS_OBJ_DESC *odl_tree;        /* ODL tree */

    ODL_LIST_TYPE list[] = 
    {
        {group_name, "K1_Constant", tirs_thermal_constants->k1_constant,
         sizeof(tirs_thermal_constants->k1_constant), IAS_ODL_Double,
         IAS_TIRS_MAX_NBANDS},

        {group_name, "K2_Constant", tirs_thermal_constants->k2_constant,
         sizeof(tirs_thermal_constants->k2_constant), IAS_ODL_Double,
         IAS_TIRS_MAX_NBANDS}
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
