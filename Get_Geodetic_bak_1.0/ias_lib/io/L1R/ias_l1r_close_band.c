/*******************************************************************************
Name: ias_l1r_close_band

Purpose: Close the currently open L1R band

Returns:
    SUCCESS if the band is successfully closed
    ERROR if an error is detected when closing the band

*******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_l1r.h"
#include "ias_l1r_private.h"

int ias_l1r_close_band
(
    L1R_BAND_IO *l1r_band      /* I: L1R BAND IO structure to close */
)
{
    int status;

    ias_linked_list_remove_node(&l1r_band->node);

    /* close the band, saving the return status for later */
    status = H5Dclose(l1r_band->id);

    /* clean up a few other resources for the band */
    if (l1r_band->memory_data_type >= 0)
    {
        H5Tclose(l1r_band->memory_data_type);
    }

    if (l1r_band->dataspace_id >= 0)
    {
        H5Sclose(l1r_band->dataspace_id);
    }


    /* return the correct value */
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing band %d for %s", l1r_band->number,
                      l1r_band->l1r_file->filename);
        /* free the memory associated with the structure */
        free(l1r_band);
        return ERROR;
    }
    else
    {
        /* free the memory associated with the structure */
        free(l1r_band);
        return SUCCESS;
    }
}
