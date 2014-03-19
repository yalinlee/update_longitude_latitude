/*****************************************************************************
NAME: ias_odl_add_field_list

PURPOSE:  Add the group and list of attributes to the ODL tree

RETURN VALUE: SUCCESS or ERROR

******************************************************************************/
#include "lablib3.h"
#include "ias_odl.h"
#include "ias_odl_private.h"
#include "ias_logging.h"

int ias_odl_add_field_list
(
    IAS_OBJ_DESC *ODLTree,     /* I/O: ODL object tree */
    ODL_LIST_TYPE *list_parms, /* I: List of ODL attribute to add */
    const int count            /* I: Number of items in ODL_LIST_TYPE */
)
{
    OBJDESC *ODLGroup;    /* ODL Group to populate */
    int i;                /* Loop counter */
    int status;           /* Status of return from function */

    /* Create a new group in the ODL Object Tree */
    ODLGroup = ias_odl_add_group(ODLTree, list_parms[0].group_name);
    if (ODLGroup == NULL)
    {
        IAS_LOG_ERROR("Adding group %s to ODL tree", 
        list_parms[0].group_name);
        return(ERROR);
    }

    /* Add the attributes to the ODL */
    for (i = 0; i < count; i++)
    {
        status = ias_odl_add_field(ODLGroup, list_parms[i].attribute, 
                           list_parms[i].parm_type, list_parms[i].parm_size, 
                           list_parms[i].parm_ptr, list_parms[i].parm_count);
        if (status != SUCCESS) 
        {
            IAS_LOG_ERROR("Adding attribute %s to the ODL tree",
                    list_parms[i].attribute);
            return(ERROR);
        }
    }

    return(SUCCESS);
}
