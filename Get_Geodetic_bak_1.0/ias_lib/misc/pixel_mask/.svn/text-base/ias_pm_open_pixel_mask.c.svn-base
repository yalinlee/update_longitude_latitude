/**************************************************************************
 NAME:       ias_pm_open_pixel_mask

 PURPOSE:    Opens the specified pixel mask file with the specified
             IAS access mode

 RETURNS:    Pointer to allocated IAS_PIXEL_MASK_IO data structure if
             successful; NULL pointer if there is an error

 NOTES:      The data in a pixel mask file is arranged as follows.  The
             mask data is stored in the file as a series of integers.

             Pixel mask file format:
             -----------------------------------------------------------
            | number of masks                                           |
            |                                                           |
            |   ------------------------------------------------------  |
            |  | Mask 1: consists of 4 integers and m spans           | |
            |  |--------------------------------------------          | |
            |  | band number                                          | |
            |  | SCA number                                           | |
            |  | Number of detectors                                  | |
            |  | number of pixels                                     | |
            |  |   -----------------------------------                | |
            |  |  | Span 1: consists of 4 integers |                  | |
            |  |  | ----------------------------------|               | |
            |  |  | detector index                    |               | |
            |  |  | starting pixel index              |               | |
            |  |  | length of span                    |               | |
            |  |  | pixel mask                        |               | |
            |  |   -----------------------------------                | |
            |  |                                                      | |
            |  |   -----------------------------------                | |
            |  |  | Span 1                            |               | |
            |  |   -----------------------------------                | |
            |  |                                                      | |
            |  |   ...                                                | |
            |  |                                                      | |
            |  |   -----------------------------------                | |
            |  |  | Span m                            |               | |
            |  |   -----------------------------------                | |
            |  |                                                      | |
            |  | End of mask marker                                   | |
            |   ------------------------------------------------------  |
            |                                                           |
            |   ------------------------------------------------------  |
            |  | Mask 2                                               | |
            |   ------------------------------------------------------  |
            |                                                           |
            |   ...                                                     |
            |                                                           |
            |   ------------------------------------------------------  |
            |  | Mask n                                               | |
            |   ------------------------------------------------------  |
            |                                                           |
            |                                                           |
            |   ------------------------------------------------------  |
            |  | Start of housekeeping marker                         | |
            |  |   ---------------------------------------------      | |
            |  |  | Housekeeping -- Mask 1                      |     | |
            |  |   ---------------------------------------------      | |
            |  |                                                      | |
            |  |   ...                                                | |
            |  |                                                      | |
            |  |   ---------------------------------------------      | |
            |  |  | Housekeeping -- Mask n                      |     | |
            |  |   ---------------------------------------------      | |
            |   ------------------------------------------------------  |
            |                                                           |
            | End of file marker                                        |
             -----------------------------------------------------------


***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "ias_const.h"
#include "ias_pixel_mask.h"
#include "ias_logging.h"
#include "ias_types.h"                 /* IAS_ACCESS_MODE */
#include "pm_local.h"


IAS_PIXEL_MASK_IO *ias_pm_open_pixel_mask
(
    const char *pixel_mask_file_name,   /* I: Path and name of mask file */
    IAS_ACCESS_MODE access_mode         /* I: Requested access mode */
)
{
    const char *fopen_mode_string = NULL; /* fopen() access mode string */
    IAS_PIXEL_MASK_IO *pm_file = NULL;    /* open pixel mask file */


    /* Allocate the IO data structure. */
    pm_file = calloc(1, sizeof(IAS_PIXEL_MASK_IO));
    if (pm_file == NULL)
    {
        IAS_LOG_ERROR("Allocating pixel mask IO file block");
        return NULL;
    }

    /* Construct the proper access mode string for 'fopen' based on the
       requested IAS access mode.  The only valid access modes for a
       pixel mask file are IAS_WRITE or IAS_READ -- it is assumed that
       the file will be opened once and all writes/reads to/from it occur
       before it is closed.  */
    switch (access_mode)
    {
        case IAS_WRITE:                           /* Write-only */
            fopen_mode_string = "w";
            break;
        case IAS_READ:                            /* Read-only */
            fopen_mode_string = "r";
            break;
        default:
            IAS_LOG_ERROR("Invalid/unknown IAS access mode %d", access_mode);
            return NULL;
    }

    /* Initialize the file access mode. */
    pm_file->access_mode = access_mode;

    /* Copy the file name. */
    pm_file->pixel_mask_file_name = strdup(pixel_mask_file_name);
    if (pm_file->pixel_mask_file_name == NULL)
    {
        IAS_LOG_ERROR("Allocating buffer for pixel mask file name");
        free(pm_file);
        return NULL;
    };

    /* Open the file. */
    pm_file->fptr = fopen(pixel_mask_file_name, fopen_mode_string);
    if (pm_file->fptr == NULL)
    {
        IAS_LOG_ERROR("Cannot open pixel mask file %s",
            pixel_mask_file_name);
        free(pm_file->pixel_mask_file_name);
        free(pm_file);
        return NULL;
    }

    /* For read-only and access, read the existing mask housekeeping data.  */
    if (access_mode == IAS_READ)
    {
        off_t starting_hk_data_offset;     /* starting byte of
                                              start-of-housekeeping marker */
        int index;                         /* local loop counter */
        int status;
        size_t number_of_records_read;

        /* Get the current number of masks present in the file. Make sure
           we've got a nonzero mask count at this point.  */
        number_of_records_read = fread(&(pm_file->number_of_masks_present),
            sizeof(int), 1, pm_file->fptr);
        if (number_of_records_read != 1)
        {
            IAS_LOG_ERROR("Problem reading number of masks in file, %d "
                "of 1 records read", (int)number_of_records_read);
            ias_pm_close_pixel_mask(pm_file);
            return NULL;
        }
        if (pm_file->number_of_masks_present <= 0)
        {
            IAS_LOG_ERROR("Mask count not found in pixel mask file");
            ias_pm_close_pixel_mask(pm_file);
            return NULL;
        }

        /* Allocate the housekeeping data buffer. */
        pm_file->hk = malloc(pm_file->number_of_masks_present
            * sizeof(IAS_PIXEL_MASK_FILE_HOUSEKEEPING));
        if (pm_file->hk == NULL)
        {
            IAS_LOG_ERROR("Allocating mask housekeeping data buffer");
            ias_pm_close_pixel_mask(pm_file);
            return NULL;
        }

        /* Get to the offset for the start-of-housekeeping marker.
           If this marker isn't read from the file, the file is suspect.  */
        starting_hk_data_offset =
            (off_t)((IAS_PM_NUMBER_OF_MARKER_VALUES * sizeof(int))
            + (pm_file->number_of_masks_present
               * sizeof(IAS_PIXEL_MASK_FILE_HOUSEKEEPING))
            + (IAS_PM_NUMBER_OF_MARKER_VALUES * sizeof(int)));

        if (fseeko(pm_file->fptr, -starting_hk_data_offset, SEEK_END) < 0)
        {
            IAS_LOG_ERROR("Setting file pointer to start-of-housekeeping "
                "marker");
            ias_pm_close_pixel_mask(pm_file);
            return NULL;
        }

        /* Verify the start-of-housekeeping marker is present.  */
        status = ias_pm_confirm_marker_in_file(pm_file, IAS_PM_SOH);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Finding/verifying start-of-housekeeping marker "
                "in pixel mask file");
            ias_pm_close_pixel_mask(pm_file);
            return NULL;
        }

        /* Read the housekeeping data. */
        number_of_records_read = fread(pm_file->hk,
            sizeof(IAS_PIXEL_MASK_FILE_HOUSEKEEPING),
            pm_file->number_of_masks_present,
            pm_file->fptr);
        if (number_of_records_read != pm_file->number_of_masks_present)
        {
            IAS_LOG_ERROR("Problem reading mask housekeeping information "
                "from file, %d of %d records read",
                (int)number_of_records_read,
                pm_file->number_of_masks_present);
            ias_pm_close_pixel_mask(pm_file);
            return NULL;
        }

        /* Print the housekeeping data for debugging purposes. */
        IAS_LOG_DEBUG("Starting byte offset of housekeeping data:  %llu",
            (long long unsigned)starting_hk_data_offset);

        for (index = 0; index < pm_file->number_of_masks_present; index++)
        {
            IAS_LOG_DEBUG("Housekeeping data for mask: %d\n"
                "Band number:       %d\n"
                "SCA number:        %d\n"
                "Mask index:        %d\n"
                "Starting byte:     %llu\n"
                "Mask size (bytes): %u\n\n",
                (index + 1), pm_file->hk[index].band_number,
                pm_file->hk[index].sca_number,
                pm_file->hk[index].mask_index,
                (long long unsigned)pm_file->hk[index].starting_data_offset,
                pm_file->hk[index].mask_data_size);
        }

        /* Read the end-of-file marker and verify it's valid. */
        status = ias_pm_confirm_marker_in_file(pm_file, IAS_PM_EOF);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading/validating end-of-file marker in "
                "pixel mask file");
            ias_pm_close_pixel_mask(pm_file);
            return NULL;
        }

        /* Return to the beginning of the file. */
        rewind(pm_file->fptr);
    }

    return pm_file;
}   /* END -- ias_pm_open_pixel_mask */
