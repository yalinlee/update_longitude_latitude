/*----------------------------------------------------------------------------
NAME:    ias_pm_write_array_to_file

PURPOSE: Converts an array of pixel masks (likely for multiple SCAs and/or
         bands) to a format suitable for writing to a file and writes it to
         the filename provided.  Refer to the function prolog in
         'ias_pm_open_pixel_mask.c' for an explanation of the pixel mask
         file contents and layout.

RETURNS: Integer status code of SUCCESS or ERROR
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include "ias_pixel_mask.h"
#include "pm_local.h"
#include "ias_logging.h"
#include "ias_const.h"
#include "ias_types.h"


int ias_pm_write_array_to_file
(
    IAS_PIXEL_MASK *pixel_mask_array[],
                                        /* I: Array of pixel masks */
    int num_of_masks,                   /* I: Number of pixel masks in array */
    const char *output_file_name        /* I: File name of output */
)
{
    IAS_PIXEL_MASK_IO *pm_file = NULL; /* Open pixel mask file */
    int index;                         /* local loop counter */
    int status;


    /* Open a pixel mask file with write-only access. */
    pm_file = ias_pm_open_pixel_mask(output_file_name, IAS_WRITE);
    if (pm_file == NULL)
    {
        IAS_LOG_ERROR("Creating pixel mask file %s", output_file_name);
        return ERROR;
    }

    /* Loop through all the pixel masks in the array. */
    for ( index = 0; index < num_of_masks; index++ )
    {
        /* Get the current pixel mask */
        IAS_PIXEL_MASK *mask = pixel_mask_array[index];

        /* Write the current mask to the file.  This step also writes out
           the end-of-mask marker afterwards.  */
        status = ias_pm_write_single_mask_to_file(pm_file, mask);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Writing band number %d SCA number %d mask to file",
                mask->band, mask->sca);
            ias_pm_close_pixel_mask(pm_file);
            return ERROR;
        }
    }   /* END for loop */

    /* Close the pixel mask file. This step will write out the available
       housekeeping data before actually closing the file.  */
    status = ias_pm_close_pixel_mask(pm_file);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Closing output file %s", output_file_name);
        return ERROR;
    }

    return SUCCESS;
}   /* END ias_pm_write_array_to_file */
