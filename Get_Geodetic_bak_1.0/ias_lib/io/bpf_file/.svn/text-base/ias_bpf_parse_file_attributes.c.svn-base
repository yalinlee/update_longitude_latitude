/*************************************************************************
 NAME:                 ias_bpf_parse_file_attributes

 PURPOSE:  Retrieve the FILE_ATTRIBUTES data group from the odl tree and
           insert into the internal BPF data structure

 RETURNS:  Integer status code of SUCCESS or ERROR

**************************************************************************/
#include "ias_bpf.h"
#include "ias_logging.h"
#include "bpf_local_defines.h"


int ias_bpf_parse_file_attributes
(
    const IAS_BPF *bpf,                      /* I: BPF data structure */
    struct IAS_BPF_FILE_ATTRIBUTES *file_attributes
                                             /* O: Populated file attributes
                                                data structure */
)
{
    char group_name[] = "FILE_ATTRIBUTES";

    int status;                      /* Function return value */
    int attribute_count = 0;         /* Number of group attributes */

    IAS_OBJ_DESC *odl_tree = NULL;   /* ODL tree */

    ODL_LIST_TYPE list[] =

    {
        {group_name, "Effective_Date_Begin",
            &file_attributes->effective_date_begin,
            sizeof(file_attributes->effective_date_begin),
            IAS_ODL_String, 1},

        {group_name, "Effective_Date_End",
            &file_attributes->effective_date_end,
            sizeof(file_attributes->effective_date_end),
            IAS_ODL_String, 1},

        {group_name, "Baseline_Date", &file_attributes->baseline_date,
         sizeof(file_attributes->baseline_date), IAS_ODL_String, 1},

        {group_name, "File_Name", &file_attributes->file_name,
         sizeof(file_attributes->file_name), IAS_ODL_String, 1},

        {group_name, "File_Source", &file_attributes->file_source,
         sizeof(file_attributes->file_source), IAS_ODL_String, 1},

        {group_name, "Spacecraft_Name", &file_attributes->spacecraft_name,
         sizeof(file_attributes->spacecraft_name), IAS_ODL_String, 1},

        {group_name, "Sensor_Name", &file_attributes->sensor_name,
         sizeof(file_attributes->sensor_name), IAS_ODL_String, 1},

        {group_name, "Description", &file_attributes->description,
         sizeof(file_attributes->description), IAS_ODL_String, 1},

        {group_name, "Version", &file_attributes->version,
         sizeof(file_attributes->version), IAS_ODL_Int, 1},
    };

    attribute_count = sizeof(list) / sizeof(ODL_LIST_TYPE);

    /* Pull the BPF_FILE_ATTRIBUTES information */
    GET_BPF_GROUP_FROM_CACHE(bpf, group_name, odl_tree);

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, attribute_count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Parsing file attributes group from ODL tree");
        DROP_BPF_ODL_TREE(odl_tree);
        return ERROR;
    }

    /* Free up allocated memory for the ODL tree */
    DROP_BPF_ODL_TREE(odl_tree);

    /* Done */
    return SUCCESS;
}
