/*****************************************************************************
NAME: ias_odl_get_group

PURPOSE:  Retrieve a pointer to a specific group

RETURN VALUE:
    Type = IAS_OBJ_DESC *

Value           Description
--------------- ----------------------------------------------------------------

positive        Pointer to ODL tree
NULL            Failure

NOTES:

******************************************************************************/
#include "lablib3.h"
#include "ias_logging.h"
#include "ias_const.h"
#include "ias_odl.h"

IAS_OBJ_DESC *ias_odl_get_group
(
    IAS_OBJ_DESC *p_ODLTree,     /* I: Parsed ODL object tree */
    const char *p_ClassName      /* I: ODL group name */
)
{
    OBJDESC *p_lp;         /* Object Descriptor */


    /* Validate input parameter */
    if ( p_ClassName == NULL )
    {
        IAS_LOG_ERROR("Invalid Group Name");
        return NULL;
    }

    /* Locate the group in the ODL */
    if ((p_lp = OdlFindObjDesc(p_ODLTree, p_ClassName, NULL, NULL, 1, 
        ODL_RECURSIVE_DOWN)) == NULL)
    {
        IAS_LOG_ERROR("Group %s not found", p_ClassName);
        return NULL;
    }

    return p_lp;
}
