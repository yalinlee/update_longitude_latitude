/*******************************************************************************
Name: ias_l1r_close_image

Purpose: Close the specified image.

Returns:
    SUCCESS if the image is successfully closed
    ERROR if an error occurs closing the image

*******************************************************************************/
#include <stdlib.h>
#include "ias_logging.h"
#include "ias_l1r.h"
#include "ias_l1r_private.h"

int ias_l1r_close_image
(
    L1RIO *l1r_file      /* I: L1R IO structure to close */
)
{
    int status = SUCCESS;

    /* verify the input parameter is valid */
    if (!l1r_file)
    {
        IAS_LOG_ERROR("NULL pointer provided for the L1R file");
        return ERROR;
    }

    /* if there is a band open, close it and issue a warning since
       the band should be closed before calling this routine */
    while (ias_linked_list_count_nodes(&l1r_file->open_bands) > 0)
    {
        L1R_BAND_IO *l1r_band;
        IAS_LINKED_LIST_NODE *node;

        IAS_LOG_WARNING("Closing image %s with a band still open. "
                        "Closing band, but calling code should be fixed",
                        l1r_file->filename);

        node = ias_linked_list_get_head_node(&l1r_file->open_bands);
        if (!node)
        {
            /* Error since this could only be null if no nodes were in the 
               linked list */
            IAS_LOG_ERROR("Invalid node returned from the open bands linked "
                          "list");
            return ERROR;
        }
        l1r_band = GET_OBJECT_OF(node, L1R_BAND_IO, node);
        ias_l1r_close_band(l1r_band);

    }

    /* close the file */
    if (H5Fclose(l1r_file->file_id) < 0)
    {
        IAS_LOG_ERROR("Closing image %s", l1r_file->filename);
        status = ERROR;
    }

    /* free the linked lists */
    BAND_DATASET_LINKED_LIST_DELETE(&l1r_file->band_datasets);
    BAND_METADATA_LINKED_LIST_DELETE(&l1r_file->band_metadata);

    /* free the memory associated with the structure */
    free(l1r_file->filename);
    free(l1r_file);

    return status;
}
