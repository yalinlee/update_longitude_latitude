/*************************************************************************

NAME: ias_cpf_parse_los_model_correction

PURPOSE: Retrieve the los model correction parameters data group from the 
         odl tree and insert into the internal cpf strucure.
        
RETURN VALUE: SUCCESS or FAILURE

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_los_model_correction
(
    const IAS_CPF *cpf,             /* I: CPF structure */
    struct IAS_CPF_LOS_MODEL_CORRECTION *los_model_correction
)                                   /* O: Structure of los model Parameters */
{
    int status;           /* Status of return from function */
    int count = 0;        /* Number of items in CPF_LIST_TYPE */
    char group_name[] = "LOS_MODEL_CORRECTION"; 
                          /* Group to retrieve from the CPF */
    IAS_OBJ_DESC *odl_tree;    /* ODL tree */

    /* populate the list with the attributes to retrieve */
    ODL_LIST_TYPE list[] = 
    {
        {group_name, "Attitude_Apri", los_model_correction->attitude_apri,
         sizeof(los_model_correction->attitude_apri), IAS_ODL_Double,
         IAS_CPF_LOS_ATTITUDE_APRI_COUNT},

        {group_name, "Ephemeris_Apri", los_model_correction->ephemeris_apri,
         sizeof(los_model_correction->ephemeris_apri), IAS_ODL_Double,
         IAS_CPF_LOS_EPHEMERIS_APRI_COUNT},

        {group_name, "DOQ_Observation_Apri",
         los_model_correction->doq_observation_apri,
         sizeof(los_model_correction->doq_observation_apri), IAS_ODL_Double,
         IAS_CPF_LOS_OBSERVATION_APRI_COUNT},

        {group_name, "GLS_Observation_Apri",
         los_model_correction->gls_observation_apri,
         sizeof(los_model_correction->gls_observation_apri), IAS_ODL_Double,
         IAS_CPF_LOS_OBSERVATION_APRI_COUNT}
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
