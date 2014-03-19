/******************************************************************************
PROGRAM: ias_geo_get_units

PURPOSE: Get the unit code from the name of the units. Takes a string and 
         returns the int value defined in gctp.h. The unit code returned
         from this function can be entered into an IAS_PROJECTION.

******************************************************************************/
#include <string.h>
#include "ias_miscellaneous.h"
#include "ias_logging.h"
#include "ias_const.h"
#include "gctp.h"
#include "ias_geo.h"


int ias_geo_get_units 
(
    const char *unit_name,  /* I: Units name */
    int *unit_num           /* O: Units number */
)
{
    char units[30];
 
    /* ensure that the unit name is upper case, compare names and assign the
       code number appropriately */
    strncpy(units, unit_name, sizeof(units));
    units[29] = '\0';
    ias_misc_convert_to_uppercase(units);
    if(strcmp(units, "RADIANS") == 0)
        *unit_num = RADIAN;
    else if(strcmp(units, "FEET") == 0) 
        *unit_num = FEET;
    else if(strcmp(units, "METERS") == 0) 
        *unit_num = METER;
    else if(strcmp(units, "SECONDS") == 0) 
        *unit_num = SECOND;
    else if(strcmp(units, "DEGREES") == 0) 
        *unit_num = DEGREE;
    else if(strcmp(units, "DMS") == 0) 
        *unit_num = DMS;
    else
    {
        IAS_LOG_ERROR("Illegal projection units for gctp: Valid units include "
                      "RADIANS, FEET, METERS, SECONDS, DEGREES, and DMS");
        return ERROR;
    }

    return SUCCESS;
}

