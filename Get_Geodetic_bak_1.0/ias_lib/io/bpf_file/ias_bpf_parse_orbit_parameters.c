/***************************************************************************
 NAME:                ias_bpf_parse_orbit_parameters

 PURPOSE:  Retrieves the orbit parameter group data from the ODL tree and
           inserts it into the internal BPF data structure

 RETURNS:  Integer status code of SUCCESS or ERROR

****************************************************************************/
#include "ias_bpf.h"
#include "ias_logging.h"
#include "ias_odl.h"
#include "bpf_local_defines.h"


int ias_bpf_parse_orbit_parameters
(
    const IAS_BPF *bpf,                       /* I: BPF data structure */
    struct IAS_BPF_ORBIT_PARAMETERS *orbit_parameters 
                                              /* O: Populated
                                                 BPF_ORBIT_PARAMETERS data
                                                 structure */
)
{
    char group_name[] = "ORBIT_PARAMETERS";
    int status;
    int attribute_count = 0;

    IAS_OBJ_DESC *odl_tree = NULL;            /* ODL "tree" with orbit
                                                 parameter information */

    ODL_LIST_TYPE list[] =
    {
        {group_name, "Orbit_Number", &orbit_parameters->begin_orbit_number,
            sizeof(orbit_parameters->begin_orbit_number), IAS_ODL_Int, 1},
    };

    attribute_count = sizeof(list) / sizeof(ODL_LIST_TYPE);

    /* Pull the ORBIT_PARAMETERS group information for the current band */
    GET_BPF_GROUP_FROM_CACHE(bpf, group_name, odl_tree);

    /* Populate the list from the ODL tree */
    status = ias_odl_get_field_list(odl_tree, list, attribute_count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Cannot retrieve ORBIT_PARAMETERS group data ");
        DROP_BPF_ODL_TREE(odl_tree);
        return ERROR;
    }

    /* Release the ODL tree memory */
    DROP_BPF_ODL_TREE(odl_tree);


    /* Done */
    return SUCCESS;
}
