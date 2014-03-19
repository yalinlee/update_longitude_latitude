/*************************************************************************

NAME: ias_cpf_parse_tirs_align_cal

PURPOSE: Retrieve the tirs_align_cal data group from the odl tree and insert 
         into internal cpf structure
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_tirs_align_cal
(
    const IAS_CPF *cpf,                        /* I: CPF structure */
    struct IAS_CPF_TIRS_ALIGN_CAL *tirs_align_cal 
                                               /* O: CPF tirs align cal data */
)
{
    int status;                           /* Function return value */
    char group_name[] = "TIRS_ALIGN_CAL"; /* Name of group to retrieve */
    int count;                            /* Number of group attributes */
    IAS_OBJ_DESC *odl_tree;               /* ODL tree */

    ODL_LIST_TYPE list[] =       /* Struct to contain group attributes */
    {
        {group_name, "Corr_Window_Size", &tirs_align_cal->corr_window_size,
         sizeof(tirs_align_cal->corr_window_size), IAS_ODL_Int, 2},

        {group_name, "Min_Corr_Strength", &tirs_align_cal->min_corr_strength,
         sizeof(tirs_align_cal->min_corr_strength), IAS_ODL_Double, 1},

        {group_name, "Max_Displacement_Offset", 
         &tirs_align_cal->max_displacement_offset,
         sizeof(tirs_align_cal->max_displacement_offset), IAS_ODL_Double, 1},

        {group_name, "Fill_Threshold", &tirs_align_cal->fill_threshold,
         sizeof(tirs_align_cal->fill_threshold), IAS_ODL_Double, 1},

        {group_name, "Corr_Fit_Method", &tirs_align_cal->corr_fit_method,
         sizeof(tirs_align_cal->corr_fit_method), IAS_ODL_Int, 1},

        {group_name, "Fit_Order", &tirs_align_cal->fit_order,
         sizeof(tirs_align_cal->fit_order), IAS_ODL_Int, 1},

        {group_name, "Tie_Point_Weight", &tirs_align_cal->tie_point_weight,
         sizeof(tirs_align_cal->tie_point_weight), IAS_ODL_Double, 1},

        {group_name, "Along_Postfit_RMSE_Threshold", 
         &tirs_align_cal->along_postfit_rmse_threshold,
         sizeof(tirs_align_cal->along_postfit_rmse_threshold), 
         IAS_ODL_Double, 1},

        {group_name, "Across_Postfit_RMSE_Threshold", 
         &tirs_align_cal->across_postfit_rmse_threshold,
         sizeof(tirs_align_cal->across_postfit_rmse_threshold), 
         IAS_ODL_Double, 1},

        {group_name, "Align_Constraint_Weight", 
         &tirs_align_cal->align_constraint_weight,
         sizeof(tirs_align_cal->align_constraint_weight), IAS_ODL_Double, 1}
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
