/******************************************************************************
NAME: ias_odl_write_tree

PURPOSE:  Save the ODL Object Tree to a file

RETURN VALUE: None  

******************************************************************************/

#include "lablib3.h"
#include "ias_odl.h"

void ias_odl_write_tree
(
    IAS_OBJ_DESC *ODLTree,  /* I: ODL Object Tree to be saved */
    char *p_ODLFile         /* I: ODL file name (full directory path) */
)
{
    /* Write the ODL Object Tree to a file */
    OdlPrintLabel(ODLTree, p_ODLFile, NULL, 0);
}
