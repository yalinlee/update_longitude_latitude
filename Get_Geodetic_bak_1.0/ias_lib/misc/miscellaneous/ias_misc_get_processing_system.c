/******************************************************************************
NAME:           ias_misc_get_processing_system

PURPOSE:        gets the software processing system (IAS or LPGS)
******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "ias_miscellaneous.h"

IAS_PROCESSING_SYSTEM_TYPE ias_misc_get_processing_system()
{
    char *env_var;        /* String pointer */

    env_var = getenv("PROCESSING_SYSTEM");

    if (!env_var)
    {
        /* PROCESSING_SYSTEM not set */
        return IAS_PROCESSING_SYSTEM_UNDEF;
    }

    if (strncmp(env_var, "IAS", 3) == 0)
    {
        return IAS_PROCESSING_SYSTEM_IAS;
    }

    if (strncmp(env_var, "LPGS", 4) == 0)
    {
        return IAS_PROCESSING_SYSTEM_LPGS;
    }
    else
    {
        return IAS_PROCESSING_SYSTEM_UNDEF;
    }
}
