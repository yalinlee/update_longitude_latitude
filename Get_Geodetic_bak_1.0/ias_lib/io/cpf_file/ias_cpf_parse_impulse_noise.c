/*************************************************************************

NAME: ias_cpf_parse_impulse_noise

PURPOSE: Retrieve the OLI and TIRS impulse noise data group from the 
         odl tree and insert into the internal cpf strucure.
         Includes OLI normal and VRP bands.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_impulse_noise
(
    const IAS_CPF *cpf,                         /* I: CPF structure */
    struct IAS_CPF_IMPULSE_NOISE *impulse_noise /* O: CPF Impulse Noise Parms */
)
{
    int status;                          /* Status of return from function */
    int count = 0;                       /* Number of items in CPF_LIST_TYPE */
    char group_name[] = "IMPULSE_NOISE"; /* Group to retrieve from the CPF */

    IAS_OBJ_DESC *odl_tree;              /* ODL tree */
    
    ODL_LIST_TYPE list[] =               /* Struct for group attributes */
    {
        {group_name, "IN_Limit", &impulse_noise->in_limit,
         sizeof(impulse_noise->in_limit), IAS_ODL_Int, IAS_MAX_TOTAL_BANDS},

        {group_name, "Median_Filter_Width", &impulse_noise->median_filter_width,
         sizeof(impulse_noise->median_filter_width), IAS_ODL_Int, 
         IAS_MAX_TOTAL_BANDS},
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
