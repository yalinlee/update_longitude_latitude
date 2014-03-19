/*----------------------------------------------------------------------------
NAME:    ias_pm_read_array_from_file

PURPOSE: Read an array of pixel masks from a file that was written by the
         ias_pm_write_array_to_file' routine.  Refer to the function prolog
         in 'ias_pm_open_pixel_mask.c' for an explanation of the pixel mask
         file contents and layout.

RETURNS: Pointer to array of pixel masks or NULL on error
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pm_local.h"
#include "ias_logging.h"
#include "ias_const.h"


/* Local prototype */
static void free_the_pm(IAS_PIXEL_MASK **, int);

IAS_PIXEL_MASK **ias_pm_read_array_from_file
(
    const char *input_file_name,   /* I: Name of input mask file */
    int *num_of_masks              /* O: Number of pixel masks read */
)
{
    IAS_PIXEL_MASK_IO *pm_file = NULL;
    IAS_PIXEL_MASK **pixel_mask_array;
    IAS_PIXEL_MASK *curr_mask = NULL;
    IAS_PIXEL_MASK_BAND_SCA_LIST *band_sca_list = NULL;
    int pm;                        /* Pixel map loop counter */
    int status;
    int number_of_masks_present;
    size_t array_size;


    /* Initialization. */
    *num_of_masks = 0;

    /* Open the pixel mask file.  As part of the opening process, we'll
       verify the start-of-housekeeping and end-of-file markers are correct. */
    pm_file = ias_pm_open_pixel_mask(input_file_name, IAS_READ);
    if (pm_file == NULL)
    {
        IAS_LOG_ERROR("Opening pixel mask file %s for reading",
            input_file_name);
        return NULL;
    }

    /* Get the list of bands/SCAs in the mask file.  This will also
       determine the number of masks present in the file. */
    status = ias_pm_get_file_band_sca_list(pm_file, &band_sca_list,
        &number_of_masks_present);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Retrieving mask band/SCA list from pixel mask file");
        ias_pm_close_pixel_mask(pm_file);
        return NULL;
    }

    /* Allocate the return array of structures */
    array_size = number_of_masks_present * sizeof(*pixel_mask_array);
    pixel_mask_array = malloc(array_size);
    if (!pixel_mask_array)
    {
        IAS_LOG_ERROR("Allocating a pixel mask array");
        ias_pm_close_pixel_mask(pm_file);
        free(band_sca_list);
        return NULL;
    }
    memset(pixel_mask_array, 0, array_size);

    /* Loop through all the masks. */
    for (pm = 0; pm < number_of_masks_present; pm++)
    {
        /* Read the current mask.  Verification that the end-of-mask
           marker follows the mask data is performed as part of this
           operation. */
        curr_mask = ias_pm_read_single_mask_from_file(pm_file,
            band_sca_list[pm].band_number,
            band_sca_list[pm].sca_number);
        if (curr_mask == NULL)
        {
            IAS_LOG_ERROR("Reading pixel mask with band number %d SCA "
                "number %d", band_sca_list[pm].band_number,
                band_sca_list[pm].sca_number);
            free_the_pm(pixel_mask_array, number_of_masks_present);
            ias_pm_close_pixel_mask(pm_file);
            free(band_sca_list);
            return NULL;
        }
        pixel_mask_array[pm] = curr_mask;

    }  /* END for pm loop */

    /* Final cleanup */
    *num_of_masks = number_of_masks_present;
    ias_pm_close_pixel_mask(pm_file);
    free(band_sca_list);

    return pixel_mask_array;
}  /* END ias_pm_read_array_from_file */


/* Internal routine to free the pixel mask array and any pixel masks
added to it. */
static void free_the_pm(IAS_PIXEL_MASK **pixel_mask, int num_of_masks)
{
    IAS_PIXEL_MASK *pm; /* Pointer to a single pixel mask */
    int m;              /* Mask loop counter */

    if (pixel_mask != NULL)
    {
        /* Loop through the pixel mask array looking for allocated masks */
        for ( m = 0; m < num_of_masks; m++ )
        {
            pm = pixel_mask[m];
            /* If a mask is found, destroy it */
            if (pm != NULL)
                ias_pm_destroy(pm);
        }

        free(pixel_mask);
    }
}  /* END internal routine free_the_pm */
