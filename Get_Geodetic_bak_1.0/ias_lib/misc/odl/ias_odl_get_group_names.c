/******************************************************************************

UNIT NAME: ias_odl_get_group_names.c

PURPOSE: Get the names of the groups in an ODL tree

RETURN VALUE:
    Type = int

Value                               Description
--------------                      ----------------------------------------
SUCCESS                             Found and converted field(s) into 
                                        requested type
IAS_ODL_NOT_FOUND                   Group/label not found
ERROR                               Fatal error                       

NOTES:
This function allocates memory for an array of strings to hold the 
group (class) names.  This is done since the caller probably won't 
know ahead of time how many group names there are, or how long each
one is.  The number of strings (group names) is returned in p_Count.

The caller is responsible for freeing the memory.

******************************************************************************/
#include "toolbox.h"
#include "lablib3.h"
#include "ias_odl.h"
#include "ias_logging.h"

int ias_odl_get_group_names
(
    IAS_OBJ_DESC *p_ODLTree,  /* I: ODL Object Tree to parse */
    char ***p_ClassNames,     /* O: ODL Group/Object names */
    int *p_Count              /* O: Number of group names returned */
)
{

    OBJDESC *p_lp;          /* Object Descriptor */
    int i, pos;             /* loop counters */
    char **p_tmp;           /* temporary class names pointer */

    *p_ClassNames = NULL;   /* pointer to list of names to retrieve */
    *p_Count = OdlGetObjDescChildCount(p_ODLTree);
                            /* number of objects in tree*/

    if (*p_Count == 0)
    {
        /* No groups found */
        return IAS_ODL_NOT_FOUND;
    }


    *p_ClassNames = (char **)malloc( *p_Count * sizeof(char *) );
    if (*p_ClassNames == NULL) 
    {
        *p_Count = 0;
        IAS_LOG_ERROR("Allocating Memory for group names");
        return ERROR;
    }
    p_tmp = *p_ClassNames; /* convenience */

    /* The ODL search routine position parameter is 1-relative, and the 
       matching object in position 1 is the root (current) object, so start 
       with position 2 */
    for (i=0, pos=2; i < *p_Count; i++, pos++)
    {
        if ((p_lp = OdlFindObjDesc(p_ODLTree, "*", NULL, NULL, pos, 
        ODL_CHILDREN_ONLY)) == NULL)
    {
            for (i = 0; i < *p_Count; i++)
                free(*p_ClassNames);

        *p_Count = 0;
        IAS_LOG_ERROR("Getting name of group #%d", i+1);
        return ERROR;
    }

    CopyString(p_tmp[i], OdlGetObjDescClassName(p_lp));
    }

    return SUCCESS;
}
