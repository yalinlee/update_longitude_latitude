/******************************************************************************
UNIT NAME: ias_odl_find_keyword.c

PURPOSE: Wrapper function for OdlFindKwd.

RETURN VALUE:
   Type = IAS_ODL_KEYWORD *
Value                   Description
-----                   -----------
*IAS_ODL_KEYWORD        Pointer to IAS_ODL_KEYWORD object tree 
NULL                    Failure

******************************************************************************/

#include <sys/param.h>
#include "ias_odl.h"
#include "lablib3.h"                 /* prototypes for the ODL functions */
#include "ias_logging.h"

IAS_ODL_KEYWORD *ias_odl_find_keyword
(
    IAS_OBJ_DESC *start_object,     /* I: ODL tree to parse */
    const char *keyword_name,       /* I: keyword to search for */
    char *keyword_value,            /* I: keyword value to search for */
    unsigned long keyword_position  /* I: object position to search for */
)
{
    IAS_ODL_KEYWORD *p_lp = NULL;

    p_lp = OdlFindKwd(start_object, keyword_name, keyword_value,
        keyword_position, ODL_RECURSIVE_DOWN);

    return p_lp;
}
