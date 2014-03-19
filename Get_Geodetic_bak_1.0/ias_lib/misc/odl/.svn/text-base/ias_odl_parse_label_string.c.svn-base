/******************************************************************************
UNIT NAME: ias_odl_parse_label_string.c

PURPOSE: Wrapper function for OdlParseLabelString.

RETURN VALUE:
   Type = IAS_OBJ_DESC *
Value           Description
-----           -----------
*IAS_OBJ_DESC   Pointer to ODL object tree 
NULL            Failure

******************************************************************************/

#include "lablib3.h"                 /* prototypes for the ODL functions */
#include "ias_odl.h"
#include "ias_logging.h"

IAS_OBJ_DESC *ias_odl_parse_label_string
(
    char *odl_string             /* I: ODL string to parse */
)
{
    OBJDESC *p_lp= NULL;

    /* Initialize a ROOT object to start the ODL tree */
    p_lp = OdlParseLabelString(odl_string, NULL, ODL_NOEXPAND, 1);
    if (!p_lp)
    {
        IAS_LOG_ERROR("Cannot parse label string");
        return NULL;
    }

    return p_lp;
}
