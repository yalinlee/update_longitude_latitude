/*****************************************************************************
NAME:    ias_pm_read_mask_data_from_file

PURPOSE: Checks the pixel mask file for the mask data with the specified
         mask index and creates a pixel mask with the data.  Refer to the
         function prolog in 'ias_pm_open_pixel_mask.c' for additional
         information regarding the pixel mask file contents and layout.

RETURNS: Pointer to the single band/SCA mask read from the file if
         successful, NULL if there is an error.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "pm_local.h"
#include "ias_pixel_mask.h"
#include "ias_logging.h"
#include "ias_const.h"


IAS_PIXEL_MASK *ias_pm_read_mask_data_from_file
(
    const IAS_PIXEL_MASK_IO *pm_file,  /* I: Open pixel mask file */
    int band_number,                   /* I: Band number of mask */
    int sca_number                     /* I: SCA number of mask */
)
{
    IAS_PIXEL_MASK *pm = NULL;         /* Pixel mask structure to allocate
                                          and populate */
    off_t starting_data_offset = 0;    /* Starting offset for this mask's
                                          data (in bytes) */
    off_t number_of_span_integers_to_read;
                                       /* Total number of span data integers
                                          to be read */
    off_t mask_data_size;              /* Amount of mask data in bytes */
    size_t number_of_integers_read = 0;   /* Number of items read from file */
    size_t read_buffer_size;           /* Total size of the data buffer
                                          holding the mask data */
    unsigned int *data_buffer = NULL;  /* Data buffer to read into */
    int index;                         /* Local loop counter */
    int mask_index;
    int mask_found = 0;                /* Flag indicating mask with the
                                          specified band/SCA number was
                                          found. */


    /* If no masks are recorded as present in the file, that's an error. */
    if (pm_file->number_of_masks_present == 0)
    {
        IAS_LOG_ERROR("No masks are recorded as present in the pixel "
            "mask file");
        return NULL;
    }

    /* Search the housekeeping data for the mask with the specified
       band and SCA number. */
    for (index = 0; index < pm_file->number_of_masks_present; index++)
    {
        if ((pm_file->hk[index].band_number == band_number)
            && (pm_file->hk[index].sca_number == sca_number))
        {
            mask_index = pm_file->hk[index].mask_index;
            mask_found = 1;
            break;
        }
    }
    if (!mask_found)
    {
        IAS_LOG_ERROR("Band number %d SCA number %d mask not found in "
            "pixel mask file", band_number, sca_number);
        return NULL;
    }

    /* Allocate the data buffer the mask 'header' and span data will be
       read into (the 'header' data is considered to be the band number,
       the SCA number, the number of detectors, and the number of pixels).
       The constant IAS_PM_MAX_BUFFER_SIZE should be defined as a multiple
       of the number of integers used to represent a span's data in the
       pixel mask file (which is currently 4, based on the definition of the
       pixel mask span).  */
    read_buffer_size = IAS_PM_MAX_BUFFER_SIZE * sizeof(unsigned int);
    data_buffer = malloc(read_buffer_size);
    if (data_buffer == NULL)
    {
        IAS_LOG_ERROR("Allocating input data buffer");
        return NULL;
    }

    /* Get the starting data offset and the size of the current mask from the          housekeeping data. */
    starting_data_offset = pm_file->hk[mask_index].starting_data_offset;
    mask_data_size = pm_file->hk[mask_index].mask_data_size;

    /* Go to the starting data location. */
    if (fseeko(pm_file->fptr, starting_data_offset, SEEK_SET) < 0)
    {
        IAS_LOG_ERROR("Positioning file pointer at start of mask %d "
            "data", (mask_index + 1));
        free(data_buffer);
        return NULL;
    }

    /* Determine the number of span integers to read.   This is just the
       size of the mask - the span 'header' bytes, divided by the number of
       bytes in an integer. */
    number_of_span_integers_to_read =
        (mask_data_size - (4 * sizeof(unsigned int))) / sizeof(unsigned int);

    /* Read in the band/SCA and dimension information. */
    number_of_integers_read = fread(data_buffer, sizeof(unsigned int), 4,
        pm_file->fptr);
    if (number_of_integers_read != 4)
    {
        IAS_LOG_ERROR("Problem reading mask dimension information from "
            "file");
        free(data_buffer);
        return NULL;
    }

    /* Create and initialize the mask data structure with the band/SCA and
       dimension information.  The information is organized as follows:
       data_buffer[0]:  band number
       data_buffer[1]:  SCA number
       data_buffer[2]:  number of detectors
       data_buffer[3]:  number of pixels */
    pm = ias_pm_create(data_buffer[0], data_buffer[1], data_buffer[2],
        data_buffer[3]);
    if (pm == NULL)
    {
        IAS_LOG_ERROR("Creating band number %d SCA number %d pixel mask "
            "data structure", pm_file->hk[mask_index].band_number,
            pm_file->hk[mask_index].sca_number);
        free(data_buffer);
        return NULL;
    }

    /* Read the span data.  */
    while (number_of_span_integers_to_read > 0)
    {
        size_t current_number_of_integers_to_read;

        if (number_of_span_integers_to_read < IAS_PM_MAX_BUFFER_SIZE)
        {
            current_number_of_integers_to_read =
                number_of_span_integers_to_read;
        }
        else
        {
            current_number_of_integers_to_read = IAS_PM_MAX_BUFFER_SIZE;
        }

        number_of_integers_read = fread(data_buffer, sizeof(unsigned int),
            current_number_of_integers_to_read, pm_file->fptr);

        if (number_of_integers_read != current_number_of_integers_to_read)
        {
            IAS_LOG_ERROR("Problem reading span data:  %d of %d records "
                "read", (int)number_of_integers_read,
                (int)current_number_of_integers_to_read);
            ias_pm_destroy(pm);
            free(data_buffer);
            return NULL;
        }

        /* Add the span(s) to the mask.  The integer values representing a
           span are as follows:
           data_buffer[index]     = starting detector index
           data_buffer[index + 1] = starting pixel index
           data_buffer[index + 2] = number of pixels
           data_buffer[index + 3] = composite mask value */
        for (index = 0; index < number_of_integers_read; index += 4)
        {
            if (ias_pm_add_pixels(pm, data_buffer[index],
                data_buffer[index + 1], data_buffer[index + 2],
                data_buffer[index + 3]))
            {
                IAS_LOG_ERROR("Problem adding span data to band number %d "
                    "SCA number %d mask", pm->band, pm->sca);
                ias_pm_destroy(pm);
                free(data_buffer);
                return NULL;
            }
        }

        /* Decrement the number of span integers left to read by the number of
           span integers that were just read. */
        number_of_span_integers_to_read -= number_of_integers_read;
    }

    /* Cleanup. */
    free(data_buffer);

    return pm;
}   /* END -- ias_pm_read_mask_data_from_file */



/*---------------------------------------------------------------------------
 NAME:     ias_pm_confirm_marker_in_file

 PURPOSE:  Confirms the specified input marker is present in a pixel mask
           file

 RETURNS:  Integer status code of SUCCESS if the buffer read from the
           file contains the correct marker value, ERROR otherwise

 NOTES:    This function assumes/expects that a pixel mask marker is
           represented as a 1D array of integers all having the
           same marker value.  This format is currently written by the
           function 'ias_pm_write_marker_to_file'.
----------------------------------------------------------------------------*/
int ias_pm_confirm_marker_in_file
(
    const IAS_PIXEL_MASK_IO *pm_file,   /* I: Open pixel mask file */
    int expected_marker_value           /* I: Marker value to confirm */
)
{
    int read_marker_buffer[IAS_PM_NUMBER_OF_MARKER_VALUES];
                                        /* data buffer to contain marker
                                           data read from file */
    int i;                              /* loop counter */
    size_t number_of_records_read;


    /* Make sure we're using one of the defined marker values for the
       marker -- the end-of-mask marker, end-of-file marker, or
       start-of-housekeeping marker.  */
    if (!((expected_marker_value == IAS_PM_EOM)
        || (expected_marker_value == IAS_PM_EOF)
        || (expected_marker_value == IAS_PM_SOH)))
    {
        IAS_LOG_ERROR("Invalid pixel mask marker value %d",
            expected_marker_value);
        return ERROR;
    }

    /* Read the marker values from the pixel mask file. */
    number_of_records_read = fread(read_marker_buffer, sizeof(int),
        IAS_PM_NUMBER_OF_MARKER_VALUES, pm_file->fptr);
    if (number_of_records_read != IAS_PM_NUMBER_OF_MARKER_VALUES)
    {
        IAS_LOG_ERROR("Problem reading expected marker data, %d of %d "
            "records read", (int)number_of_records_read,
            IAS_PM_NUMBER_OF_MARKER_VALUES);
        return ERROR;
    }

    /* Make sure the marker array just read has the expected marker values. */
    for (i = 0; i < IAS_PM_NUMBER_OF_MARKER_VALUES; i++)
    {
        if (read_marker_buffer[i] != expected_marker_value)
        {
            IAS_LOG_ERROR("Invalid pixel mask marker read from file");
            return ERROR;
        }
    }

    return SUCCESS;
}   /* END -- ias_pm_confirm_marker_in_file */
