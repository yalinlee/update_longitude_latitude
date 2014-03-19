/*************************************************************************

NAME: ias_cpf_parse_attitude_params

PURPOSE: Retrieve the attitude parameters data group from the odl tree and 
         insert into the internal cpf strucure.
        
RETURN VALUE: SUCCESS or FAILURE

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_attitude_params
(
    const IAS_CPF *cpf,             /* I: CPF structure */
    struct IAS_CPF_ATTITUDE_PARAMETERS *attitude
)                                   /* O: Structure of Attitude Parameters */
{
    int status;           /* Status of return from function */
    int count = 0;        /* Number of items in CPF_LIST_TYPE */
    char group_name[] = "ATTITUDE_PARAMETERS"; 
                          /* Group to retrieve from the CPF */
    IAS_OBJ_DESC *odl_tree;/* ODL tree */

    /* populate the list with the attributes to retrieve */
    ODL_LIST_TYPE list[] = 
    {
        {group_name, "CM_to_OLI_Offsets", attitude->cm_to_oli_offsets,
         sizeof(attitude->cm_to_oli_offsets), IAS_ODL_Double, 3},

        {group_name, "IRU_to_ACS_Align_Matrix", 
         attitude->iru_to_acs_align_matrix,
         sizeof(attitude->iru_to_acs_align_matrix), IAS_ODL_Double, 9},

        {group_name, "ACS_to_OLI_Rotation_Matrix", 
            attitude->acs_to_oli_rotation_matrix,
         sizeof(attitude->acs_to_oli_rotation_matrix), IAS_ODL_Double, 9},

        {group_name, "CM_to_TIRS_Offsets", attitude->cm_to_tirs_offsets,
         sizeof(attitude->cm_to_tirs_offsets), IAS_ODL_Double, 3},

        {group_name, "ACS_to_TIRS_Rotation_Matrix", 
            attitude->acs_to_tirs_rotation_matrix,
         sizeof(attitude->acs_to_tirs_rotation_matrix), IAS_ODL_Double, 9},

        {group_name, "SIRU_Axis_A", attitude->siru_axis_a,
         sizeof(attitude->siru_axis_a), IAS_ODL_Double, 3},

        {group_name, "SIRU_Axis_B", attitude->siru_axis_b,
         sizeof(attitude->siru_axis_b), IAS_ODL_Double, 3},

        {group_name, "SIRU_Axis_C", attitude->siru_axis_c,
         sizeof(attitude->siru_axis_c), IAS_ODL_Double, 3},

        {group_name, "SIRU_Axis_D", attitude->siru_axis_d,
         sizeof(attitude->siru_axis_d), IAS_ODL_Double, 3}
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
