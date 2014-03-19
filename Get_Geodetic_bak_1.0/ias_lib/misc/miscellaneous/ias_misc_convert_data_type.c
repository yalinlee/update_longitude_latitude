#include <string.h>
#include "ias_logging.h"
#include "ias_const.h"
#include "ias_miscellaneous.h"

/*********************************************************************
 NAME:      ias_misc_convert_data_type_to_string

 PURPOSE:   Converts a given IAS_DATA_TYPE to a string representation

 RETURNS:   SUCCESS or ERROR
**********************************************************************/
int ias_misc_convert_data_type_to_string
(
    IAS_DATA_TYPE data_type,        /* I: IAS data type */
    const char **data_type_string   /* O: The data type converted to a string */
)
{
    const char *type_string;

    switch (data_type)
    {
        case IAS_BYTE:
            type_string = "BYTE";
            break;
        case IAS_I2:
            type_string = "I*2";
            break;
        case IAS_UI2:
            type_string = "UI*2";
            break;
        case IAS_I4:
            type_string = "I*4";
            break;
        case IAS_UI4:
            type_string = "UI*4";
            break;
        case IAS_R4:
            type_string = "R*4";
            break;
        case IAS_R8:
            type_string = "R*8";
            break;
        case IAS_CHAR:
            type_string = "CHAR";
            break;
        case IAS_UCHAR:
            type_string = "UCHAR";
            break;
        default:
            IAS_LOG_ERROR("Invalid data type %d", data_type);
            return ERROR;
    }

    *data_type_string = type_string;

    return SUCCESS;
}

/*********************************************************************
 NAME:      ias_misc_convert_string_to_data_type

 PURPOSE:   Converts a given string representation of an IAS data type
            to the IAS_DATA_TYPE equivalent.

 RETURNS:   SUCCESS or ERROR
**********************************************************************/
int ias_misc_convert_string_to_data_type
(
    const char *data_type_string,   /* I: The string data type */
    IAS_DATA_TYPE *data_type        /* O: The string to its IAS data type */
)
{
    IAS_DATA_TYPE local_data_type;

    if (strcasecmp(data_type_string,"BYTE") == 0)
        local_data_type = IAS_BYTE;
    else if (strcasecmp(data_type_string,"I*2") == 0)
        local_data_type = IAS_I2;
    else if (strcasecmp(data_type_string,"UI*2") == 0)
        local_data_type = IAS_UI2;
    else if (strcasecmp(data_type_string,"I*4") == 0)
        local_data_type = IAS_I4;
    else if (strcasecmp(data_type_string,"UI*4") == 0)
        local_data_type = IAS_UI4;
    else if (strcasecmp(data_type_string,"R*4") == 0)
        local_data_type = IAS_R4;
    else if (strcasecmp(data_type_string,"R*8") == 0)
        local_data_type = IAS_R8;
    else if (strcasecmp(data_type_string,"CHAR") == 0)
        local_data_type = IAS_CHAR;
    else if (strcasecmp(data_type_string,"UCHAR") == 0)
        local_data_type = IAS_UCHAR;
    else
    {
        IAS_LOG_ERROR("Invalid data type %s", data_type_string);
        return ERROR;
    }

    *data_type = local_data_type;

    return SUCCESS;
}
