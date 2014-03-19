/***************************************************************************
 NAME:        ias_pm_get_file_band_sca_list

 PURPOSE:     Reads the housekeeping data in the specified pixel mask file
              and populates an array of structures providing the band and
              SCA numbers for the mask(s) contained in the file.

 RETURNS:     Integer status code of SUCCESS if band/SCA list is populated,
              ERROR otherwise.  The caller of this routine is responsible
              for allocating and freeing the memory block for the list.
****************************************************************************/
#include <stdlib.h>
#include "pm_local.h"
#include "ias_pixel_mask.h"
#include "ias_logging.h"
#include "ias_const.h"


int ias_pm_get_file_band_sca_list
(
    const IAS_PIXEL_MASK_IO *pm_file,    /* I: Open pixel mask file */
    IAS_PIXEL_MASK_BAND_SCA_LIST **band_sca_list,
                                         /* O: Populated band/SCA list */
    int *list_size                       /* O: Number of items in list
                                            (equivalent to the number of
                                            masks present in the file) */
)
{
    IAS_PIXEL_MASK_BAND_SCA_LIST *local_list = NULL;
    int mask_index;


    /* Make sure the file is validly open. */
    if (pm_file == NULL)
    {
        IAS_LOG_ERROR("Pixel mask file needs to be opened");
        return ERROR;
    }

    /* Make sure the input band/SCA list pointer is initialized to
       something known. */
    *band_sca_list = NULL;

    /* Allocate the local band/SCA list. */
    local_list = malloc(pm_file->number_of_masks_present
        * sizeof(IAS_PIXEL_MASK_BAND_SCA_LIST));
    if (local_list == NULL)
    {
        IAS_LOG_ERROR("Allocating local band/SCA list");
        return ERROR;
    }

    /* Pull the available band/SCA information out of the housekeeping data.
       Depending on how the mask file was first written out, the masks may
       not be in band/SCA order.  */
    for (mask_index = 0;
         mask_index < pm_file->number_of_masks_present;
         mask_index++)
    {
        local_list[mask_index].band_number =
            pm_file->hk[mask_index].band_number;
        local_list[mask_index].sca_number =
            pm_file->hk[mask_index].sca_number;
    }

    /* Save the number of list entries. */
    *list_size = pm_file->number_of_masks_present;

    /* Set the return pointer to the local list pointer. */
    *band_sca_list = local_list;

    return SUCCESS;
}   /* END -- ias_pm_get_file_bandsca_list */
