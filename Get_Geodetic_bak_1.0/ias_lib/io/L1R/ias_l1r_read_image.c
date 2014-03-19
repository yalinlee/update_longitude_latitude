/******************************************************************************
Name: ias_l1r_read_image

Purpose: Read image data from the open band of an L1R image.

Returns:
    SUCCESS if the data is successfully read
    ERROR if an error occurs reading the data

******************************************************************************/
#include <stdlib.h>         /* for malloc */
#include "ias_const.h"      /* ERROR/SUCCESS */
#include "ias_logging.h"
#include "ias_l1r.h"        /* L1R structure definition and APIs */
#include "ias_l1r_private.h"/* Internal/private definitions */

int ias_l1r_read_image 
(
    const L1R_BAND_IO *l1r_band, /* I: HDF IO band structure */
    int sca_index,       /* I: SCA to read (0-rel) */
    int line_start,      /* I: Line to start reading (0-rel) */
    int sample_start,    /* I: Sample to start reading (0-rel) */
    int lines,           /* I: Number of lines to read */
    int samples,         /* I: Number of samples to read */
    void *data           /* O: Data buffer */
)
{
    hid_t data_space;       /* dataspace for the data buffer dimensions */
    hsize_t data_dims[2] = {lines, samples}; /* size of the data buffer */
    hsize_t file_size[3] = {1, lines, samples}; /* slab size to read from the 
                                                   file */
    /* location to read in the file */
    hsize_t file_offset[3] = {sca_index, line_start, sample_start}; 
    int status;

    /* check for various errors in the input */
    if (l1r_band == NULL)
    {
        IAS_LOG_ERROR("NULL band pointer passed in");
        return ERROR;
    }

    if (l1r_band->id < 0)
    {
        IAS_LOG_ERROR("Band is not open for reading in file %s",
                      l1r_band->l1r_file->filename);
        return ERROR;
    }

    /* verify the window of data being read actually falls within the band */
    if ((sca_index < 0) || (sca_index >= l1r_band->scas)
        || (line_start < 0) || ((line_start + lines) > l1r_band->lines)
        || (sample_start < 0) || ((sample_start + samples) > l1r_band->samples))
    {
        IAS_LOG_ERROR("Attempted to read imagery from SCA index %d, band "
            "number %d of %s at line %d, sample %d for a window %d lines "
            "x %d samples when the band has %d SCAs, %d lines, %d samples", 
            sca_index, l1r_band->number, l1r_band->l1r_file->filename, 
            line_start, sample_start, lines, samples, l1r_band->scas,  
            l1r_band->lines, l1r_band->samples);
        return ERROR;
    }

    /* define the memory dataspace to read data into */
    data_space = H5Screate_simple(2, data_dims, NULL);
    if (data_space < 0)
    {
        IAS_LOG_ERROR("Creating memory dataspace for file %s",
                      l1r_band->l1r_file->filename);
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

    /* read the data from the dataset */
    status = H5Dread(l1r_band->id, l1r_band->memory_data_type,
                     data_space, l1r_band->dataspace_id, H5P_DEFAULT, data);
    H5Sclose(data_space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading from file %s, band number %d, SCA index %d, "
            "line %d, sample %d, number of lines %d, number of samples %d",
            l1r_band->l1r_file->filename, l1r_band->number, sca_index, 
            line_start, sample_start, lines, samples);
        return ERROR;
    }

    return SUCCESS;
}
