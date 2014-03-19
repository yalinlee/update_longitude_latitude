/*************************************************************************

NAME: ias_cpf_parse_histogram_characterization

PURPOSE: Retrieve the histogram characterization data group from the 
         odl tree and insert into the internal cpf strucure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_histogram_characterization
(
    const IAS_CPF *cpf,                           /* I: CPF structure */
    struct IAS_CPF_HISTOGRAM_CHAR *histogram_char /* O: CPF sensor params */
)
{
    int status;                     /* Status of return from function */
    int count = 0;                  /* Number of items in CPF_LIST_TYPE */
    char group_name[] = "HISTOGRAM_CHARACTERIZATION"; 
                                    /* Group to retrieve from the CPF */

    IAS_OBJ_DESC *odl_tree;         /* ODL tree */

    ODL_LIST_TYPE list[] =          /* Struct to contain group attributes */
    {
        {group_name, "Frames_To_Skip_Top", &histogram_char->frames_to_skip_top,
         sizeof(histogram_char->frames_to_skip_top), IAS_ODL_Int, 1},

        {group_name, "Frames_To_Skip_Bottom", 
         &histogram_char->frames_to_skip_bottom,
         sizeof(histogram_char->frames_to_skip_bottom), IAS_ODL_Int, 1}
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
