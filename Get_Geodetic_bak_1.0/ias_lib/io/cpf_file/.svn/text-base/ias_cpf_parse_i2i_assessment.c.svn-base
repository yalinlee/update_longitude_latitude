/*************************************************************************

NAME: ias_cpf_parse_i2i_assessment

PURPOSE: Retrieve the OLI and TIRS i2i assessment data group from the odl 
         tree and insert into the internal cpf structure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_i2i_assessment
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_I2I_ASSESSMENT *i2i_assess /* O: CPF i2i_assessment data  */
)
{
    int status;                           /* Function return value */
    int count = 0;                        /* number of list buckets */

    char group_name[] = "I2I_ASSESSMENT"; /* Name of group to retrieve */

    IAS_OBJ_DESC *odl_tree;               /* ODL tree */

    /* populate the attribute list */
    ODL_LIST_TYPE list[] =      /* struct to contain group attributes */
    {
        {group_name, "Trend_Threshold_Line", &i2i_assess->trend_threshold_line,
         sizeof(i2i_assess->trend_threshold_line), IAS_ODL_Double, 
         IAS_MAX_NBANDS},

        {group_name, "Trend_Threshold_Sample", 
         &i2i_assess->trend_threshold_sample,
         sizeof(i2i_assess->trend_threshold_sample), IAS_ODL_Double, 
         IAS_MAX_NBANDS},

        {group_name, "Fill_Threshold", &i2i_assess->fill_threshold,
         sizeof(i2i_assess->fill_threshold), IAS_ODL_Double, 1},

        {group_name, "Corr_Pix_Min", &i2i_assess->corr_pix_min,
         sizeof(i2i_assess->corr_pix_min), IAS_ODL_Int, 1},

        {group_name, "Corr_Pix_Max", &i2i_assess->corr_pix_max,
         sizeof(i2i_assess->corr_pix_max), IAS_ODL_Int, 1},

        {group_name, "Corr_Window_Size", &i2i_assess->corr_window_size,
         sizeof(i2i_assess->corr_window_size), IAS_ODL_Int, 2},

        {group_name, "Corr_Fit_Method", &i2i_assess->corr_fit_method,
         sizeof(i2i_assess->corr_fit_method), IAS_ODL_Int, 1},

        {group_name, "Max_Displacement_Offset", 
         &i2i_assess->max_displacement_offset,
         sizeof(i2i_assess->max_displacement_offset), IAS_ODL_Int, 1},

        {group_name, "Min_Corr_Strength", &i2i_assess->min_corr_strength,
         sizeof(i2i_assess->min_corr_strength), IAS_ODL_Double, 1},
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
