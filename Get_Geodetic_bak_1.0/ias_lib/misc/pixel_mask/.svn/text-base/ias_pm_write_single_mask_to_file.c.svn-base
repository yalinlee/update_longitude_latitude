/****************************************************************************
NAME:       ias_pm_write_single_mask_to_file

PURPOSE:    Writes a single pixel mask to a pixel mask file.  Refer to
            the function prolog in 'ias_pm_open_pixel_mask.c' for additional
            information regarding the contents and overall layout of the pixel
            mask file.

RETURNS:    Integer status code of SUCCESS or ERROR
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "ias_pixel_mask.h"
#include "pm_local.h"
#include "ias_logging.h"
#include "ias_const.h"


int ias_pm_write_single_mask_to_file
(
    IAS_PIXEL_MASK_IO *pm_file,     /* I: Open pixel mask file */
    IAS_PIXEL_MASK *mask            /* I: Pixel mask structure for a
                                       single band/SCA */
)
{
    int status;


    /* Make sure we have an "open" pixel mask file. */
    if (pm_file == NULL)
    {
        IAS_LOG_ERROR("NULL pointer returned for pixel mask file");
        return ERROR;
    }
    
    /* Make sure we've opened the file with write-only access. */
    if (pm_file->access_mode == IAS_READ)
    {
        IAS_LOG_ERROR("Attempting to write to a pixel mask file opened "
            "with read-only access");
        return ERROR;
    }

    /* Write the mask data to the file. */
    status = ias_pm_write_mask_data_to_file(pm_file, mask);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Writing mask data for band number %d SCA number %d",
            mask->band, mask->sca);
        return ERROR;
    }

    /* Write the end-of-mask marker to the file. */
    status = ias_pm_write_marker_to_file(pm_file, IAS_PM_EOM);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Writing end-of-mask marker to pixel mask file");
        return ERROR;
    }

    return SUCCESS;
}   /* END -- ias_pm_write_single_mask_to_file */
