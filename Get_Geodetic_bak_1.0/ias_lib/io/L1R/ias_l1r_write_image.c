/******************************************************************************
Name: ias_l1r_write_image

Purpose: Write to the specified 1R image.

Returns:
    SUCCESS if the data is successfully written
    ERROR if an error occurs writing the data

******************************************************************************/
#include <limits.h>
#include "ias_logging.h"
#include "ias_l1r.h"
#include "ias_l1r_private.h"

int ias_l1r_write_image
(
    const L1R_BAND_IO *l1r_band, /* I: HDF IO band structure */
    int sca_index,               /* I: SCA to write to (0-rel) */
    int line_start,              /* I: Line to start writing (0-rel) */
    int start_sample,            /* I: Sample to start writing (0-rel) */
    int lines,                   /* I: Number of lines to write */
    int samples,                 /* I: Number of samples to write */
    void *data                   /* I: Data buffer */
)
{
    hid_t data_space;       /* dataspace for the data buffer dimensions */
    hsize_t data_dims[2] = {lines, samples}; /* size of the data buffer */
    hsize_t file_size[3] = {1, lines, samples}; /* slab size */
    /* hyperslab start location */
    hsize_t file_offset[3] = {sca_index, line_start, start_sample}; 
    int status;

    /* check for various errors in the input */
    if (l1r_band == NULL)
    {
        IAS_LOG_ERROR("NULL band pointer passed in");
        return ERROR;
    }

    if (l1r_band->id < 0)
    {
        IAS_LOG_ERROR("Band is not open for writing in file %s",
                      l1r_band->l1r_file->filename);
        return ERROR;
    }

    /* verify the window of data being written actually falls within the band */
    if ((sca_index < 0) || (sca_index >= l1r_band->scas)
        || (line_start < 0) || ((line_start + lines) > l1r_band->lines)
        || (start_sample < 0) || ((start_sample + samples) > l1r_band->samples))
    {
        IAS_LOG_ERROR("Attempted to write imagery to SCA index %d, band number "
            "%d of %s at line %d, sample %d for a window %d lines x %d samples "
            "when the band has %d SCAs, %d lines, %d samples", sca_index,
            l1r_band->number, l1r_band->l1r_file->filename, line_start, 
            start_sample, lines, samples, l1r_band->scas, l1r_band->lines, 
            l1r_band->samples);
        return ERROR;
    }

    /* verify the access mode allows writing */
    if (l1r_band->l1r_file->access_mode == IAS_READ)
    {
        IAS_LOG_ERROR("Attempted write to file %s opened in read mode",
                      l1r_band->l1r_file->filename);
        return ERROR;
    }

    /* define the memory dataspace for the buffer that will be written */
    data_space = H5Screate_simple(2, data_dims, NULL);
    if (data_space < 0)
    {
        IAS_LOG_ERROR("Creating memory dataspace");
        return ERROR;
    }

    status = H5Sselect_hyperslab(l1r_band->dataspace_id, H5S_SELECT_SET, 
                file_offset, NULL, file_size, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Selecting hyperslab for file %s, band %d",
                      l1r_band->l1r_file->filename, l1r_band->number);
        H5Sclose(data_space);
        return ERROR;
    }

    /* write the data to the dataset */
    status = H5Dwrite(l1r_band->id, l1r_band->memory_data_type,
                      data_space, l1r_band->dataspace_id, H5P_DEFAULT, data);
    H5Sclose(data_space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing to file %s, band number %d, SCA index %d, line "
            "%d, sample %d, number of lines %d, number of samples %d",
            l1r_band->l1r_file->filename, l1r_band->number, sca_index, 
            line_start, start_sample, lines, samples);
        return ERROR;
    }

    return SUCCESS;
}
