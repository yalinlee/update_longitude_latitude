/*************************************************************************

NAME: ias_cpf_parse_geo_system

PURPOSE: Retrieve the geo system data group from the odl tree and insert 
         into internal cpf structure
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_geo_system
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    struct IAS_CPF_GEO_SYSTEM *geo_sys  /* O: CPF geo system data */
)
{
    int status;                       /* Function return value */
    char group_name[] = "GEO_SYSTEM"; /* Name of group to retrieve */
    int count;                        /* Number of group attributes */
    IAS_OBJ_DESC *odl_tree;           /* ODL tree */

    ODL_LIST_TYPE list[] =       /* Struct to contain group attributes */
    {
        {group_name, "X_Prefit_GCP_RMS", &geo_sys->x_prefit_gcp_rms,
         sizeof(geo_sys->x_prefit_gcp_rms), IAS_ODL_Double, 1},

        {group_name, "Y_Prefit_GCP_RMS", &geo_sys->y_prefit_gcp_rms,
         sizeof(geo_sys->y_prefit_gcp_rms), IAS_ODL_Double, 1},

        {group_name, "X_Postfit_GCP_RMS", &geo_sys->x_postfit_gcp_rms,
         sizeof(geo_sys->x_postfit_gcp_rms), IAS_ODL_Double, 1},

        {group_name, "Y_Postfit_GCP_RMS", &geo_sys->y_postfit_gcp_rms,
         sizeof(geo_sys->y_postfit_gcp_rms), IAS_ODL_Double, 1},

        {group_name, "Max_Percent_GCP_Outliers",
         &geo_sys->max_percent_gcp_outliers,
         sizeof(geo_sys->max_percent_gcp_outliers), IAS_ODL_Double, 1},

        {group_name, "Percent_Outlier_Threshold", 
         &geo_sys->percent_outlier_threshold,
         sizeof(geo_sys->percent_outlier_threshold), IAS_ODL_Double, 1},

        {group_name, "Time_Code_Outlier_Threshold", 
         &geo_sys->time_code_outlier_threshold,
         sizeof(geo_sys->time_code_outlier_threshold), IAS_ODL_Double, 1},

        {group_name, "MS_Grid_Density_Elev", &geo_sys->ms_grid_density_elev,
         sizeof(geo_sys->ms_grid_density_elev), IAS_ODL_Int, 1},

        {group_name, "PAN_Grid_Density_Elev", &geo_sys->pan_grid_density_elev,
         sizeof(geo_sys->pan_grid_density_elev), IAS_ODL_Int, 1},

        {group_name, "TIRS_Grid_Density_Elev", &geo_sys->tirs_grid_density_elev,
         sizeof(geo_sys->tirs_grid_density_elev), IAS_ODL_Int, 1},

        {group_name, "MS_Grid_Density_Lines", &geo_sys->ms_grid_density_lines,
         sizeof(geo_sys->ms_grid_density_lines), IAS_ODL_Int, 1},

        {group_name, "PAN_Grid_Density_Lines", &geo_sys->pan_grid_density_lines,
         sizeof(geo_sys->pan_grid_density_lines), IAS_ODL_Int, 1},

        {group_name, "TIRS_Grid_Density_Lines",
         &geo_sys->tirs_grid_density_lines,
         sizeof(geo_sys->tirs_grid_density_lines), IAS_ODL_Int, 1},

        {group_name, "MS_Grid_Density_Samples",
         &geo_sys->ms_grid_density_samples,
         sizeof(geo_sys->ms_grid_density_samples), IAS_ODL_Int, 1},

        {group_name, "PAN_Grid_Density_Samples", 
         &geo_sys->pan_grid_density_samples,
         sizeof(geo_sys->pan_grid_density_samples), IAS_ODL_Int, 1},

        {group_name, "TIRS_Grid_Density_Samples", 
         &geo_sys->tirs_grid_density_samples,
         sizeof(geo_sys->tirs_grid_density_samples), IAS_ODL_Int, 1},

        {group_name, "Optimal_Band_Order", 
         &geo_sys->optimal_band_order,
         sizeof(geo_sys->optimal_band_order), IAS_ODL_Int, IAS_MAX_NBANDS},

        {group_name, "Low_Pass_Cutoff",
         &geo_sys->low_pass_cutoff,
         sizeof(geo_sys->low_pass_cutoff), IAS_ODL_Double, 1},

        {group_name, "Minimum_Number_Correlated_Validation_GCPs",
         &geo_sys->minimum_number_correlated_validation_gcps,
         sizeof(geo_sys->minimum_number_correlated_validation_gcps),
         IAS_ODL_Int, 1},

        {group_name, "Minimum_Number_Nonoutlier_Control_GCPs",
         &geo_sys->minimum_number_nonoutlier_control_gcps,
         sizeof(geo_sys->minimum_number_nonoutlier_control_gcps),
         IAS_ODL_Int, 1}
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
