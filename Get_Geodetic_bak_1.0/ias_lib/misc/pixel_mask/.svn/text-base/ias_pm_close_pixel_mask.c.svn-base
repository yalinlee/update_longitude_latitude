/****************************************************************************
 NAME:       ias_pm_close_pixel_mask

 PURPOSE:    Closes the specified pixel mask file and frees allocated
             resources in the IAS_PIXEL_MASK_IO data structure

 RETURNS:    Integer status code of SUCCESS or ERROR
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "ias_const.h"
#include "ias_pixel_mask.h"
#include "ias_logging.h"
#include "pm_local.h"


/*  Internal function implemented to free allocated memory blocks within
    a PIXEL_MASK_IO data structure, as well as free the memory block
    allocated for the structure itself. */
static void free_pixel_mask_io
(
    IAS_PIXEL_MASK_IO *pm_file        /* I: Currently open pixel mask file */
)
{
    if (pm_file != NULL)
    {
        free(pm_file->hk);
        free(pm_file->pixel_mask_file_name);
        free(pm_file);
    }
}   /* END -- internal function free_pixel_mask_io */


int ias_pm_close_pixel_mask
(
    IAS_PIXEL_MASK_IO *pm_file        /* I: Currently open pixel mask file */
)
{
    /* Verify the input pointer is valid. */
    if (pm_file == NULL)
    {
        IAS_LOG_ERROR("NULL pointer provided for the pixel mask file");
        return ERROR;
    }

    /* Close out the file if it's open. */
    if (pm_file->fptr != NULL)
    {
        /* For files opened with write-only access, write
           out any housekeeping data to the file, then write out the
           end-of-file-marker. */ 
        if (pm_file->hk != NULL)
        {
            if (pm_file->access_mode == IAS_WRITE)
            {
                off_t starting_hk_offset;
                int status;
                size_t number_of_records_written;

                /* Move the file pointer to the end of the last mask's
                   end-of-mask marker relative to the beginning of the
                   file.  The required offset is just the last mask's starting
                   offset + its mask data size + the size of the end-of-mask
                   marker.  */
                starting_hk_offset =
                    (pm_file->hk[pm_file->number_of_masks_present - 1]
                       .starting_data_offset
                     + pm_file->hk[pm_file->number_of_masks_present - 1]
                       .mask_data_size
                     + IAS_PM_NUMBER_OF_MARKER_VALUES * sizeof(int));
                if (starting_hk_offset <= 0)
                {
                    IAS_LOG_ERROR("Invalid file offset %llu calculated",
                        (long long unsigned)starting_hk_offset);
                    fclose(pm_file->fptr);
                    free_pixel_mask_io(pm_file);
                    return ERROR;
                }
                if (fseeko(pm_file->fptr, starting_hk_offset, SEEK_SET) < 0)
                {
                    IAS_LOG_ERROR("Positioning file pointer to starting "
                        "offset where housekeeping data should be written");
                    fclose(pm_file->fptr);
                    free_pixel_mask_io(pm_file);
                    return ERROR;
                }

                /* Write the start-of-housekeeping marker to the file. */
                status = ias_pm_write_marker_to_file(pm_file, IAS_PM_SOH);
                if (status != SUCCESS)
                {
                    IAS_LOG_ERROR("Writing start-of-housekeeping marker to "
                        "pixel mask file");
                    fclose(pm_file->fptr);
                    free_pixel_mask_io(pm_file);
                    return ERROR;
                }

                /* Write out any housekeeping data to the file. */
                number_of_records_written = fwrite(pm_file->hk,
                    sizeof(IAS_PIXEL_MASK_FILE_HOUSEKEEPING),
                    pm_file->number_of_masks_present,
                    pm_file->fptr);
                if (number_of_records_written
                    != pm_file->number_of_masks_present)
                {
                    IAS_LOG_ERROR("Problem writing housekeeping data to file, "
                        "%d of %d records written",
                        (int)number_of_records_written,
                        pm_file->number_of_masks_present);
                    fclose(pm_file->fptr);
                    free_pixel_mask_io(pm_file);
                    return ERROR;
                }

                /* Write the end-of-file marker to the file. */
                status = ias_pm_write_marker_to_file(pm_file, IAS_PM_EOF);
                if (status != SUCCESS)
                {
                    IAS_LOG_ERROR("Writing end-of-file marker to pixel mask "
                        "file");
                    fclose(pm_file->fptr);
                    free_pixel_mask_io(pm_file);
                    return ERROR;
                }
            }
        }

        /* Close the file. */
        if (fclose(pm_file->fptr))
        {
            IAS_LOG_ERROR("Closing pixel mask file");
            free_pixel_mask_io(pm_file);
            return ERROR;
        }
    }
    else
    {
        /* We shouldn't ever encounter this case -- an IAS_PIXEL_MASK_IO
           structure has been allocated, it has an associated file name
           and/or housekeeping data, but no mask file is open.  If it does
           occur, it's an error. */
        if ((pm_file->hk != NULL) || (pm_file->pixel_mask_file_name != NULL))
        {
            IAS_LOG_ERROR("File not open, but memory blocks for housekeeping "
                "and/or file name data are allocated");
            free_pixel_mask_io(pm_file);
            return ERROR;
        }
    }

    /* Cleanup. */
    free_pixel_mask_io(pm_file);

    return SUCCESS;
}   /* END -- ias_pm_close_pixel_mask */
