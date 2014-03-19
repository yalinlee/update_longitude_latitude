/******************************************************************************
UNIT NAME: ias_odl_parse_file.c

PURPOSE: Parse a label file and return a pointer to the root object of the
         tree. Wrapper for OdlParseFile function.

RETURN VALUE:
   Type = OBJDESC *
Value           Description
-----           -----------
*IAS_OBJ_DESC   Pointer to ODL object tree 
NULL            Failure

******************************************************************************/

#include "lablib3.h"                 /* prototypes for the ODL functions */
#include "ias_odl.h"
#include "ias_logging.h"

IAS_OBJ_DESC *ias_odl_parse_file
(
    char *label_fname,                 /* I: File name to read */
    FILE *label_fptr                   /* I: File pointer to read */
)
{
    OBJDESC *p_lp= NULL;

    /* Initialize a ROOT object to start the ODL tree */
    p_lp = OdlParseFile(label_fname, label_fptr, NULL, NULL,
            TRUE, 1, 1, 1);
    if (!p_lp)
    {
        IAS_LOG_ERROR("Cannot parse file");
        return NULL;
    }

    return p_lp;
}
