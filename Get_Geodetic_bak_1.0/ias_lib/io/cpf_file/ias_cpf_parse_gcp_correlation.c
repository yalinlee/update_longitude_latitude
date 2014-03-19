/*************************************************************************

NAME: ias_cpf_parse_gcp_correlation

PURPOSE: Retrieve the gcp correlation data group from the odl tree and insert 
         into internal cpf structure
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_gcp_correlation
(
    const IAS_CPF *cpf,                        /* I: CPF structure */
    struct IAS_CPF_GCP_CORRELATION *gcp_corr   /* O: CPF gcp corr data group */
)
{
    int status;                      /* Function return value */
    char group_name[] = "GCP_CORRELATION"; /* Name of group to retrieve */
    int count;                       /* Number of group attributes */
    IAS_OBJ_DESC *odl_tree;          /* ODL tree */

    ODL_LIST_TYPE list[] =       /* Struct to contain group attributes */
    {
        {group_name, "Corr_Fit_Method", &gcp_corr->corr_fit_method,
         sizeof(gcp_corr->corr_fit_method), IAS_ODL_Int, 1},

        {group_name, "Corr_Window_Size", &gcp_corr->corr_window_size,
         sizeof(gcp_corr->corr_window_size), IAS_ODL_Int, 2},

        {group_name, "Max_Displacement_Offset", 
         &gcp_corr->max_displacement_offset,
         sizeof(gcp_corr->max_displacement_offset), IAS_ODL_Int, 1},

        {group_name, "Min_Corr_Strength", &gcp_corr->min_corr_strength,
         sizeof(gcp_corr->min_corr_strength), IAS_ODL_Double, 1},

        {group_name, "Fill_Threshold", &gcp_corr->fill_threshold,
         sizeof(gcp_corr->fill_threshold), IAS_ODL_Double, 1},

        {group_name, "Corr_Fill_Value", &gcp_corr->corr_fill_value,
         sizeof(gcp_corr->corr_fill_value), IAS_ODL_Int, 1}
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
