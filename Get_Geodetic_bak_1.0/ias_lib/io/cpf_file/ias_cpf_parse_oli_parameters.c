/*************************************************************************

NAME: ias_cpf_parse_oli_parameters

PURPOSE: Retrieve the oli parameters data group from the odl tree and 
         insert into the internal cpf strucure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

int ias_cpf_parse_oli_parameters
(
    const IAS_CPF *cpf,                          /* I: CPF structure */
    struct IAS_CPF_OLI_PARAMETERS *oli_parameters /* O: CPF sensor params */
)
{
    int status;                     /* Status of return from function */
    int vrp_nbands;                 /* number of vrp bands */
    int band_list[IAS_MAX_NBANDS];  /* band numbers of vrp bands */
    int count = 0;                  /* Number of items in CPF_LIST_TYPE */
    char group_name[] = "OLI_PARAMETERS"; /* Group to retrieve from the CPF */

    IAS_OBJ_DESC *odl_tree;         /* ODL tree */

    status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, IAS_VRP_BAND,
                                                  IAS_BLIND_BAND, band_list,
                                                  IAS_MAX_NBANDS, &vrp_nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting OLI band numbers");
        return ERROR;
    }


    ODL_LIST_TYPE list[] =       /* Struct to contain group attributes */
    {
        {group_name, "SCA_Count", &oli_parameters->sca_count,
         sizeof(oli_parameters->sca_count), IAS_ODL_Int, 1},

        {group_name, "Band_Count", &oli_parameters->band_count,
         sizeof(oli_parameters->band_count), IAS_ODL_Int, 1},

        {group_name, "VRP_Count", &oli_parameters->vrp_count,
         sizeof(oli_parameters->vrp_count), IAS_ODL_Int, vrp_nbands},

        {group_name, "Nominal_Frame_Time", 
         &oli_parameters->nominal_frame_time,
         sizeof(oli_parameters->nominal_frame_time), IAS_ODL_Double, 1},

        {group_name, "Nominal_Integration_Time_MS", 
         &oli_parameters->nominal_integration_time_ms,
         sizeof(oli_parameters->nominal_integration_time_ms), 
         IAS_ODL_Double, 1},

        {group_name, "Nominal_Integration_Time_PAN", 
         &oli_parameters->nominal_integration_time_pan,
         sizeof(oli_parameters->nominal_integration_time_pan), 
         IAS_ODL_Double, 1},

        {group_name, "Integration_Time_Tolerance", 
         &oli_parameters->integration_time_tolerance,
         sizeof(oli_parameters->integration_time_tolerance), 
         IAS_ODL_Double, 1},

        {group_name, "Integration_Time_Scale", 
         &oli_parameters->integration_time_scale,
         sizeof(oli_parameters->integration_time_scale), 
         IAS_ODL_Double, 1},

        {group_name, "Detector_Settling_Time_MS", 
         &oli_parameters->detector_settling_time_ms,
         sizeof(oli_parameters->detector_settling_time_ms), IAS_ODL_Double, 1},

        {group_name, "Detector_Settling_Time_PAN", 
         &oli_parameters->detector_settling_time_pan,
         sizeof(oli_parameters->detector_settling_time_pan), IAS_ODL_Double, 1},

        {group_name, "Along_IFOV_MS", &oli_parameters->along_ifov_ms,
         sizeof(oli_parameters->along_ifov_ms), IAS_ODL_Double, 1},

        {group_name, "Along_IFOV_PAN", &oli_parameters->along_ifov_pan,
         sizeof(oli_parameters->along_ifov_pan), IAS_ODL_Double, 1},

        {group_name, "Across_IFOV_MS", &oli_parameters->across_ifov_ms,
         sizeof(oli_parameters->across_ifov_ms), IAS_ODL_Double, 1},

        {group_name, "Across_IFOV_PAN", &oli_parameters->across_ifov_pan,
         sizeof(oli_parameters->across_ifov_pan), IAS_ODL_Double, 1},

        {group_name, "Rollover_Error_Tolerance", 
         &oli_parameters->rollover_error_tolerance,
         sizeof(oli_parameters->rollover_error_tolerance), IAS_ODL_Double, 1},

        {group_name, "Frame_Time_Fill_Offset_Tolerance", 
         &oli_parameters->frame_time_fill_offset_tolerance,
         sizeof(oli_parameters->frame_time_fill_offset_tolerance), 
         IAS_ODL_Double, 1},

        {group_name, "Frame_Time_Clock_Drift_Tolerance", 
         &oli_parameters->frame_time_clock_drift_tolerance,
         sizeof(oli_parameters->frame_time_clock_drift_tolerance), 
         IAS_ODL_Double, 1}
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
} /* end of parse oli parameters */
