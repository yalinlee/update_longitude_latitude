/**************************************************************************
 NAME:                    ias_misc_free_report_header

 PURPOSE:  Frees memory block internally allocated for the IAS_REPORT_HEADER
           data structure

 RETURNS:  Nothing
***************************************************************************/
#include <stdlib.h>
#include "ias_miscellaneous.h"

void ias_misc_free_report_header
(
    IAS_REPORT_HEADER *header   /* I: Pointer to header memory block */
)
{
    /* First, verify the header pointer is valid */
    if (header == NULL)
        return;

    /* Free the memory block */
    free(header);
}
