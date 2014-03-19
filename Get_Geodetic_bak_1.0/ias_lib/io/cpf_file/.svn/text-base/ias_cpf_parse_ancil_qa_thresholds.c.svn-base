/*************************************************************************

NAME: ias_cpf_parse_ancil_qa_thresholds

PURPOSE: Retrieve the ancillary threshold group from the odl tree and 
         insert into the internal cpf structure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_ancil_qa_thresholds
(
    const IAS_CPF *cpf,                  /* I: CPF structure */
    struct IAS_CPF_ANCILLARY_QA_THRESHOLDS *ancil_qa_thresholds 
                                         /* O: Struct for ancillary data */
)
{
    int status;                                     /* Function return value */
    char group_name[] = "ANCILLARY_QA_THRESHOLDS";  /* group to retrieve */
    int count = 0;                                  /* number of list buckets */
    IAS_OBJ_DESC *odl_tree;                         /* ODL tree */


    /* populate the list of elements to retrieve */
    ODL_LIST_TYPE list[] = 
    {
        
       {group_name, "Orbit_Radius_Tolerance",
       &ancil_qa_thresholds->orbit_radius_tolerance,
       sizeof(ancil_qa_thresholds->orbit_radius_tolerance), 
       IAS_ODL_Double, 1},

       {group_name, "Angular_Momentum_Tolerance", 
       &ancil_qa_thresholds->angular_momentum_tolerance,
       sizeof(ancil_qa_thresholds->angular_momentum_tolerance), 
       IAS_ODL_Double, 1}, 

       {group_name, "Spacecraft_Clock_TAI_Epoch", 
        &ancil_qa_thresholds->spacecraft_clock_tai_epoch,
        sizeof(ancil_qa_thresholds->spacecraft_clock_tai_epoch), 
        IAS_ODL_Double, 1},

       {group_name, "Quaternion_Normalization_Outlier_Threshold", 
        &ancil_qa_thresholds->quaternion_normalization_outlier_threshold,
        sizeof(ancil_qa_thresholds->quaternion_normalization_outlier_threshold),
        IAS_ODL_Double, 1},

       {group_name, "IRU_Outlier_Threshold", 
        ancil_qa_thresholds->iru_outlier_threshold,
        sizeof(ancil_qa_thresholds->iru_outlier_threshold), IAS_ODL_Double, 4}
    };

    /* Calculate the number of attributes to retrieve */
    count = sizeof(list) / sizeof(ODL_LIST_TYPE);

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
       IAS_LOG_ERROR("Getting group: %s from CPF ", group_name);
       DROP_ODL_TREE(odl_tree);
       return ERROR;
    }
    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}
