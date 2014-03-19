/******************************************************************************
NAME: ias_odl_add_group

PURPOSE:  Add the specified ODL group to the ODL object tree

RETURN VALUE: NULL or pointer to OBJDESC structure

******************************************************************************/

#include "lablib3.h"
#include "ias_odl.h"
#include "ias_logging.h"

IAS_OBJ_DESC *ias_odl_add_group
(
    IAS_OBJ_DESC *ODLTree,      /* I/O: Object tree to populate */
    const char *p_ClassName     /* I: Group/Object name */
)
{
    OBJDESC *new_object;         /* Pointer to a new ODL object */
    OBJDESC *curr_object;        /* Pointer to current ODL object */

    /* Check to make sure a valid Class Name was specified */
    if ((p_ClassName == NULL) || (strlen(p_ClassName) == 0))
    {
        IAS_LOG_ERROR("Invalid Class Name");
        return NULL;
    }

    /* Create the new ODL Object for the group */
    new_object = OdlNewObjDesc(p_ClassName, NULL, NULL, NULL, NULL, NULL,
                               ODL_GROUP, 0);
    if (!new_object)
    {
        IAS_LOG_ERROR("Unable to add new ODL Group");
        return NULL;
    } 

    /* Add the new object to the parent object */
    curr_object = OdlPasteObjDesc(new_object, ODLTree);
    if (!curr_object)
    {
        IAS_LOG_ERROR("Unable to add ODL Group");
        return NULL;
    } 

    return curr_object;
}
