/*************************************************************************

NAME:    ias_parm_update

PURPOSE: This routine updates an ODL file by replacing the attribute(s)
         with a new value if one exists, or by adding it if it does not exist.

RETURNS:
Type=int
Value    Description
-----    -----------
SUCCESS  All required items successfully updated in the ODL file
ERROR    Error preparing for update or in actual write to update the ODL file

**************************************************************************/

#include <stdio.h>
#include <limits.h>             /* PATH_MAX definition */
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_odl.h"
#include "ias_parm.h"
#include "ias_parm_private.h"

int ias_parm_update
(
    const char *odl_file_name,      /* I: ODL file name to update */
    IAS_PARM_PARAMETER_DEFINITION **odl_list_ptr,
                                    /* I/O : pointer to list of items to update
                                       from the ODL file */
    int list_length                 /* I: number of items in the list */
)
{
    int status;                     /* function return status */
    IAS_OBJ_DESC *odl_tree = NULL;  /* parsed ODL object */
    IAS_ODL_KEYWORD *keyword = NULL;/* ODL keyword */
    IAS_OBJ_DESC *group_name = NULL;/* ODL Group to populate */
    int list_index;                 /* current list index */
    IAS_PARM_PARAMETER_DEFINITION *pd = NULL;
                                    /* pointer to current item in the list */
    IAS_ODL_TYPE type;              /* type of pd -- array or not array */
    char update_filename[PATH_MAX]; /* update file name */

    /* read the ODL file */
    odl_tree = ias_odl_read_tree(odl_file_name);
    if (odl_tree == NULL)
    {
        IAS_LOG_ERROR("ODL tree is NULL");
        return ERROR;
    }

    /* loop through the list of parameters to update or add */
    for (list_index = 0; list_index < list_length; list_index++)
    {
        /* get a pointer to the current item */
        pd = odl_list_ptr[list_index];

        /* map the IAS type to ODL type */
        status = ias_parm_map_odl_type(pd, &type);
        if (status != SUCCESS) 
        {
            IAS_LOG_ERROR("Mapping type for ODL parameter \"%s\"", pd->name);
            ias_odl_free_tree(odl_tree);
            return ERROR;
        }

        /* find the correct object in the ODL file */
        if ((group_name = ias_odl_find_object_description(odl_tree, "*", NULL,
            NULL, 2)) == NULL)
        {
            IAS_LOG_ERROR("Can't find any groups in file: %s", odl_file_name);
            ias_odl_free_tree(odl_tree);
            return ERROR;
        }

        if ((keyword = ias_odl_find_keyword(odl_tree, pd->name, NULL, 1))
            != NULL)
        {
            /* found the keyword / attribute, now replace it */
            status = ias_odl_replace_field(group_name, pd->name, 
                            type, pd->value_bytes, pd->value.type_void, 
                            pd->count_read);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Replacing attribute %s in ODL file", pd->name);
                ias_odl_free_tree(odl_tree);
                return ERROR;
            }
        }
        else
        {
            /* did not find the keyword / attribute, now add it */
            status = ias_odl_add_field(group_name, pd->name, type,
                pd->value_bytes, pd->value.type_void, pd->count_read);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Adding attribute %s to ODL file", pd->name);
                ias_odl_free_tree(odl_tree);
                return ERROR;
            }
        }

    }

    /* Write the file */
    sprintf(update_filename,"%s%s",odl_file_name,".update");
    ias_odl_write_tree(odl_tree, update_filename);

    ias_odl_free_tree(odl_tree);

    /* move the update file back to the actual file */
    status = rename(update_filename, odl_file_name);
    if (status != 0)
    {
        IAS_LOG_ERROR("Renaming %s to %s", update_filename, odl_file_name);
        return ERROR;
    }

    return SUCCESS;
}


