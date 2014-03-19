/****************************************************************************
NAME:    ias_pm_write_mask_data_to_file

PURPOSE: Converts a pixel mask from a single band/SCA to a format suitable
         for writing to a file and appends it to the specified pixel mask
         file.  Refer to the function prolog in 'ias_pm_open_pixel_mask.c'
         for additional information on the pixel mask file contents and
         layout.

RETURNS: Integer status code of SUCCESS or ERROR
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "ias_pixel_mask.h"
#include "pm_local.h"            /* Marker values defined here */
#include "ias_logging.h"
#include "ias_const.h"


int ias_pm_write_mask_data_to_file
(
    IAS_PIXEL_MASK_IO *pm_file,           /* I/O: Open pixel mask file */
    IAS_PIXEL_MASK *mask                  /* I: Pixel mask data to add to
                                             file */
)
{
    IAS_PIXEL_MASK_FILE_HOUSEKEEPING *temp_hk = NULL;
                                          /* Temporary pointer for
                                             housekeeping data block */
    IAS_PIXEL_MASK_ITERATOR *it = NULL;   /* Pointer to an iterator */
    IAS_PIXEL_MASK_SPAN span;             /* Span structure */
    unsigned int *data_buffer = NULL;
    unsigned int buffer_size;
                                          /* Buffer containing the mask data
                                             written out as a sequence of
                                             integers */
    off_t ending_offset;                  /* File position where last byte of
                                             mask data is written */
    off_t starting_offset;                /* File position where first byte
                                             of mask data is written */
    int buffer_index;                     /* Index into data buffer array */
    size_t new_housekeeping_buffer_size;  /* Size of housekeeping data block
                                             expanded to accommodate new
                                             mask */
    size_t number_of_records_written;


    /* Increment the number of masks present. */
    pm_file->number_of_masks_present++;

    /* Allocate the housekeeping data block. If pm_file->hk is initially
       NULL, the realloc call is just a malloc.  Use a temporary pointer
       to allocate the larger data block, then set the structure pointer to
       the temporary pointer if the allocation succeeds.  */
    new_housekeeping_buffer_size = pm_file->number_of_masks_present
        * sizeof(IAS_PIXEL_MASK_FILE_HOUSEKEEPING);
    temp_hk = realloc(pm_file->hk, new_housekeeping_buffer_size);
    if (temp_hk == NULL)
    {
        IAS_LOG_ERROR("Allocating larger housekeeping data block");
        return ERROR;
    }
    pm_file->hk = temp_hk;

    /* Initialize the housekeeping data for this mask.  */
    pm_file->hk[pm_file->number_of_masks_present - 1].band_number =
        mask->band;
    pm_file->hk[pm_file->number_of_masks_present - 1].sca_number =
        mask->sca;
    pm_file->hk[pm_file->number_of_masks_present - 1].mask_index =
        pm_file->number_of_masks_present - 1;

    /* Get the starting byte offset in the file for the current mask.  If
       this is the first mask written to the file, the starting offset
       will be sizeof(int) bytes (to account for the integer representing
       the number of masks in the file).  Otherwise, it's the starting offset
       for the previous mask + its size (in bytes)
       + number_of_mask_marker_values*sizeof(int) for the end-of-mask
       marker. */
    if (pm_file->number_of_masks_present == 1)
    {
        starting_offset = (off_t)sizeof(int);
    }
    else
    {
        starting_offset =
            (off_t)(pm_file->hk[pm_file->number_of_masks_present - 2]
                .starting_data_offset
            + pm_file->hk[pm_file->number_of_masks_present - 2].mask_data_size
            + IAS_PM_NUMBER_OF_MARKER_VALUES * sizeof(int));
    }
    pm_file->hk[pm_file->number_of_masks_present - 1].starting_data_offset =
        starting_offset;

    /* Allocate the data buffer for the current masks's contents.
       Make sure the buffer size is a multiple of of the number
       of integers used to represent a span's data in the file (which is
       currently 4, based on the definition of the pixel mask).  */
    buffer_size = IAS_PM_MAX_BUFFER_SIZE;
    if ((buffer_size % 4) != 0)
    {
        IAS_LOG_ERROR("Data buffer where pixel mask spans will be copied to "
            "should be a multiple of 4, since a span is represented by "
            "4 integer values");
        return ERROR;
    }
    data_buffer = malloc(buffer_size * sizeof(unsigned int));
    if (data_buffer == NULL)
    {
        IAS_LOG_ERROR("Allocating output data buffer");
        return ERROR;
    }

    /* Position the file pointer to the required starting byte offset. */
    if (fseeko(pm_file->fptr, starting_offset, SEEK_SET) < 0)
    {
        IAS_LOG_ERROR("Positioning file pointer to correct starting "
            "offset in pixel mask file");
        free(data_buffer);
        return ERROR;
    }

    /* Write the band/SCA and dimension information first. */
    data_buffer[0] = mask->band;
    data_buffer[1] = mask->sca;
    data_buffer[2] = mask->num_of_detectors;
    data_buffer[3] = mask->num_of_pixels;

    number_of_records_written = fwrite(data_buffer, sizeof(unsigned int), 4,
        pm_file->fptr);
    if (number_of_records_written != 4)
    {
        IAS_LOG_ERROR("Problem writing mask dimensions to output file, %d "
            "of 4 records written", (int)number_of_records_written);
        free(data_buffer);
        return ERROR;
    }

    /* Create an iterator for masked spans only. */
    it = ias_pm_get_iterator(mask, IAS_PM_INCLUDE, ~0x0);
    if (it == NULL)
    {
        IAS_LOG_ERROR("Creating pixel mask iterator");
        free(data_buffer);
        return ERROR;
    }

    /*  Write the mask contents to the file.*/
    buffer_index = 0;
    while (ias_pm_get_next_span(it, &span))
    {
        /* Populate the output buffer with the current span's contents. */
        data_buffer[buffer_index]     = span.detector_index;
        data_buffer[buffer_index + 1] = span.starting_pixel_index;
        data_buffer[buffer_index + 2] = span.length_of_span;
        data_buffer[buffer_index + 3] = (int)span.pixel_mask;

        /* Increment the buffer index. This doubles as a count of the
           number of populated elements in the buffer.  */
        buffer_index += 4;

        /* If we've filled the output buffer, write it to the file.  */
        if (buffer_index == IAS_PM_MAX_BUFFER_SIZE)
        {
            number_of_records_written = fwrite(data_buffer,
                sizeof(unsigned int), IAS_PM_MAX_BUFFER_SIZE,
                pm_file->fptr);
            if (number_of_records_written != IAS_PM_MAX_BUFFER_SIZE)
            {
                IAS_LOG_ERROR("Problem writing mask data to output file, %d "
                    "of %d records written", (int)number_of_records_written,
                    IAS_PM_MAX_BUFFER_SIZE);
                free(data_buffer);
                ias_pm_destroy_iterator(it);
                return ERROR;
            }

            /* Reset the buffer index to point to the start of the buffer. */
            buffer_index = 0;
        }
    }

    /* Free the iterator resources. */
    ias_pm_destroy_iterator(it);
    it = NULL;

    /* We may have a partially populated output buffer after all the
       masked spans have been retrieved.  Write these buffer contents to
       the file as well. */
    if (buffer_index)
    {
        number_of_records_written = fwrite(data_buffer, sizeof(unsigned int),
            buffer_index, pm_file->fptr);
        if (number_of_records_written != buffer_index)
        {
            IAS_LOG_ERROR("Problem writing to output file, %d of %d records "
                "written", (int)number_of_records_written, buffer_index);
            free(data_buffer);
            return ERROR;
        }
    }

    /* Free up the local data buffer. */
    free(data_buffer);
    data_buffer = NULL;

    /* Get the current position of the file pointer after the last byte of
       mask data is written. */  
    ending_offset = ftello(pm_file->fptr);
    if (ending_offset < 0)
    {
        IAS_LOG_ERROR("Problem returning current file offset");
        return ERROR;
    }

    /* Set the mask data size as the difference between the ending and
       starting file offsets.  The last mask byte is at (ending_offset - 1)
       bytes. */
    pm_file->hk[pm_file->number_of_masks_present - 1].mask_data_size =
        ending_offset - starting_offset;

    /* Go to the beginning of the file to update the number of masks. */
    if (fseeko(pm_file->fptr, 0, SEEK_SET) < 0)
    {
        IAS_LOG_ERROR("Repositioning file pointer to physical start of "
           "file");
        return ERROR;
    }

    /* Update the number_of_masks, then return to the original ending
       offset in the file (which is the starting byte for the next object
       to be written to the file). */
    number_of_records_written = fwrite(&pm_file->number_of_masks_present,
        sizeof(int), 1, pm_file->fptr);
    if (number_of_records_written != 1)
    {
        IAS_LOG_ERROR("Problem updating number of pixel masks, %d of 1 "
            "records written", (int)number_of_records_written);
        return ERROR;
    }
    if (fseeko(pm_file->fptr, ending_offset, SEEK_SET) < 0)
    {
        IAS_LOG_ERROR("Positioning file pointer to current ending offset");
        return ERROR;
    }

    return SUCCESS;
}   /* END -- ias_pm_write_mask_data_to_file*/



/*--------------------------------------------------------------------------
 NAME:     ias_pm_write_marker_to_file

 PURPOSE:  Writes a marker to the pixel mask file.  The current markers
           that can be written are represented by one of the following
           constants:
           IAS_PM_EOM for an end-of-mask marker
           IAS_PM_EOF for an end-of-file marker
           IAS_PM_SOH for a start-of-housekeeping marker

 RETURNS:  Integer status code of SUCCESS or ERROR

 NOTES:    This function creates a marker buffer as a 1D array of
           integers all having the specified marker value.  The function
           'ias_pm_confirm_marker_in_file' currently expects to read
           a marker buffer with this format.
----------------------------------------------------------------------------*/
int ias_pm_write_marker_to_file
(
    IAS_PIXEL_MASK_IO *pm_file,           /* I: Open pixel mask file */
    int marker_value                      /* I: Specified marker value */
)
{
    size_t number_of_marker_records_written;
    int marker_buffer[IAS_PM_NUMBER_OF_MARKER_VALUES];
    int index;


    /* Make sure we're using one of the defined marker values for the
       marker -- the end-of-mask marker, end-of-file marker, or
       start-of-housekeeping marker.  */
    if (!((marker_value == IAS_PM_EOM) || (marker_value == IAS_PM_EOF)
        || (marker_value == IAS_PM_SOH)))
    {
        IAS_LOG_ERROR("Invalid pixel mask marker value %d", marker_value);
        return ERROR;
    }

    /* Initialize the marker buffer. */
    for (index = 0; index < IAS_PM_NUMBER_OF_MARKER_VALUES; index++)
        marker_buffer[index] = marker_value;

    /* Write the marker data buffer to the mask file. */
    number_of_marker_records_written = fwrite(marker_buffer,
        sizeof(int), IAS_PM_NUMBER_OF_MARKER_VALUES, pm_file->fptr);
    if (number_of_marker_records_written != IAS_PM_NUMBER_OF_MARKER_VALUES)
    {
        IAS_LOG_ERROR("Problem writing pixel mask file marker, %d of "
            "%d records written", (int)number_of_marker_records_written,
            IAS_PM_NUMBER_OF_MARKER_VALUES);
        return ERROR;
    }

    return SUCCESS;
}   /* END -- ias_pm_write_marker_to_file */
