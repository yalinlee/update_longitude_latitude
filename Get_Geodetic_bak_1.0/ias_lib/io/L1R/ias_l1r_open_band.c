/******************************************************************************
Name: ias_l1r_open_band

Purpose: Open a band of the image.

******************************************************************************/
#include <string.h>
#include <stdlib.h>         /* for malloc */
#include "ias_types.h"
#include "ias_const.h"
#include "ias_l1r.h"
#include "ias_logging.h"
#include "ias_l1r_private.h"
#include "hdf5_hl.h"        /* HDF5 LT functions */


/******************************************************************************
Name: get_HDF_file_data_type

Purpose: For an IAS_DATA_TYPE, returns the matching HDF5 type for the file.

Returns:
    Returns the matching type
******************************************************************************/
static hid_t get_HDF_file_data_type
(
    IAS_DATA_TYPE data_type     /* I: input data type to convert */
)
{
    hid_t h5_type;

    switch (data_type)
    {
        case IAS_BYTE: h5_type = H5T_STD_U8LE; break;
        case IAS_I2:   h5_type = H5T_STD_I16LE; break;
        case IAS_UI2:  h5_type = H5T_STD_U16LE; break;
        case IAS_I4:   h5_type = H5T_STD_I32LE; break;
        case IAS_UI4:  h5_type = H5T_STD_U32LE; break;
        case IAS_R4:   h5_type = H5T_IEEE_F32LE; break;
        case IAS_R8:   h5_type = H5T_IEEE_F64LE; break;
        case IAS_CHAR: h5_type = H5T_C_S1; break;
        case IAS_UCHAR: h5_type = H5T_NATIVE_UCHAR; break;
        default:
            IAS_LOG_ERROR("Unsupported data type of %d", data_type);
            h5_type = -1;
            break;
    }

    return h5_type;
}

/******************************************************************************
Name: get_HDF_memory_data_type

Purpose: For an IAS_DATA_TYPE, returns the matching HDF5 type for the data in
    memory.

Returns:
    Returns the matching type
******************************************************************************/
static hid_t get_HDF_memory_data_type
(
    IAS_DATA_TYPE data_type     /* I: input data type to convert */
)
{
    hid_t h5_type;

    switch (data_type)
    {
        case IAS_BYTE: h5_type = H5T_NATIVE_UINT8; break;
        case IAS_I2:   h5_type = H5T_NATIVE_INT16; break;
        case IAS_UI2:  h5_type = H5T_NATIVE_UINT16; break;
        case IAS_I4:   h5_type = H5T_NATIVE_INT32; break;
        case IAS_UI4:  h5_type = H5T_NATIVE_UINT32; break;
        case IAS_R4:   h5_type = H5T_NATIVE_FLOAT; break;
        case IAS_R8:   h5_type = H5T_NATIVE_DOUBLE; break;
        case IAS_CHAR: h5_type = H5T_NATIVE_CHAR; break;
        case IAS_UCHAR: h5_type = H5T_NATIVE_UCHAR; break;
        default:
            IAS_LOG_ERROR("Unsupported data type of %d", data_type);
            h5_type = -1;
            break;
    }

    return h5_type;
}

/******************************************************************************
Name: get_data_type

Purpose: For an HDF5 data type, return the matching IAS_DATA_TYPE.

Returns:
    Returns the matching type, or ERROR if there is no match.
******************************************************************************/
static IAS_DATA_TYPE get_data_type
(
    hid_t h5_data_type      /* I: HDF5 data type to convert */
)
{
    if (H5Tequal(h5_data_type, H5T_STD_U8LE)
        || H5Tequal(h5_data_type, H5T_NATIVE_UINT8))
    {
        return IAS_BYTE;
    }
    else if (H5Tequal(h5_data_type, H5T_STD_I16LE)
        || H5Tequal(h5_data_type, H5T_NATIVE_INT16))
    {
        return IAS_I2;
    }
    else if (H5Tequal(h5_data_type, H5T_NATIVE_UINT16)
        || H5Tequal(h5_data_type, H5T_STD_U16LE))
    {
        return IAS_UI2;
    }
    else if (H5Tequal(h5_data_type, H5T_NATIVE_INT32)
        || H5Tequal(h5_data_type, H5T_STD_I32LE))
    {
        return IAS_I4;
    }
    else if (H5Tequal(h5_data_type, H5T_NATIVE_UINT32)
        || H5Tequal(h5_data_type, H5T_STD_U32LE))
    {
        return IAS_UI4;
    }
    else if (H5Tequal(h5_data_type, H5T_NATIVE_FLOAT)
        || H5Tequal(h5_data_type, H5T_IEEE_F32LE))
    {
        return IAS_R4;
    }
    else if (H5Tequal(h5_data_type, H5T_NATIVE_DOUBLE)
        || H5Tequal(h5_data_type, H5T_IEEE_F64LE))
    {
        return IAS_R8;
    }
    else if (H5Tequal(h5_data_type, H5T_NATIVE_CHAR)
        || H5Tequal(h5_data_type, H5T_C_S1))
    {
        return IAS_CHAR;
    }
    else if (H5Tequal(h5_data_type, H5T_NATIVE_UCHAR))
    {
        return IAS_UCHAR;
    }
    else 
    {
        IAS_LOG_ERROR("Unsupported H5 datatype %d", h5_data_type);
        return ERROR;
    }
}

/******************************************************************************
Name: ias_l1r_open_band

Purpose: Open a band of the image.

Returns:
    pointer to the L1R_BAND_IO band if the band was opened successfully
    NULL if the band was not opened successfully
******************************************************************************/
L1R_BAND_IO *ias_l1r_open_band
(
    L1RIO *l1r_file,        /* I: L1R info structure for the image */
    int band_number,        /* I: band number to open */
    IAS_DATA_TYPE *data_type,/* I/O: data type for the imagery */
    int *scas,              /* I/O: SCAs to include in the band (1 if an
                                    SCA combined image is needed) */
    int *lines,             /* I/O: lines in the band (input if writing a new 
                                    band, output if opening an existing band) */
    int *samples            /* I/O: samples in the band (input if writing a new
                                    band, output if opening an existing band) */
)
{
    L1R_BAND_IO *hdf_band;

    hsize_t dims[3];
    int create_dataset = 0;
    char band_name[30];

    /* verify the input parameter is valid */
    if (!l1r_file)
    {
        IAS_LOG_ERROR("NULL pointer provided for the L1R file");
        return NULL;
    }

    /* verify the band number is legal */
    if (band_number < 1)
    {
        IAS_LOG_ERROR("Attempting to open illegal band number %d for %s",
                      band_number, l1r_file->filename);
        return NULL;
    }

    /* search the linked list to see if band_number appears;
       if the band is already open, issue an error */
    GET_OBJECT_FOR_EACH_ENTRY(hdf_band, &l1r_file->open_bands,
                              L1R_BAND_IO, node)
    {
        if (hdf_band->number == band_number)
        {
            IAS_LOG_ERROR("Trying to open band %d when band %d is "
                          "already open for %s", band_number, 
                          hdf_band->number, l1r_file->filename);
            return NULL;
        }
    }

    /* allocate space for the data structure */
    hdf_band = malloc(sizeof(*hdf_band));
    if (!hdf_band)
    {
        IAS_LOG_ERROR("Allocating memory");
        return NULL;
    }

    /* temporary initialization of L1R_BAND_IO struct elements */
    hdf_band->id = -1;
    hdf_band->dataspace_id = -1;
    hdf_band->number = -1;
    hdf_band->l1r_file = l1r_file;

    /* create the band dataset name */
    snprintf(band_name, sizeof(band_name), "/B%02d", band_number);

    /* if opening in the band in update mode, determine whether the dataset
       needs to be created by searching for it in the band dataset cache */
    if (l1r_file->access_mode == IAS_UPDATE)
    {
        BAND_DATASET_CACHE_NODE *current;
        int found = 0;

        GET_OBJECT_FOR_EACH_ENTRY(current, &l1r_file->band_datasets,
                BAND_DATASET_CACHE_NODE, node)
        {
            if (current->band_number == band_number)
            {
                found = 1;
                break;
            }
        }

        /* if the band wasn't found, flag that it needs to be created */
        if (!found)
            create_dataset = 1;
    }

    if ((l1r_file->access_mode == IAS_WRITE) || create_dataset)
    {
        hid_t file_data_type;

        /* verify the number of SCAs is valid */
        if (*scas < 1)
        {
            IAS_LOG_ERROR("Attempting to open band %d with %d SCAs for file %s",
                band_number, *scas, l1r_file->filename);
            free(hdf_band);
            return NULL;
        }

        /* convert the requested data type to the HDF5 file and memory data
           types */
        file_data_type = get_HDF_file_data_type(*data_type);
        if (file_data_type < 0)
        {
            IAS_LOG_ERROR("Unsupported data type of %d encountered when "
                "opening band %d of file %s", *data_type, band_number,
                l1r_file->filename);
            free(hdf_band);
            return NULL;
        }
        hdf_band->memory_data_type 
            = H5Tcopy(get_HDF_memory_data_type(*data_type));
        if (hdf_band->memory_data_type < 0)
        {
            IAS_LOG_ERROR("Failed to create the HDF5 type when opening band %d"
                          " of file %s", band_number, l1r_file->filename);
            free(hdf_band);
            return NULL;
        }

        /* create the dataspace for the band */
        dims[0] = *scas;
        dims[1] = *lines;
        dims[2] = *samples;
        hdf_band->dataspace_id = H5Screate_simple(3, dims, NULL);
        if (hdf_band->dataspace_id < 0)
        {
            /* failed to create the dataspace */
            IAS_LOG_ERROR("Failed creating dataspace with %lld SCAs, "
                  "%lld lines, and %lld samples for file %s, band %d", dims[0],
                  dims[1], dims[2], l1r_file->filename, band_number);
            H5Tclose(hdf_band->memory_data_type);
            free(hdf_band);
            return NULL;
        }

        /* add the band to the list of datasets in the file */
        if (ias_l1r_add_band_to_cache(l1r_file, band_number, *data_type, 
                *scas, *lines, *samples) != SUCCESS)
        {
            IAS_LOG_ERROR("Allocating memory for band cache");
            H5Tclose(hdf_band->memory_data_type);
            hdf_band->memory_data_type = -1;
            H5Sclose(hdf_band->dataspace_id);
            free(hdf_band);
            return NULL;
        }

        /* create the dataset.  Note that attempting to create a dataset that
           already exists results in an error.  If that happens, an error is
           returned to the calling routine since the dataset shouldn't exist
           when we get here unless the application has a bug. */
        hdf_band->id = H5Dcreate(l1r_file->file_id, band_name,
                                file_data_type, hdf_band->dataspace_id,
                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (hdf_band->id < 0)
        {
            /* error creating dataset */
            IAS_LOG_ERROR("Failed creating dataset for band %d of file %s",
                          band_number, l1r_file->filename);
            H5Tclose(hdf_band->memory_data_type);
            hdf_band->memory_data_type = -1;
            H5Sclose(hdf_band->dataspace_id);
            free(hdf_band);
            return NULL;
        }

        /* Create the CLASS attribute on the dataset with a value of IMAGE
           to let tools like hdfview know that it is an image */
        if (H5LTset_attribute_string(hdf_band->id, band_name, "CLASS", "IMAGE")
            < 0)
        {
            IAS_LOG_ERROR("Failed creating CLASS attribute for band %d of "
                "file %s", band_number, l1r_file->filename);
            H5Tclose(hdf_band->memory_data_type);
            hdf_band->memory_data_type = -1;
            H5Sclose(hdf_band->dataspace_id);
            free(hdf_band);
            return NULL;
        }
    }
    else
    {
        hid_t file_data_type;

        /* read mode or in update mode and the dataset already exists */
        hdf_band->id = H5Dopen(l1r_file->file_id, band_name, 
                                    H5P_DEFAULT);
        if (hdf_band->id < 0)
        {
            IAS_LOG_ERROR("Failed opening dataset for band %d of file %s",
                          band_number, l1r_file->filename);
            free(hdf_band);
            return NULL;
        } 
        
        /* get the dataspace for the dataset */
        hdf_band->dataspace_id = H5Dget_space(hdf_band->id);
        if (hdf_band->dataspace_id < 0)
        {
            IAS_LOG_ERROR("Unable to open dataspace for band %d of file %s",
                          band_number, l1r_file->filename);
            H5Dclose(hdf_band->id);
            free(hdf_band);
            return NULL;
        }

        /* verify the dataspace indicates 3 dimensions */
        if ((H5Sget_simple_extent_ndims(hdf_band->dataspace_id) != 3)
            || (H5Sget_simple_extent_dims(hdf_band->dataspace_id, dims,
                NULL) != 3))
        {
            IAS_LOG_ERROR("Incorrect dimensions found for band %d of file %s",
                          band_number, l1r_file->filename);
            H5Sclose(hdf_band->dataspace_id);
            hdf_band->dataspace_id = -1;
            H5Dclose(hdf_band->id);
            free(hdf_band);
            return NULL;
        }
        *scas = dims[0];
        *lines = dims[1];
        *samples = dims[2];

        /* get the data type in the dataset.  Note that the H5Dget_type call
           is needed separately for the file and memory data types since
           they could be different in write mode and they need to be kept
           consistent for closing when the band is closed. */
        file_data_type = H5Dget_type(hdf_band->id);
        if (file_data_type < 0)
        {
            IAS_LOG_ERROR("Unable to retrieve data type for band %d of file %s",
                          band_number, l1r_file->filename);
            H5Sclose(hdf_band->dataspace_id);
            hdf_band->dataspace_id = -1;
            H5Dclose(hdf_band->id);
            free(hdf_band);
            return NULL;
        }

        /* convert data type to the interface type */
        *data_type = get_data_type(file_data_type);
        if (*data_type == ERROR)
        {
            IAS_LOG_ERROR("Unsupported data type for band %d of file %s",
                          band_number, l1r_file->filename);
            H5Sclose(hdf_band->dataspace_id);
            hdf_band->dataspace_id = -1;
            H5Dclose(hdf_band->id);
            free(hdf_band);
            return NULL;
        }

        /* get the memory data type */
        hdf_band->memory_data_type
            = H5Tcopy(get_HDF_memory_data_type(*data_type));
        if (hdf_band->memory_data_type < 0)
        {
            IAS_LOG_ERROR("Unable to determine memory data type for band %d "
                          "of file %s", band_number, l1r_file->filename);
            H5Sclose(hdf_band->dataspace_id);
            hdf_band->dataspace_id = -1;
            H5Dclose(hdf_band->id);
            free(hdf_band);
            return NULL;
        }
    }

    /* cache info about the current band */
    hdf_band->number = band_number;
    hdf_band->scas = *scas;
    hdf_band->lines = *lines;
    hdf_band->samples = *samples;

    /* add the band to the linked list of open bands */
    ias_linked_list_add_tail(&l1r_file->open_bands, &hdf_band->node);

    return (L1R_BAND_IO*)hdf_band;
}
