/******************************************************************************
Name: ias_l1g_read_image

Purpose: Read image data from the open band of an L1G image.

Returns:
    SUCCESS if the data is successfully read
    ERROR if an error occurs reading the data

******************************************************************************/
#include <stdlib.h>         /* for malloc */
#include "ias_const.h"      /* ERROR/SUCCESS */
#include "ias_logging.h"
#include "ias_l1g.h"        /* L1G structure definition and APIs */
#include "ias_l1g_private.h"/* Internal/private definitions */

int ias_l1g_read_image 
(
    const L1G_BAND_IO *l1g_band,   /* I: L1G IO band structure */
    int sca_index,      /* I: SCA to read to (0-rel) */
    int start_line,     /* I: Line to start reading (0-rel) */
    int start_sample,   /* I: Sample to start reading (0-rel) */
    int line_count,     /* I: Number of lines to read */
    int sample_count,   /* I: Number of samples to read */
    void *data          /* O: Data buffer */
)
{
    hid_t data_space;       /* dataspace for the data buffer dimensions */
    hsize_t data_dims[2] = {line_count, sample_count};
                            /* size of the data buffer */
    hsize_t file_size[3] = {1, line_count, sample_count};
                            /* slab size to read from the file */
    hsize_t file_offset[3] = {sca_index, start_line, start_sample}; 
                            /* location to read in the file */
    int status;

    /* check for various errors in the input */
    if (l1g_band == NULL)
    {
        IAS_LOG_ERROR("NULL band pointer passed in");
        return ERROR;
    }

    if (l1g_band->band_id < 0)
    {
        IAS_LOG_ERROR("Band is not open for reading in file %s",
                      l1g_band->l1g_file->filename);
        return ERROR;
    }

    /* verify the window of data being read actually falls within the band */
    if ((sca_index < 0) || (sca_index >= l1g_band->sca_count)
        || (start_line < 0)
        || ((start_line + line_count) > l1g_band->line_count)
        || (start_sample < 0)
        || ((start_sample + sample_count) > l1g_band->sample_count))
    {
        IAS_LOG_ERROR("Attempted to read imagery from SCA index %d, band "
            "number %d of %s at line %d, sample %d for a window %d lines "
            "x %d samples when the band has %d SCAs, %d lines, %d samples",
            sca_index, l1g_band->band_number, l1g_band->l1g_file->filename, 
            start_line, start_sample, line_count, sample_count,
            l1g_band->sca_count, l1g_band->line_count, l1g_band->sample_count);
        return ERROR;
    }

    /* define the memory dataspace to read data into */
    data_space = H5Screate_simple(2, data_dims, NULL);
    if (data_space < 0)
    {
        IAS_LOG_ERROR("Creating memory dataspace for file %s",
                      l1g_band->l1g_file->filename);
        return ERROR;
    }

    status = H5Sselect_hyperslab(l1g_band->band_dataspace_id, H5S_SELECT_SET,
                file_offset, NULL, file_size, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Selecting hyperslab for file %s, band %d",
                      l1g_band->l1g_file->filename, l1g_band->band_number);
        H5Sclose(data_space);
        return ERROR;
    }

    /* read the data from the dataset */
    status = H5Dread(l1g_band->band_id, l1g_band->band_memory_data_type,
                 data_space, l1g_band->band_dataspace_id, H5P_DEFAULT, data);
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading from file %s, band %d, SCA %d, line %d, "
            "sample %d, number of lines %d, number of samples %d",
            l1g_band->l1g_file->filename, l1g_band->band_number, sca_index,
            start_line, start_sample, line_count, sample_count);
        H5Sclose(data_space);
        return ERROR;
    }

    H5Sclose(data_space);

    return SUCCESS;
}
