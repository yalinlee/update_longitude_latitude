/*****************************************************************************
NAME: ias_odl_get_field_list

PURPOSE:  Retrieve the list of attributes (ODL fields)

RETURN VALUE:
Type = int
Value    Description
-----    -----------
  0      Successful completion (SUCCESS)
  1      Operation failed (ERROR)

NOTES:
This function assumes that the file has been opened already.
This is a convenience routine for retrieving a number of ODL fields at one
time, when you know all the field names.

******************************************************************************/
#include "lablib3.h"
#include "ias_logging.h"
#include "ias_const.h"
#include "ias_odl.h"

int ias_odl_get_field_list
(
    IAS_OBJ_DESC *p_ODLTree,   /* I: Parsed ODL object tree */
    ODL_LIST_TYPE *p_ListParms,/* I/O: List of ODL attributes to retrieve */
    const int Count            /* I: Number of items in ODL_LIST_TYPE */
)
{
    int status;           /* Status of return from function */
    int i;                /* Loop counter */
    int nelements;        /* Number of returned attribute values */

    /* Retrieve the attributes in the list */
    for (i = 0; i < Count; i++)
    {
        status = ias_odl_get_field(p_ListParms[i].parm_ptr,
                 p_ListParms[i].parm_size, p_ListParms[i].parm_type, 
                 p_ODLTree, p_ListParms[i].group_name, p_ListParms[i].attribute,
                 &nelements);
        if (status != SUCCESS) 
        {
            IAS_LOG_ERROR("Retrieving %s from the ODL",
                    p_ListParms[i].attribute);
            return ERROR;
        }

        /* Verify the correct number of parameters was read */
        if (p_ListParms[i].parm_count != nelements)
        {
            /* Note that this will likely result in leaking memory in this
               error case since the caller won't know for sure that it should
               free pointers to string arrays, but it is a small price to
               pay for an error that will rarely happen. */
            IAS_LOG_ERROR("Expected %d values for group %s, parameter %s, "
                "but read %d", p_ListParms[i].parm_count,
                p_ListParms[i].group_name,  p_ListParms[i].attribute,
                nelements);
            return ERROR;
        }
    }

    return SUCCESS;
}
