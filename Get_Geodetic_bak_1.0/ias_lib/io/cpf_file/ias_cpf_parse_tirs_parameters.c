/**************************************************************************

NAME: ias_cpf_parse_tirs_parameters

PURPOSE: Retrieve the tirs parameters data group from the odl tree and 
         insert into the internal cpf strucure.
        
RETURNS: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_tirs_parameters
(
    const IAS_CPF *cpf,                            /* I: CPF structure */
    struct IAS_CPF_TIRS_PARAMETERS *tirs_parameters /* O: CPF tirs params */
)
{
    int status;                         /* Status of return from function */
    int count = 0;                      /* Number of items in CPF_LIST_TYPE */
    char group_name[] = "TIRS_PARAMETERS"; /* Group to retrieve from the CPF */

    IAS_OBJ_DESC *odl_tree;             /* ODL tree */

    ODL_LIST_TYPE list[] =              /* struct to contain group attributes */
    {
        {group_name, "Nominal_Frame_Time", &tirs_parameters->nominal_frame_time,
        sizeof(tirs_parameters->nominal_frame_time), IAS_ODL_Double, 1},

        {group_name, "Nominal_Integration_Time", 
         &tirs_parameters->nominal_integration_time, 
         sizeof(tirs_parameters->nominal_integration_time), 
         IAS_ODL_Double, 1},

        {group_name, "Integration_Time_Tolerance", 
         &tirs_parameters->integration_time_tolerance, 
         sizeof(tirs_parameters->integration_time_tolerance), 
         IAS_ODL_Double, 1},

        {group_name, "Integration_Time_Scale", 
         &tirs_parameters->integration_time_scale, 
         sizeof(tirs_parameters->integration_time_scale), 
         IAS_ODL_Double, 1},

        {group_name, "SCA_Count", &tirs_parameters->sca_count,
         sizeof(tirs_parameters->sca_count), IAS_ODL_Int, 1},

        {group_name, "Band_Count", &tirs_parameters->band_count,
         sizeof(tirs_parameters->band_count), IAS_ODL_Int, 1},

        {group_name, "SSM_Mirror_Angle", &tirs_parameters->ssm_mirror_angle,
         sizeof(tirs_parameters->ssm_mirror_angle), IAS_ODL_Double, 1},

        {group_name, "SSM_Mirror_Angle_Deviation", 
         &tirs_parameters->ssm_mirror_angle_deviation, 
         sizeof(tirs_parameters->ssm_mirror_angle_deviation), 
         IAS_ODL_Double, 1},

        {group_name, "SSM_Encoder_Origin_SideA",
            &tirs_parameters->ssm_encoder_origin_side_a,
            sizeof(tirs_parameters->ssm_encoder_origin_side_a), IAS_ODL_Double,
            1},

        {group_name, "SSM_Encoder_Origin_SideB",
            &tirs_parameters->ssm_encoder_origin_side_b,
            sizeof(tirs_parameters->ssm_encoder_origin_side_b), IAS_ODL_Double,
            1},

        {group_name, "SSM_Encoder_Time_Offset", 
         &tirs_parameters->ssm_encoder_time_offset, 
         sizeof(tirs_parameters->ssm_encoder_time_offset), IAS_ODL_Double, 1},

        {group_name, "SSM_Telescope_Roll_Offset", 
         &tirs_parameters->ssm_telescope_roll_offset, 
         sizeof(tirs_parameters->ssm_telescope_roll_offset), IAS_ODL_Double, 1},

        {group_name, "SSM_Telescope_Pitch_Offset", 
         &tirs_parameters->ssm_telescope_pitch_offset, 
         sizeof(tirs_parameters->ssm_telescope_pitch_offset), 
         IAS_ODL_Double, 1},

        {group_name, "SSM_Telescope_Yaw_Offset", 
         &tirs_parameters->ssm_telescope_yaw_offset, 
         sizeof(tirs_parameters->ssm_telescope_yaw_offset), 
         IAS_ODL_Double, 1},

        {group_name, "SSM_Tolerance",  &tirs_parameters->ssm_tolerance, 
         sizeof(tirs_parameters->ssm_tolerance), IAS_ODL_Double, 1},

        {group_name, "Along_IFOV_Thermal", &tirs_parameters->along_ifov_thermal,
         sizeof(tirs_parameters->along_ifov_thermal), IAS_ODL_Double, 1},

        {group_name, "Across_IFOV_Thermal", 
         &tirs_parameters->across_ifov_thermal, 
         sizeof(tirs_parameters->across_ifov_thermal), IAS_ODL_Double, 1},

        {group_name, "Frame_Time_Fill_Offset_Tolerance", 
         &tirs_parameters->frame_time_fill_offset_tolerance, 
         sizeof(tirs_parameters->frame_time_fill_offset_tolerance), 
         IAS_ODL_Double, 1},

        {group_name, "Frame_Time_Clock_Drift_Tolerance", 
         &tirs_parameters->frame_time_clock_drift_tolerance, 
         sizeof(tirs_parameters->frame_time_clock_drift_tolerance), 
         IAS_ODL_Double, 1},
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

    /* free allocated memory */
    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}
