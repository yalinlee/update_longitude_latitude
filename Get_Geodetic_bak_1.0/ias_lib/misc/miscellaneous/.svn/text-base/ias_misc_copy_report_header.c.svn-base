/***************************************************************************
 NAME:     ias_misc_copy_report_header

 PURPOSE:  Creates a local copy of the standard IAS report header

 RETURNS:  Pointer to an IAS_REPORT_HEADER data structure if successful
           NULL pointer if there's an error
***************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "ias_miscellaneous_private.h"

IAS_REPORT_HEADER *ias_misc_copy_report_header
(
    const IAS_REPORT_HEADER *header_to_copy  /* I: Standard report header
                                                   data to copy */
)
{
    IAS_REPORT_HEADER *header = NULL;

    /* Make sure we have an allocated input report header data block */
    if (header_to_copy == NULL)
    {
        IAS_LOG_ERROR("Input IAS_REPORT_HEADER data block is invalid");
        return NULL;
    }

    /* Allocate the basic report header block */
    header = malloc(sizeof(*header));
    if (header == NULL)
    {
        IAS_LOG_ERROR("Allocating base data block for report header copy");
        return NULL;
    }

    /* Copy the entire data block */
    memcpy(header, header_to_copy, sizeof(IAS_REPORT_HEADER));

    return header;
}   /* END -- ias_misc_copy_report_header */
