/******************************************************************************
UNIT NAME: ias_odl_create_tree.c

PURPOSE: Create a Root ODL Object Tree 

RETURN VALUE:
   Type = OBJDESC *
Value           Description
-----           -----------
*IAS_OBJ_DESC   Pointer to ODL object tree 
NULL            Failure

******************************************************************************/

#include "lablib3.h"                 /* prototypes for the ODL functions */
#include "ias_odl.h"
#include "ias_logging.h"

IAS_OBJ_DESC *ias_odl_create_tree
(
)
{
    OBJDESC *p_lp= NULL;

    /* Initialize a ROOT object to start the ODL tree */
    p_lp = OdlNewObjDesc("ROOT", NULL, NULL, NULL, NULL, NULL, 0, 0);
    if (!p_lp)
    {
        IAS_LOG_ERROR("Cannot Create the Root ODL Object");
        return NULL;
    }

    return p_lp;
}
