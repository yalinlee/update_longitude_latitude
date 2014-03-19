/******************************************************************************
NAME:           ias_misc_get_software_version

PURPOSE:        
ias_misc_get_software_version returns the current IAS software version

RETURN VALUE:
Type = char *
Value    Description
-----    -----------
pointer  Returns a string pointer to the IAS version number
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ias_miscellaneous.h"
#include "config.h"

#ifdef IAS_VERSION
static const char ias_version[IAS_SOFTWARE_VERSION_SIZE] = IAS_VERSION;
#else
static const char ias_version[IAS_SOFTWARE_VERSION_SIZE] = "Unknown";
#endif
static const char dummy_version[IAS_SOFTWARE_VERSION_SIZE] = "12.34.56";


const char *ias_misc_get_software_version()
{
    const char *ias_version_override;

    ias_version_override = getenv("IAS_VERSION_OVERRIDE");
    if (ias_version_override != NULL && strcmp(ias_version_override, "Y") == 0)
    {
        /* Intended for use on our regression tests (easily recognizable) */
        return dummy_version;
    }
    return ias_version;
}
