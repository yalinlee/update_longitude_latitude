/******************************************************************************
NAME: ias_odl_replace_field

PURPOSE:  Replace the specified ODL field to the ODL object tree
          if the input parameter replace is true the attribute name will 
          be replaced, if false will add the attribute.


RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "lablib3.h"
#include "ias_odl.h"
#include "ias_odl_private.h"
#include "ias_logging.h"

extern char ODLErrorMessage[];       /* External Variables */

int ias_odl_replace_field
(
    IAS_OBJ_DESC *curr_object,  /* I/O: Object tree to populate */
    const char *p_LabelName,    /* I: Field to add */
    IAS_ODL_TYPE ValueType,     /* I: What type the field is */
    const int p_MemorySize,     /* I: Total memory size of attribues values */
    void *p_MemoryAddr,         /* I: Pointer to the attribute information */
    const int nelements         /* I: Number of attribute values */
)
{
    int status;                 /* function return value */

    status = ias_odl_add_or_replace_field(curr_object, p_LabelName, ValueType,
                                          p_MemorySize, p_MemoryAddr, nelements,
                                          IAS_ODL_REPLACE);

    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Replace field failed");
        return ERROR;
    }
    return SUCCESS;
}
