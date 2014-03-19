/*******************************************************************************
Name: ias_l1g_close_band

Purpose: Close the currently open L1G band

Returns:
    SUCCESS if the band is successfully closed
    ERROR if an error is detected when closing the band

*******************************************************************************/
#include <stdlib.h>
#include "ias_const.h"      /* SUCCESS/ERROR definition */
#include "ias_logging.h"
#include "ias_l1g.h"
#include "ias_l1g_private.h"

int ias_l1g_close_band
(
    L1G_BAND_IO *l1g_band      /* I: L1G_BAND_IO structure to close */
)
{
    int status;

    ias_linked_list_remove_node(&l1g_band->node);

    /* close the band, saving the return status for later */
    status = H5Dclose(l1g_band->band_id);

    /* clean up a few other resources for the band */
    if (l1g_band->band_memory_data_type >= 0)
    {
        H5Tclose(l1g_band->band_memory_data_type);
    }

    if (l1g_band->band_dataspace_id >= 0)
    {
        H5Sclose(l1g_band->band_dataspace_id);
    }

    /* return the correct value */
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing band %d for %s", l1g_band->band_number, 
                      l1g_band->l1g_file->filename);
        free(l1g_band);
        return ERROR;
    }

    free(l1g_band);

    return SUCCESS;
}
