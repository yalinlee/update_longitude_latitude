/*************************************************************************

NAME: ias_cpf_parse_ancil_conv

PURPOSE: Retrieve the ancillary conv data group from the odl tree and 
         insert into the internal cpf structure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_ancil_conv
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    struct IAS_CPF_ANCILLARY_ENG_CONV *ancil_eng_conv  
                                        /* O: Structure for ancil eng conv */
)
{
    int status;                            /* Function return value */
    char group_name[] = "ANCILLARY_ENG_CONV"; /* Name of group to retrieve */
    int count = 0;                         /* number of list buckets */
    IAS_OBJ_DESC *odl_tree;                /* ODL tree */


    /* populate the list of elements to retrieve */
    ODL_LIST_TYPE list[] =             /* Struct to contain group attributes */
    {
        {group_name, "Quaternion_Eng_Conv",
         &ancil_eng_conv->quaternion_eng_conv,
         sizeof(ancil_eng_conv->quaternion_eng_conv), IAS_ODL_Double, 1},

        {group_name, "Quaternion_Conv_Vector",
         &ancil_eng_conv->quaternion_conv_vector,
         sizeof(ancil_eng_conv->quaternion_conv_vector), IAS_ODL_Double, 1},

        {group_name, "Quaternion_Conv_Scalar",
         &ancil_eng_conv->quaternion_conv_scalar,
         sizeof(ancil_eng_conv->quaternion_conv_scalar), IAS_ODL_Double, 1},

        {group_name, "Star_Conv_Time",
         &ancil_eng_conv->star_conv_time,
         sizeof(ancil_eng_conv->star_conv_time), IAS_ODL_Double, 1},

        {group_name, "Star_Conv_Position", &ancil_eng_conv->star_conv_position,
         sizeof(ancil_eng_conv->star_conv_position), IAS_ODL_Double, 1},

        {group_name, "Star_Conv_Intensity", 
         &ancil_eng_conv->star_conv_intensity,
         sizeof(ancil_eng_conv->star_conv_intensity), IAS_ODL_Double, 1},

        {group_name, "Gyro_Conv_Angle", &ancil_eng_conv->gyro_conv_angle,
         sizeof(ancil_eng_conv->gyro_conv_angle), IAS_ODL_Double, 1},

        {group_name, "Gyro_Conv_Time", &ancil_eng_conv->gyro_conv_time,
         sizeof(ancil_eng_conv->gyro_conv_time), IAS_ODL_Double, 1},

        {group_name, "TIRS_Integration_Time_Scale",
         &ancil_eng_conv->tirs_integration_time_scale,
         sizeof(ancil_eng_conv->tirs_integration_time_scale), IAS_ODL_Double,
         1},

        {group_name, "OLI_Integration_Time_Scale",
         &ancil_eng_conv->oli_integration_time_scale,
         sizeof(ancil_eng_conv->oli_integration_time_scale), 
         IAS_ODL_Double, 1},

        {group_name, "OLI_Integration_Time_MS_Offset_Nominal",
         &ancil_eng_conv->oli_integration_time_ms_offset_nominal,
         sizeof(ancil_eng_conv->oli_integration_time_ms_offset_nominal), 
         IAS_ODL_Int, 1},

        {group_name, "OLI_Integration_Time_PAN_Offset_Nominal",
         &ancil_eng_conv->oli_integration_time_pan_offset_nominal,
         sizeof(ancil_eng_conv->oli_integration_time_pan_offset_nominal), 
         IAS_ODL_Int, 1},

        {group_name, "OLI_Integration_Time_MS_Offset_8x",
         &ancil_eng_conv->oli_integration_time_ms_offset_8x,
         sizeof(ancil_eng_conv->oli_integration_time_ms_offset_8x), 
         IAS_ODL_Int, 1},

        {group_name, "OLI_Integration_Time_PAN_Offset_8x",
         &ancil_eng_conv->oli_integration_time_pan_offset_8x,
         sizeof(ancil_eng_conv->oli_integration_time_pan_offset_8x), 
         IAS_ODL_Int, 1},
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
} /* end of ias_cpf_parse_ancil_conv */
