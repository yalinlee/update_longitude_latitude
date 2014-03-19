/******************************************************************************
Name: ias_l1g_write_image

Purpose: Write to the specified 1G image.

Returns:
    SUCCESS if the data is successfully written
    ERROR if an error occurs writing the data

******************************************************************************/
#include <limits.h>
#include "ias_types.h"      /* IAS_READ definition */
#include "ias_const.h"      /* SUCCESS/ERROR definition */
#include "ias_logging.h"
#include "ias_l1g.h"
#include "ias_l1g_private.h"

int ias_l1g_write_image
(
    const L1G_BAND_IO *l1g_band,   /* I: L1G IO band structure */
    int sca_index,      /* I: SCA to write to (0-rel) */
    int start_line,     /* I: Line to start writing (0-rel) */
    int start_sample,   /* I: Sample to start writing (0-rel) */
    int line_count,     /* I: Number of lines to write */
    int sample_count,   /* I: Number of samples to write */
    void *data          /* I: Data buffer */
)
{
    hid_t data_space;       /* dataspace for the data buffer dimensions */
    hsize_t data_dims[2] = {line_count, sample_count}; 
                            /* size of the data buffer */
    hsize_t file_size[3] = {1, line_count, sample_count};
                            /* slab size to write to the file */
    hsize_t file_offset[3] = {sca_index, start_line, start_sample};
                            /* location to write in the file */
    int status;

    /* check for various errors in the input */
    if (l1g_band == NULL)
    {
        IAS_LOG_ERROR("NULL band pointer passed in");
        return ERROR;
    }

    if (l1g_band->band_id < 0)
    {
        IAS_LOG_ERROR("Band is not open for writing in file %s",
                      l1g_band->l1g_file->filename);
        return ERROR;
    }

    /* verify the window of data being written actually falls within the band */
    if ((sca_index < 0) || (sca_index >= l1g_band->sca_count)
        || (start_line < 0) 
        || ((start_line + line_count) > l1g_band->line_count)
        || (start_sample < 0) 
        || ((start_sample + sample_count) > l1g_band->sample_count))
    {
        IAS_LOG_ERROR("Attempted to write imagery to SCA %d, band %d of %s at"
            " line %d, sample %d for a window %d lines x %d samples when the "
            "band has %d SCAs, %d lines, %d samples", sca_index,
            l1g_band->band_number, l1g_band->l1g_file->filename, start_line,
            start_sample, line_count, sample_count, l1g_band->sca_count,
            l1g_band->line_count, l1g_band->sample_count);
        return ERROR;
    }

    /* verify the access mode allows writing */
    if (l1g_band->l1g_file->access_mode == IAS_READ)
    {
        IAS_LOG_ERROR("Attempted write to file %s opened in read mode",
                      l1g_band->l1g_file->filename);
        return ERROR;
    }

    /* define the memory dataspace for the buffer that will be written */
    data_space = H5Screate_simple(2, data_dims, NULL);
    if (data_space < 0)
    {
        IAS_LOG_ERROR("Creating memory dataspace");
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

    /* write the data to the dataset */
    status = H5Dwrite(l1g_band->band_id, l1g_band->band_memory_data_type,
                  data_space, l1g_band->band_dataspace_id, H5P_DEFAULT, data);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing to file %s, band number %d, SCA index %d, "
            "line %d, sample %d, number of lines %d, number of samples %d",
            l1g_band->l1g_file->filename, l1g_band->band_number, sca_index,
            start_line, start_sample, line_count, sample_count);
        H5Sclose(data_space);
        return ERROR;
    }

    H5Sclose(data_space);

    return SUCCESS;
}
