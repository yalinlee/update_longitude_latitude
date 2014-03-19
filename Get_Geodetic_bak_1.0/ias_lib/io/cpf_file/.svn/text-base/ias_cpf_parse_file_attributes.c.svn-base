/*************************************************************************

NAME: ias_cpf_parse_file_attributes

PURPOSE: Retrieve the file attribute data group from the odl tree and insert 
         into internal cpf structure
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_file_attributes
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_FILE_ATTRIBUTES *file_atts /* O: file attributes data */
)
{
    int status;                      /* Function return value */
    char group_name[] = "FILE_ATTRIBUTES"; /* Name of group to retrieve */
    int count;                       /* Number of group attributes */
    IAS_OBJ_DESC *odl_tree;          /* ODL tree */

    ODL_LIST_TYPE list[] =       /* Struct to contain group attributes */
    {
        {group_name, "Effective_Date_Begin", &file_atts->effective_date_begin,
         sizeof(file_atts->effective_date_begin), IAS_ODL_String, 1},

        {group_name, "Effective_Date_End", &file_atts->effective_date_end,
         sizeof(file_atts->effective_date_end), IAS_ODL_String, 1},

        {group_name, "Baseline_Date", &file_atts->baseline_date,
         sizeof(file_atts->baseline_date), IAS_ODL_String, 1},

        {group_name, "File_Name", &file_atts->file_name,
         sizeof(file_atts->file_name), IAS_ODL_String, 1},

        {group_name, "File_Source", &file_atts->file_source,
         sizeof(file_atts->file_source), IAS_ODL_String, 1},

        {group_name, "Spacecraft_Name", &file_atts->spacecraft_name,
         sizeof(file_atts->spacecraft_name), IAS_ODL_String, 1},

        {group_name, "Sensor_Name", &file_atts->sensor_name,
         sizeof(file_atts->sensor_name), IAS_ODL_String, 1},

        {group_name, "Description", &file_atts->description,
         sizeof(file_atts->description), IAS_ODL_String, 1},

        {group_name, "Version", &file_atts->version,
         sizeof(file_atts->version), IAS_ODL_Int,
         sizeof(file_atts->version) / sizeof(int)}
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
