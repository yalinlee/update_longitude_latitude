/*************************************************************************

NAME: ias_cpf_parse_b2b_assessment

PURPOSE: Retrieve the oli and tirs b2b assessment data groups from the odl tree 
         and insert into the b2b assessment data structure of the cpf
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

#define NUMBER_ATTRIBUTES 7
#define NUMBER_BAND_ATTRIBUTES 2

int ias_cpf_parse_b2b_assessment
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_B2B_ASSESSMENT *b2b_assess /* O: CPF b2b_assessment data */
)
{
    int status;                           /* Function return value */
    int band_index;                       /* band loop var */
    int band_number;                      /* Actual band number */
    int normal_bands;                     /* total normal bands of satellite */
    int nscas;                            /* total number scas */
    int count = 0;                        /* number of list buckets */

    char group_name[] = "B2B_ASSESSMENT"; /* group to retrieve */

    IAS_OBJ_DESC *odl_tree;               /* ODL tree */

    /* get total satellite band count */
    normal_bands = ias_sat_attr_get_normal_band_count();
    if (normal_bands == ERROR)
    {
        IAS_LOG_ERROR("Getting sat normal band count");
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[normal_bands * NUMBER_BAND_ATTRIBUTES 
                + NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[normal_bands * NUMBER_BAND_ATTRIBUTES 
                      + NUMBER_ATTRIBUTES];

    /* Loop through the bands */
    for (band_index = 0; band_index < normal_bands; band_index++)
    {
        band_number = ias_sat_attr_convert_band_index_to_number(band_index);
        if (band_number == ERROR)
        {
        IAS_LOG_ERROR("Getting sat band number from band index");
        return ERROR;
        }

        /* get the number of scas */
        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting sat sca count");
            return ERROR;
        }

        /* Add the band and sca information to the CPF attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]), 
            "Trend_Threshold_Line_B%02d", band_number);
        if (status < 0 || status >= sizeof(attribute[count]))
        { 
            IAS_LOG_ERROR("Creating line attribute string");
            return ERROR;
        }

        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr = (void *) &b2b_assess
                        ->trend_threshold_line[band_index];
        list[count].parm_size = nscas * sizeof(double);
        list[count].parm_type = IAS_ODL_Double;
        list[count].parm_count = nscas;
        count++;

        /* Add the band and sca information to the CPF attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]), 
                 "Trend_Threshold_Sample_B%02d", band_number);
        if (status < 0 || status >= sizeof(attribute[count]))
        { 
            IAS_LOG_ERROR("Creating sample attribute string");
            return ERROR;
        }

        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr = (void *) &b2b_assess
                            ->trend_threshold_sample[band_index];
        list[count].parm_size = nscas * sizeof(double);
        list[count].parm_type = IAS_ODL_Double;
        list[count].parm_count = nscas;
        count++;
    }

    /* Retrieve b2b assessment data group from odl tree */
    list[count].group_name = group_name;
    list[count].attribute = "Corr_Window_Size";
    list[count].parm_ptr = (void *) &b2b_assess->corr_window_size;
    list[count].parm_size = sizeof(int) * 2;
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = 2;
    count++;

    list[count].group_name = group_name;
    list[count].attribute = "Corr_Pix_Min";
    list[count].parm_ptr = (void *) &b2b_assess->corr_pix_min;
    list[count].parm_size = sizeof(int);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = 1;
    count++;

    list[count].group_name = group_name;
    list[count].attribute = "Corr_Pix_Max";
    list[count].parm_ptr = (void *) &b2b_assess->corr_pix_max;
    list[count].parm_size = sizeof(int);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = 1;
    count++;

    list[count].group_name = group_name;
    list[count].attribute = "Fill_Threshold";
    list[count].parm_ptr = (void *) &b2b_assess->fill_threshold;
    list[count].parm_size = sizeof(double);
    list[count].parm_type = IAS_ODL_Double;
    list[count].parm_count = 1;
    count++;

    list[count].group_name = group_name;
    list[count].attribute = "Corr_Fit_Method";
    list[count].parm_ptr = (void *) &b2b_assess->corr_fit_method;
    list[count].parm_size = sizeof(int);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = 1;
    count++;

    list[count].group_name = group_name;
    list[count].attribute = "Max_Displacement_Offset";
    list[count].parm_ptr = (void *) &b2b_assess->max_displacement_offset;
    list[count].parm_size = sizeof(double);
    list[count].parm_type = IAS_ODL_Double;
    list[count].parm_count = 1;
    count++;

    list[count].group_name = group_name;
    list[count].attribute = "Min_Corr_Strength";
    list[count].parm_ptr = (void *) &b2b_assess->min_corr_strength;
    list[count].parm_size = sizeof(double);
    list[count].parm_type = IAS_ODL_Double;
    list[count].parm_count = 1;
    count++;

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* make a sanity check of number of parameters to retrieve */
    if ((normal_bands * NUMBER_BAND_ATTRIBUTES + NUMBER_ATTRIBUTES) != count)
    {
        IAS_LOG_ERROR("Number of parameters does not match number to retrieve");
        return ERROR;
    }

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name);
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);
    
    return SUCCESS;
}
