/*****************************************************************************
NAME:    ias_pm_read_single_mask_from_file

PURPOSE: Reads a single band/SCA mask from the pixel mask file that was
         written by the 'ias_pm_write_single_mask_to_file' or
         'ias_pm_write_array_to_file routines'.  Refer to the function
          prolog of 'ias_pm_open_pixel_mask.c' for an explanation of the
         pixel mask file contents and layout.

RETURNS: Pointer to the single band/SCA mask read from the file if
         successful, NULL if there is an error.
*****************************************************************************/
#include "pm_local.h"
#include "ias_pixel_mask.h"
#include "ias_logging.h"
#include "ias_const.h"


IAS_PIXEL_MASK *ias_pm_read_single_mask_from_file
(
    const IAS_PIXEL_MASK_IO *pm_file,  /* I: Open pixel mask file */
    int band_number,                   /* I: Band number of requested mask */
    int sca_number                     /* I: SCA number of requested mask */
)
{
    IAS_PIXEL_MASK *pm = NULL;         /* Pixel mask structure to allocate
                                          and populate */
    int status;


    /* Read the mask.  At the end of this process, the file pointer is
       positioned where the end-of-mask marker should be located.  */
    pm = ias_pm_read_mask_data_from_file(pm_file, band_number, sca_number);
    if (pm == NULL)
    {
        IAS_LOG_ERROR("Reading requested pixel mask data from file");
        return NULL;
    }

    /* Verify that the end-of-mask marker follows the mask data. */
    status = ias_pm_confirm_marker_in_file(pm_file, IAS_PM_EOM);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Verifying end-of-mask marker for band number %d "
            "SCA number %d mask", band_number, sca_number);
        ias_pm_destroy(pm);
        return NULL;
    }

    return pm;
}   /* END -- ias_pm_read_single_mask_from_file */
