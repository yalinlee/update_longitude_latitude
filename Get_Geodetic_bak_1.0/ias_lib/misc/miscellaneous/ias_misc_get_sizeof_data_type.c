#include <string.h>
#include "ias_logging.h"
#include "ias_const.h"
#include "ias_miscellaneous.h"

/*********************************************************************
 NAME:      ias_misc_get_sizeof_data_type

 PURPOSE:   Gets number of bytes for a given IAS data type using sizeof

 RETURNS:   SUCCESS or ERROR
**********************************************************************/
int ias_misc_get_sizeof_data_type
(
    IAS_DATA_TYPE dtype,    /* I: The IAS data type to get number of bytes */
    int *type_nbytes        /* O: Number of bytes for the given type */
)
{
    switch (dtype)
    {
        case IAS_BYTE:
            *type_nbytes = sizeof(IAS_BYTE_TYPE);
            break;
        case IAS_I2:
            *type_nbytes = sizeof(IAS_I2_TYPE);
            break;
        case IAS_UI2:
            *type_nbytes = sizeof(IAS_UI2_TYPE);
            break;
        case IAS_I4:
            *type_nbytes = sizeof(IAS_I4_TYPE);
            break;
        case IAS_UI4:
            *type_nbytes = sizeof(IAS_UI4_TYPE);
            break;
        case IAS_R4:
            *type_nbytes = sizeof(IAS_R4_TYPE);
            break;
        case IAS_R8:
            *type_nbytes = sizeof(IAS_R8_TYPE);
            break;
        case IAS_CHAR:
            *type_nbytes = sizeof(IAS_CHAR_TYPE);
            break;
        case IAS_UCHAR:
            *type_nbytes = sizeof(IAS_UCHAR_TYPE);
            break;
        default:
            IAS_LOG_ERROR("Invalid data type %d", dtype);
            return ERROR;
    }

    return SUCCESS;
}
