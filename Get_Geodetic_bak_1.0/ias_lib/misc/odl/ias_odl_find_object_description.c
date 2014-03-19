/******************************************************************************
UNIT NAME: ias_odl_find_object_description.c

PURPOSE: Wrapper function for OdlFindObjDesc.

RETURN VALUE:
   Type = IAS_OBJ_DESC *
Value                Description
-----                -----------
*IAS_OBJ_DESC        Pointer to ODL object tree 
NULL                 Failure

******************************************************************************/

#include <sys/param.h>
#include "lablib3.h"                 /* prototypes for the ODL functions */
#include "ias_odl.h"
#include "ias_logging.h"

IAS_OBJ_DESC *ias_odl_find_object_description
(
    IAS_OBJ_DESC *start_object,     /* I: ODL tree to parse */
    const char *object_class,       /* I: class name to search for */
    const char *keyword_name,       /* I: keyword to search for */
    char *keyword_value,            /* I: keyword value to search for */
    unsigned long object_position   /* I: object position to search for */
)
{
    IAS_OBJ_DESC *p_lp= NULL;

    p_lp = OdlFindObjDesc(start_object, object_class, keyword_name,
           keyword_value, object_position, ODL_RECURSIVE_DOWN);

    return p_lp;
}
