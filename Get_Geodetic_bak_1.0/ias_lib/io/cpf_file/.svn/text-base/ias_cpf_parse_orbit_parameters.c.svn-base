/*************************************************************************

NAME: ias_cpf_parse_orbit_parameters

PURPOSE: 
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_orbit_parameters
(
    const IAS_CPF *cpf,                    /* I: CPF structure */
    struct IAS_CPF_ORBIT_PARAMETERS *orbit /* O: CPF Orbit Parameters */
)
{
    int status;           /* Status of return from function */
    int count = 0;        /* Number of items in CPF_LIST_TYPE */
    char group_name[] = "ORBIT_PARAMETERS"; /* Group to retrieve from the CPF */
    IAS_OBJ_DESC *odl_tree;    /* ODL tree */

    ODL_LIST_TYPE list[] = 
    {
        {group_name, "Launch_Date", &orbit->launch_date,
         sizeof(orbit->launch_date), IAS_ODL_String, 1},

        {group_name, "Scenes_Per_Orbit", &orbit->scenes_per_orbit,
         sizeof(orbit->scenes_per_orbit), IAS_ODL_Int, 1},

        {group_name, "WRS_Cycle_Days", &orbit->wrs_cycle_days,
         sizeof(orbit->wrs_cycle_days), IAS_ODL_Int, 1},

        {group_name, "WRS_Cycle_Orbits", &orbit->wrs_cycle_orbits,
         sizeof(orbit->wrs_cycle_orbits), IAS_ODL_Int, 1},

        {group_name, "Orbital_Period", &orbit->orbital_period,
         sizeof(orbit->orbital_period), IAS_ODL_Double, 1},

        {group_name, "Nominal_Angular_Momentum", 
         &orbit->nominal_angular_momentum,
         sizeof(orbit->nominal_angular_momentum), IAS_ODL_Double, 1},

        {group_name, "Nominal_Orbit_Radius", &orbit->nominal_orbit_radius,
         sizeof(orbit->nominal_orbit_radius), IAS_ODL_Double, 1},

        {group_name, "Semi_Major_Axis", &orbit->semi_major_axis,
         sizeof(orbit->semi_major_axis), IAS_ODL_Double, 1},

        {group_name, "Semi_Minor_Axis", &orbit->semi_minor_axis,
         sizeof(orbit->semi_minor_axis), IAS_ODL_Double, 1},

        {group_name, "Eccentricity", &orbit->eccentricity,
         sizeof(orbit->eccentricity), IAS_ODL_Double, 1},

        {group_name, "Inclination_Angle", &orbit->inclination_angle,
         sizeof(orbit->inclination_angle), IAS_ODL_Double, 1},

        {group_name, "Argument_Of_Perigee", &orbit->argument_of_perigee,
         sizeof(orbit->argument_of_perigee), IAS_ODL_Double, 1},

        {group_name, "Descending_Node_Row", &orbit->descending_node_row,
         sizeof(orbit->descending_node_row), IAS_ODL_Int, 1},

        {group_name, "Long_Path1_Row60", &orbit->long_path1_row60,
         sizeof(orbit->long_path1_row60), IAS_ODL_Double, 1},

        {group_name, "Descending_Node_Time_Min",
         &orbit->descending_node_time_min,
         sizeof(orbit->descending_node_time_min), IAS_ODL_String, 1},

        {group_name, "Descending_Node_Time_Max",
         &orbit->descending_node_time_max,
         sizeof(orbit->descending_node_time_max), IAS_ODL_String, 1},

        {group_name, "Nodal_Regression_Rate", &orbit->nodal_regression_rate,
         sizeof(orbit->nodal_regression_rate), IAS_ODL_Double, 1},
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
