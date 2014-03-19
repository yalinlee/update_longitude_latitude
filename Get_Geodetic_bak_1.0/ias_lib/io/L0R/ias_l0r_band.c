/*-----------------------------------------------------------------------------

NAME: ias_l0r_band.c

PURPOSE: Functions related to the reading and writing of image and offset
         data contained in the L0R.

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

/* project included headers */
#include "ias_logging.h"
#include "ias_l0r.h"
#include "ias_l0r_hdf.h"
#include "ias_types.h"
#include "ias_satellite_attributes.h"

/******************************************************************************
 Constants
******************************************************************************/
#define DATASET_NAME_BAND "Image"
#define DATASET_NAME_BAND_VRP "VRP"
#define DATASET_NAME_BAND_OFFSET "Detector_Offsets"
#define FILE_COMPRESSION_LEVEL 4 /* Valid values are 0 to 9 */

static const char BAND_FILENAME_FORMAT[] = "%s_B%d.h5";

typedef enum
{
  IAS_L0R_IMAGE_DATASET,
  IAS_L0R_OFFSET_DATASET
}IAS_L0R_BAND_DATASET;

/*******************************************************************************
*private routine prototypes
*******************************************************************************/

static int ias_l0r_image_read
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    const int sca_start, /* I: First SCA to read */
    const int sca_count, /* I:Number of SCAs to read */
    const uint32_t line_start, /* I: First line to read */
    const uint32_t line_count, /* I: Number of lines the read*/
    const uint32_t pixel_start, /* I: First pixel to read */
    const uint32_t pixel_count, /* I: Number of pixels to read */
    const IAS_L0R_BAND_DATASET dataset_type, /* I: Image or Offset dataset */
    uint16_t *image /* O: pointer to memory where data should be placed  */
);

static int ias_l0r_image_write
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    const int sca_index, /* I: First SCA to write */
    const int sca_count, /* I: Number of SCAs to write */
    const uint32_t line_start, /* I: First line to write */
    const uint32_t line_count, /* I: Numbers lines to write */
    const uint32_t pixel_start, /* I: First pixel to write */
    const uint32_t pixel_count, /* I: Number of pixels to write */
    const IAS_L0R_BAND_DATASET dataset_type, /* I: Image or Offset dataset */
    const uint16_t *image  /* I: pointer to memory where data to write
                                 is located  */
);

static int ias_l0r_truncate_image
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to truncate */
    const int sca_count, /* I: Number of SCAs in band */
    const uint32_t pixel_count, /* I: Number of pixels in each SCA */
    const IAS_L0R_BAND_DATASET dataset_type /* I: Image or Offset dataset */
);

/*******************************************************************************
 Subroutine definitions
******************************************************************************/
/******************************************************************************
 NAME: ias_l0r_establish_band_file

 PURPOSE: Initializes access to or creates the specified band file

 RETURNS: SUCCESS- Access had been established
          ERROR- Could not establish access
******************************************************************************/
static int ias_l0r_establish_band_file
(
    HDFIO *hdfio_ptr, /* I: Pointer used in I/O */
    const int band_number, /* I: Band number to establish file for */
    const int create_if_absent /* I: Create the file if it does not exist */
)
{
    char *image_appendix = "_B00.h5\0";
    char image_filename[PATH_MAX];
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;
    int band_index = ERROR;
    int blind_band_index = ERROR;
    int normal_band_index = ERROR;
    int status = 0;
    int vrp_band_index = ERROR;

    landsat8_attributes = ias_sat_attr_get_attributes();
    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get sat attributes");
        return ERROR;
    }

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    /*  Populate the band index fields.  For all but the first, if
        the specified band number is 0 set the index to 0 as well.
        This will be the case when that type is not applicable to
        the passed in band number */
    band_index = band_attributes->band_index;
    if (band_attributes->normal_band_number != 0)
    {
        normal_band_index = ias_sat_attr_convert_band_number_to_index(
            band_attributes->normal_band_number);
    }
    else
    {
        normal_band_index = 0;
    }
    if (band_attributes->blind_band_number != 0)
    {
        blind_band_index = ias_sat_attr_convert_band_number_to_index(
            band_attributes->blind_band_number);
    }
    else
    {
        blind_band_index = 0;
    }
    if (band_attributes->vrp_band_number != 0)
    {
        vrp_band_index = ias_sat_attr_convert_band_number_to_index(
            band_attributes->vrp_band_number);
    }
    else
    {
        vrp_band_index = 0;
    }

    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Unable to convert band number to band index");
        return ERROR;
    }
    if (normal_band_index == ERROR)
    {
        IAS_LOG_ERROR("Unable to convert normal band number to band index");
        return ERROR;
    }
    if (blind_band_index == ERROR)
    {
        IAS_LOG_ERROR("Unable to convert blind band number to band index");
        return ERROR;
    }
    if (vrp_band_index == ERROR)
    {
        IAS_LOG_ERROR("Unable to convert vrp band number to band index");
        return ERROR;
    }


    /* verify the length of the prefix + appendix and a NULL are not longer than
    *the space allocated for the filename. */
    if ((strlen(hdfio_ptr->l0r_name_prefix) + strlen(image_appendix) + 1)
        > IAS_L0R_FILE_NAME_LENGTH)
    {
        IAS_LOG_ERROR("Image file name is too long");
        return ERROR;
    }

    /* if the band is for a vrp band, use the normal band number for the file
    *  L0R data stores the VRP in the same file as the image data */
    if (ias_sat_attr_band_classification_matches(band_number, IAS_VRP_BAND))
    {
         if (hdfio_ptr->band_info[normal_band_index].file_id > 0)
         {
           hdfio_ptr->band_info[band_index].file_id =
            hdfio_ptr->band_info[normal_band_index].file_id;
           return SUCCESS;
         }
            snprintf(image_filename, IAS_L0R_FILE_NAME_LENGTH,
                BAND_FILENAME_FORMAT, hdfio_ptr->l0r_name_prefix,
                band_attributes->normal_band_number);
    }
    else if (ias_sat_attr_band_classification_matches(band_number,
                IAS_VRP_BAND | IAS_BLIND_BAND))
    {
        if (hdfio_ptr->band_info[blind_band_index].file_id > 0)
        {
           hdfio_ptr->band_info[band_index].file_id =
               hdfio_ptr->band_info[blind_band_index].file_id;
           return SUCCESS;
        }
        snprintf(image_filename, IAS_L0R_FILE_NAME_LENGTH,
            BAND_FILENAME_FORMAT, hdfio_ptr->l0r_name_prefix,
            band_attributes->blind_band_number);
    }
    else if (ias_sat_attr_band_classification_matches(band_number,
                IAS_NORMAL_BAND)
             || ias_sat_attr_band_classification_matches(band_number,
                IAS_BLIND_BAND))
    {
        /* if the file is already opened for the VRP band */
        if (band_attributes->vrp_band_number != 0 &&
            hdfio_ptr->band_info[vrp_band_index].file_id > 0)
        {
           hdfio_ptr->band_info[band_index].file_id =
               hdfio_ptr->band_info[vrp_band_index].file_id;
           return SUCCESS;
        }
        snprintf(image_filename, IAS_L0R_FILE_NAME_LENGTH,
            BAND_FILENAME_FORMAT, hdfio_ptr->l0r_name_prefix,
            band_number);
    }
    else
    {
        snprintf(image_filename, IAS_L0R_FILE_NAME_LENGTH,
            BAND_FILENAME_FORMAT, hdfio_ptr->l0r_name_prefix,
            band_number);
    }

    status = ias_l0r_hdf_establish_file(hdfio_ptr->path,
        image_filename, &hdfio_ptr->band_info[band_index].file_id,
        hdfio_ptr->band_info[band_index].access_mode, create_if_absent);
    return status;
}

/******************************************************************************
 NAME: ias_l0r_establish_band_dataset

 PURPOSE: Initializes access to the specified dataset

 RETURNS: SUCCESS- Access had been established
          ERROR- Could not establish access
******************************************************************************/
static int ias_l0r_establish_band_dataset
(
    HDFIO *hdfio_ptr, /* I: Pointer used in I/O */
    const int band_number, /* I: Band number of the dataset */
    const int create_if_absent, /* I: true or false to create if the dataset
                                  does not exist*/
    const IAS_L0R_BAND_DATASET dataset_type /* I: Indicates whether this is
                                                a image or offset dataset */
)
{
    char full_dataset_name[PATH_MAX];
    herr_t status;
    hid_t *dataset_id = NULL;
    hid_t *dataspace_id = NULL;
    hid_t dataset_access_properties;
    hid_t cparms;
    hsize_t chunk_dims[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t dimArray[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t max_dimArray[IAS_L0R_IMAGE_DIMENSIONS];

    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;

    const int CHUNK_NUM_LINES = 128;

    band_attributes = ias_sat_attr_get_band_attributes(band_number);
    landsat8_attributes = ias_sat_attr_get_attributes();

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }
    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get sat attributes");
        return ERROR;
    }

    /*  Declare the File ID only after the band attributes have been
        retrieved */
    hid_t file_id = hdfio_ptr->band_info[band_attributes->band_index].file_id;

    dimArray[IAS_L0R_IMAGE_DIMENSION_SCA] = band_attributes->scas;
    dimArray[IAS_L0R_IMAGE_DIMENSION_LINE] = 1;
    dimArray[IAS_L0R_IMAGE_DIMENSION_DETECTOR] =
        band_attributes->detectors_per_sca;

    max_dimArray[IAS_L0R_IMAGE_DIMENSION_SCA] = band_attributes->scas;
    max_dimArray[IAS_L0R_IMAGE_DIMENSION_LINE] = H5S_UNLIMITED;
    max_dimArray[IAS_L0R_IMAGE_DIMENSION_DETECTOR] =
        band_attributes->detectors_per_sca;

    chunk_dims[IAS_L0R_IMAGE_DIMENSION_SCA] = 1;
    /*  Set a reasonable number of lines to be part of a chunk */
    chunk_dims[IAS_L0R_IMAGE_DIMENSION_LINE] = 128;
    chunk_dims[IAS_L0R_IMAGE_DIMENSION_DETECTOR] =
        band_attributes->detectors_per_sca;

    switch(dataset_type)
    {
        case IAS_L0R_IMAGE_DATASET:
            dataset_id = &hdfio_ptr->band_info[band_attributes->band_index].
                image_dataset_id;
            dataspace_id = &hdfio_ptr->band_info[band_attributes->band_index].
                image_dataspace_id;
            break;
        case IAS_L0R_OFFSET_DATASET:
            dataset_id = &hdfio_ptr->band_info[band_attributes->band_index].
                offset_dataset_id;
            dataspace_id = &hdfio_ptr->band_info[band_attributes->band_index].
                offset_dataspace_id;
            break;
        default:
            IAS_LOG_ERROR("Unknown dataset_type received");
            return ERROR;
    }

    /*  Check to make sure the dataset_id is a valid HDF ID */
    if (*dataset_id >= 0)
    {
        return SUCCESS;
    }

    if (dataset_type == IAS_L0R_OFFSET_DATASET)
    {
        snprintf (full_dataset_name, sizeof(full_dataset_name), "%s",
            DATASET_NAME_BAND_OFFSET);
    }
    else if (ias_sat_attr_band_classification_matches(band_number,
                IAS_VRP_BAND)
             || ias_sat_attr_band_classification_matches(band_number,
                IAS_VRP_BAND | IAS_BLIND_BAND))
    {
        snprintf (full_dataset_name, sizeof(full_dataset_name), "%s",
            DATASET_NAME_BAND_VRP);
    }
    else
    {
        snprintf (full_dataset_name, sizeof(full_dataset_name), "%s",
            DATASET_NAME_BAND);
    }
    full_dataset_name[sizeof(full_dataset_name) - 1] = '\0';

    if (ias_l0r_hdf_object_exists(file_id, full_dataset_name) ==
             IAS_L0R_OBJECT_NOT_FOUND )
    {
        uint16_t fill_value = 0; /* Fill used in the L0R data before written */

        if (create_if_absent != TRUE)
        {
            *dataset_id = -1;
            *dataspace_id = -1;
            return SUCCESS;
        }

        /* create the dataspace */
        *dataspace_id = H5Screate_simple(IAS_L0R_IMAGE_DIMENSIONS,
                                        dimArray, max_dimArray);
        if (*dataspace_id < 0)
        {
            IAS_LOG_ERROR("Error creating the dataspace");
            return ERROR;
        }

        cparms = H5Pcreate (H5P_DATASET_CREATE);
        if (cparms < 0)
        {
            IAS_LOG_ERROR("Call to H5Pcreate failed");
            H5Sclose(*dataspace_id);
            return ERROR;
        }
         
        /* Set datasets to be filled at creation time */
        status = H5Pset_fill_time(cparms, H5D_FILL_TIME_ALLOC);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error setting fill time");
            H5Sclose(*dataspace_id);
            H5Pclose(cparms);
            return ERROR;
        }
     
        status = H5Pset_fill_value(cparms, H5T_NATIVE_UINT16, &fill_value);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error setting fill value");
            H5Sclose(*dataspace_id);
            H5Pclose(cparms);
            return ERROR;
        }

        /* Set the parameters used for chunking */
        status = H5Pset_chunk( cparms, IAS_L0R_IMAGE_DIMENSIONS, chunk_dims);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error setting chunking");
            H5Sclose(*dataspace_id);
            H5Pclose(cparms);
            return ERROR;
        }

        if (hdfio_ptr->compression)
        {
            status = H5Pset_shuffle(cparms);
            if (status < 0)
            {
                IAS_LOG_ERROR("Error calling H5Pset_shuffle");
                H5Sclose(*dataspace_id);
                H5Pclose(cparms);
                return ERROR;
            }

            status = H5Pset_deflate(cparms,FILE_COMPRESSION_LEVEL);
            if (status < 0)
            {
                IAS_LOG_ERROR("Error calling H5Pset_deflate");
                H5Sclose(*dataspace_id);
                H5Pclose(cparms);
                return ERROR;
            }
        }

        *dataset_id = H5Dcreate(file_id, full_dataset_name,
            H5T_STD_U16LE, *dataspace_id, H5P_DEFAULT, cparms, H5P_DEFAULT);
        if (*dataset_id < 0)
        {
            IAS_LOG_ERROR("Error creating the dataset %s for band %d",
                          full_dataset_name, band_number);
            H5Sclose(*dataspace_id);
            H5Pclose(cparms);
            return ERROR;
        }
        status = H5Pclose(cparms);
        if (status < 0)
        {
            IAS_LOG_ERROR("Unable to close property list");
            H5Sclose(*dataspace_id);
            H5Dclose(*dataset_id);
            return ERROR;
        }
        if(strstr(full_dataset_name, DATASET_NAME_BAND) != NULL)
        {
            /* Adding the following attribute to the image bands in the HDF 
               files so hdfview properly formats the image for viewing */
            char attribute_name[] = "CLASS";
            char attribute_value[] = "IMAGE";

            status = H5LTset_attribute_string(file_id, full_dataset_name, 
                attribute_name, attribute_value);
            if (status < 0)
            {
                IAS_LOG_ERROR("Unable to set Image attribute");
                H5Sclose(*dataspace_id);
                H5Dclose(*dataset_id);
                return ERROR;
            }
        }
    }
    else
    {
        /* Adjust chunk cache size so the entire focal plane width can be in 
           cache at once (avoids reloading/decompressing when processing line 
           by line, etc).  */
        dataset_access_properties = H5Pcreate(H5P_DATASET_ACCESS);

        /* make the adjustment for image data */
        if(dataset_type == IAS_L0R_IMAGE_DATASET)
        {
            /* bytes per pixel */
            int bytes_per_pixel = sizeof(uint16_t); 
            int num_detectors;   /* number of detectors across entire focal
                                    plane */
            size_t rdcc_nslots;  /* number of chunk slots in the data chunk 
                                    hash table -- not changed */
            size_t rdcc_nbytes;  /* total size of cache for dataset */
            double rdcc_w0;      /* preemption policy -- not changed */
    
            /* get the default cache values */
            status = H5Pget_chunk_cache(dataset_access_properties, 
                &rdcc_nslots, &rdcc_nbytes, &rdcc_w0);
            if(status < 0)
            {
                IAS_LOG_ERROR("Unable to get default chunk cache settings, "
                    "status = %d", status);
                H5Pclose(dataset_access_properties);
                return ERROR;
            }
    
            /* number of detectors across entire focal plane */
            num_detectors = band_attributes->detectors_per_sca *
                band_attributes->scas;
    
            /* total number of bytes to accommodate all chunks across the focal 
               plane in memory.  Then, multiplying by 2 to increase the total 
               cache size to allow enough room for storing 2 chunks */
            rdcc_nbytes = (bytes_per_pixel * num_detectors * CHUNK_NUM_LINES);
            rdcc_nbytes *= 2; 
            status = H5Pset_chunk_cache(dataset_access_properties, rdcc_nslots, 
                rdcc_nbytes, rdcc_w0);
            if(status < 0)
            {
                IAS_LOG_ERROR("Unable to set chunk cache settings, status = %d",
                    status);
                H5Pclose(dataset_access_properties);
                return ERROR;
            }
        }
    
        *dataset_id = H5Dopen(file_id, full_dataset_name, 
            dataset_access_properties);
        if (*dataset_id < 0)
        {
            IAS_LOG_ERROR("Error opening the dataset %s", full_dataset_name);
            H5Pclose(dataset_access_properties);
            return ERROR;
        }
    
        *dataspace_id = H5Dget_space(*dataset_id);
        if (*dataspace_id < 0)
        {
            IAS_LOG_ERROR("Error getting the dataspace for %s", 
                full_dataset_name);
            H5Pclose(dataset_access_properties);
            H5Dclose(*dataset_id);
            return ERROR;
        }

        status = H5Pclose(dataset_access_properties);
        if (status < 0)
        {
            IAS_LOG_ERROR("Unable to close property list");
            H5Dclose(*dataset_id);
            H5Dclose(*dataspace_id);
            return ERROR;
        }
    }

    return SUCCESS;
}


/******************************************************************************
 NAME: ias_l0r_get_band_dataset_records_count

 PURPOSE: Gets the number of records in the dataset for the band specified

 RETURNS: SUCCESS- Number of records was read
          ERROR- Could not get the number of records
******************************************************************************/
static int ias_l0r_get_band_dataset_records_count
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: Band number of the dataset */
    const IAS_L0R_BAND_DATASET dataset_type, /* I: image or offset dataset */
    int *size             /* O: Number of records in the dataset */
)
{
    HDFIO *hdfio_ptr = NULL;
    herr_t status;
    hid_t *dataset_id = NULL;
    hid_t *dataspace_id = NULL;
    hid_t dim;
    hsize_t dim_max[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t dim_size[IAS_L0R_IMAGE_DIMENSIONS];

    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;

    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    hdfio_ptr =  l0r;

    status = ias_l0r_establish_band_file(hdfio_ptr, band_number, FALSE);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Unable to establish band# %i file",band_number);
        return ERROR;
    }

    /*Empty files are required to not exist. To support that the file id is
    *unset for the case where size is 0.*/
    if (hdfio_ptr->band_info[band_attributes->band_index].file_id < 0)
    {
        *size = 0;
        return SUCCESS;
    }
   
    /*The IAS_BAND_ATTRIBUTES does not treat the OFFSETS dataset as a separate
      band like it does the VRPs. The IAS_L0R_BAND_DATASET enumerated type is
      still used to differentiate between what is being used */
    switch(dataset_type)
    {
        case IAS_L0R_IMAGE_DATASET:
            status = ias_l0r_establish_band_dataset(hdfio_ptr,
                band_number, FALSE, IAS_L0R_IMAGE_DATASET);
            dataset_id = &hdfio_ptr->band_info[band_attributes->band_index].
                image_dataset_id;
            dataspace_id = &hdfio_ptr->band_info[band_attributes->band_index].
                image_dataspace_id;
            break;
        case IAS_L0R_OFFSET_DATASET:
            status = ias_l0r_establish_band_dataset(hdfio_ptr, band_number,
                FALSE, IAS_L0R_OFFSET_DATASET);
            dataset_id = &hdfio_ptr->band_info[band_attributes->band_index].
                offset_dataset_id;
            dataspace_id = &hdfio_ptr->band_info[band_attributes->band_index].
                offset_dataspace_id;
            break;
        default:
            IAS_LOG_ERROR("Unknown datatype received");
            return ERROR;
    }
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing band dataset");
        return ERROR;
    }

    /*To support not creating empty datasets, the case for the file not existing
    *is if the dataset id is unset after establish, which is size 0*/
    if (*dataset_id < 0)
    {
        *size = 0;
        return SUCCESS;
    }

    /*get the dataspace specs*/
    dim = H5Sget_simple_extent_dims(*dataspace_id, dim_size, dim_max);
    if ( dim < 0 )
    {
        IAS_LOG_ERROR("Getting the dataspace size");
        return ERROR;
    }
    *size = dim_size[IAS_L0R_IMAGE_DIMENSION_LINE];
    return SUCCESS;
}

/*******************************************************************************
*public routines
*******************************************************************************/
/******************************************************************************
 NAME: ias_l0r_get_band_records_count

 PURPOSE: Gets the size of the image dataset for the specified band.

 RETURNS: SUCCESS: Size determined
          ERROR: Size not able to be determined
******************************************************************************/
int ias_l0r_get_band_records_count
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: band number */
    int *size             /* O: number of records for the image
                                dataset for this band */
)
{
   return ias_l0r_get_band_dataset_records_count(l0r, band_number,
        IAS_L0R_IMAGE_DATASET, size);
}

/******************************************************************************
 NAME: ias_l0r_get_band_offset_records_count

 PURPOSE: Gets the size of the offset dataset for the specified band.

 RETURNS: SUCCESS: Size determined
          ERROR: Size not able to be determined
******************************************************************************/
int ias_l0r_get_band_offset_records_count
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: band number */
    int *size              /* O: number of records for the offset
                                 dataset for this band */
)
{
    return ias_l0r_get_band_dataset_records_count(l0r,
        band_number, IAS_L0R_OFFSET_DATASET, size);
}

/******************************************************************************
 NAME: ias_l0r_get_band_present

 PURPOSE: Determines if the specified band is present.

 RETURNS: SUCCESS: Able to be determine whether the band is present
          ERROR: Unable to be determine whether the band is present
******************************************************************************/
int ias_l0r_get_band_present
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: band number */
    int *band_present      /* O: Contains 1 if the band is present,
                                 contains 0 if the band is not present */
)
{
    int band_index = ERROR;
    int is_band_already_open;
    int size_of_band_dataset = 0;
    int status = 0;

    HDFIO *hdfio_ptr = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;

    /* Set this right away, in case we error out below */
    *band_present = 0;

    landsat8_attributes = ias_sat_attr_get_attributes();
    band_index = ias_sat_attr_convert_band_number_to_index(band_number);

    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get Sat attributes");
        return ERROR;
    }
    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Unable to get band index");
        return ERROR;
    }

    /* Check the band number is in range */
    if (band_number > landsat8_attributes->total_bands || band_number < 1)
    {
        IAS_LOG_ERROR("Band number %d is not valid", band_number);
        return ERROR;
    }

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    hdfio_ptr =  l0r;

    /* Check if the band is already open */
    if (hdfio_ptr->band_info[band_index].access_mode != -1)
    {
        is_band_already_open = 1;
    }
    else
    {
        is_band_already_open = 0;
        status = ias_l0r_open_band(l0r, band_number, IAS_READ);
        if (status == ERROR)
        {
            IAS_LOG_ERROR("Unable to open band #%i", band_number);
            return ERROR;
        }
    }

    /* Get size of the image dataset for this band */
    status = ias_l0r_get_band_records_count(l0r, band_number,
        &size_of_band_dataset);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Unable to get the number of records for band #%i",
            band_number);
        return ERROR;
    }

    /* Close the band if we opened it */
    if (is_band_already_open == 0)
    {
        status = ias_l0r_close_band(l0r, band_number);
        if (status == ERROR)
        {
            IAS_LOG_ERROR("Unable to close band #%i", band_number);
            return ERROR;
        }
    }

    /* Finally set whether or not the band is present based of the size
       of the image dataset */
    if (size_of_band_dataset > 0)
    {
        *band_present = 1;
    }
    else
    {
        *band_present = 0;
    }
    return SUCCESS;
}


/******************************************************************************
 NAME: ias_l0r_open_band

 PURPOSE: Opens access to the specified band.  The band is opened
          in the access mode specified.

 RETURNS: SUCCESS- Band was opened
          ERROR- Band failed to open
******************************************************************************/
int ias_l0r_open_band
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: band number */
    const IAS_ACCESS_MODE file_access_mode /* I: file access mode */
)
{
    HDFIO *hdfio_ptr = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;
    int band_index = ERROR;

    landsat8_attributes = ias_sat_attr_get_attributes();
    band_index = ias_sat_attr_convert_band_number_to_index(band_number);

    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get Sat attributes");
        return ERROR;
    }
    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Unable to get band index");
        return ERROR;
    }

    /* Check the band number is in range */
    if (band_number > landsat8_attributes->total_bands || band_number < 1)
    {
        IAS_LOG_ERROR("Band number %d is not valid", band_number);
        return ERROR;
    }

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    hdfio_ptr =  l0r;

    if (hdfio_ptr->band_info[band_index].access_mode != -1)
    {
        IAS_LOG_ERROR("File access mode is set already");
        return ERROR;
    }
    hdfio_ptr->band_info[band_index].access_mode =
        file_access_mode;
    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_close_band

 PURPOSE: Closes access to the specified band.

 RETURNS: SUCCESS- Band was closed
          ERROR- Band failed to close
******************************************************************************/
int ias_l0r_close_band
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const int band_number  /* I: band number */
)
{
    HDFIO *hdfio_ptr = NULL;
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;
    herr_t status = -1;
    int band_index = ERROR;
    int blind_band_index = ERROR;
    int error_count = 0;
    int normal_band_index = ERROR;
    int vrp_band_index = ERROR;

    landsat8_attributes = ias_sat_attr_get_attributes();
    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    /* The band must be in the range on one of the types of bands to be valid */
    if (band_number > landsat8_attributes->total_bands || band_number < 1)
    {
        IAS_LOG_ERROR("Band number %d is not valid", band_number);
        return ERROR;
    }

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }
    hdfio_ptr = l0r;

    /*  Populate the band index fields.  For all but the first, if
        the specified band number is 0 set the index to 0 as well.
        This will be the case when that type is not applicable to
        the passed in band number */
    band_index = band_attributes->band_index;
    if (band_attributes->normal_band_number != 0)
    {
        normal_band_index = ias_sat_attr_convert_band_number_to_index(
            band_attributes->normal_band_number);
    }
    else
    {
        normal_band_index = 0;
    }
    if (band_attributes->blind_band_number != 0)
    {
        blind_band_index = ias_sat_attr_convert_band_number_to_index(
            band_attributes->blind_band_number);
    }
    else
    {
        blind_band_index = 0;
    }
    if (band_attributes->vrp_band_number != 0)
    {
        vrp_band_index = ias_sat_attr_convert_band_number_to_index(
            band_attributes->vrp_band_number);
    }
    else
    {
        vrp_band_index = 0;
    }

    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Unable to convert band number to band index");
        return ERROR;
    }
    if (normal_band_index == ERROR)
    {
        IAS_LOG_ERROR("Unable to convert normal band number to band index");
        return ERROR;
    }
    if (blind_band_index == ERROR)
    {
        IAS_LOG_ERROR("Unable to convert blind band number to band index");
        return ERROR;
    }
    if (vrp_band_index == ERROR)
    {
        IAS_LOG_ERROR("Unable to convert vrp band number to band index");
        return ERROR;
    }


    if (hdfio_ptr->band_info[band_index].
        image_dataset_id >= 0)
    {
        status = H5Dclose(hdfio_ptr->band_info[band_index].
            image_dataset_id);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing hdf resources, "
                "dataset_id for band#%i",
                band_number);
            error_count += 1;
        }
        hdfio_ptr->band_info[band_index].
            image_dataset_id = -1;
    }

    if (hdfio_ptr->band_info[band_index].
        offset_dataset_id >= 0)
    {
        status = H5Dclose(hdfio_ptr->band_info[band_index].
            offset_dataset_id);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing hdf resources, "
                "offset_dataset_id for band#%i",
                band_number);
            error_count += 1;
        }
        hdfio_ptr->band_info[band_index].
            offset_dataset_id = -1;
    }

    if (hdfio_ptr->band_info[band_index].
        image_dataspace_id >= 0)
    {
        status = H5Sclose(hdfio_ptr->band_info[band_index].
            image_dataspace_id);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing hdf resources, "
                "dataspace_id for band#%i",
                band_number);
            error_count += 1;
        }
        hdfio_ptr->band_info[band_index].
            image_dataspace_id = -1;
    }

    if (hdfio_ptr->band_info[band_index].
        offset_dataspace_id >= 0)
    {
        status = H5Sclose(hdfio_ptr->band_info[band_index].
            offset_dataspace_id);
        if (status < 0)
        {
           IAS_LOG_ERROR("Error closing hdf resources, offset_dataspace_id "
                "for band #%i",band_number);
           error_count += 1;
        }
        hdfio_ptr->band_info[band_index].
            offset_dataspace_id = -1;
    }

    /* if the band is for a vrp band, use the normal band number for the file
    *  L0R data stores the VRP in the same file as the image data */
    if ( hdfio_ptr->band_info[band_index].group_id > 0)
    {
        status = H5Gclose(hdfio_ptr->band_info[band_index].
            group_id);
        /*At this point the group was either closed or is open for another band
        *so mark the group as closed for this band */
        hdfio_ptr->band_info[band_index].group_id = -1;
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing hdf resources, group_id for band %d",
                band_number);
            error_count += 1;
        }
    }

    /* The VRPs and the normal bands share a file in the L0R data. If one
    * still requires access don't close the file */
    if (hdfio_ptr->band_info[band_index].file_id >= 0)
    {
        if (ias_sat_attr_band_classification_matches(band_number,
                IAS_NORMAL_BAND))
        {
            if (band_attributes->vrp_band_number > 0
                && hdfio_ptr->band_info[vrp_band_index]
                .file_id >= 0)
            {
                status = H5Fflush(hdfio_ptr->band_info
                    [band_index].file_id,
                    H5F_SCOPE_LOCAL);
                if (status < 0)
                {
                   IAS_LOG_ERROR("Error flushing hdf resources,"
                       " file_id for band %d", band_number);
                   error_count += 1;
                }
                hdfio_ptr->band_info[band_index].file_id = -1;
            }
        }
        else if (ias_sat_attr_band_classification_matches(band_number,
                IAS_BLIND_BAND))
        {
            /* if the vrp for the blind band is open too */
            if (band_attributes->vrp_band_number-1 >= 0 &&
                hdfio_ptr->band_info[vrp_band_index].file_id
                > 0)
            {
                status = H5Fflush(hdfio_ptr->band_info
                   [band_index].file_id,
                    H5F_SCOPE_LOCAL);
                if (status < 0)
                {
                   IAS_LOG_ERROR("Error flushing hdf resources,"
                       " file_id for band %d",
                       band_number);
                   error_count += 1;
                }
                hdfio_ptr->band_info
                   [band_index].file_id = -1;
            }
        }
        else if (ias_sat_attr_band_classification_matches(band_number,
                    IAS_VRP_BAND)
                 || ias_sat_attr_band_classification_matches(band_number,
                    IAS_VRP_BAND | IAS_BLIND_BAND))
        {
            if ( (band_attributes->normal_band_number-1 >= 0  &&
                hdfio_ptr->band_info[normal_band_index].file_id > 0 ) ||
                (band_attributes->blind_band_number-1 >= 0 &&
                hdfio_ptr->band_info[blind_band_index].file_id
                > 0))
            {
                status = H5Fflush(hdfio_ptr->band_info[band_index].file_id,
                    H5F_SCOPE_LOCAL);
                if (status < 0)
                {
                   IAS_LOG_ERROR("Error flushing hdf resources,"
                                " file_id for band %d",
                       band_number);
                   error_count += 1;
                }
                hdfio_ptr->band_info[band_index].file_id = -1;
            }
        }
         
        /*If the file was not flushed and set to -1 we can
            safely close the file */
        if (hdfio_ptr->band_info[band_index].file_id >= 0)
        {
            status = H5Fclose(hdfio_ptr->band_info
                [band_index].file_id);
            if (status < 0)
            {
                IAS_LOG_ERROR("Error closing hdf resources,"
                    " file_id for band %d",
                    band_number);
                error_count += 1;
            }
            hdfio_ptr->band_info[band_index].file_id = -1;
        }
    }

    hdfio_ptr->band_info[band_index].access_mode = -1;

    if (error_count == 0)
    {
        return SUCCESS;
    }
    return ERROR;
}

/******************************************************************************
 NAME: ias_l0r_get_band_attributes

 PURPOSE: Gets various attributes for the band specified

 RETURNS: SUCCESS: Attributes retrieved
          ERROR: Unable to retrieve attributes

 NOTE: The band needs to be opened before
       calling this
******************************************************************************/
int ias_l0r_get_band_attributes
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const int band_number, /* I: band number */
    IAS_L0R_BAND_ATTRIBUTES *image_attributes /* O: attribute structure */
)
{
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;

    HDFIO *hdfio_ptr = NULL;
    hsize_t dimMax[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t dimSize[IAS_L0R_IMAGE_DIMENSIONS];
    int dim;
    int status;

    landsat8_attributes = ias_sat_attr_get_attributes();
    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes for band #%i",
            band_number);
        return ERROR;
    }

    if (l0r == NULL || image_attributes == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    hdfio_ptr =  l0r;

    if (band_number < 1 || band_number > landsat8_attributes->total_bands)
    {
        IAS_LOG_ERROR("Band number %i is out of range", band_number);
        return ERROR;
    }
    image_attributes->band_number = band_number;
  
    status = ias_l0r_establish_band_file(hdfio_ptr, band_number, FALSE);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing band #%d file access.  Band must be"
                      " opened before getting the attributes", band_number);
        return ERROR;
    }

    /* if the file doesn't exist establish sets the file_id to -1 to support */
    /* the requirement that empty files/datasets are not to exist */
    if (hdfio_ptr->band_info[band_attributes->band_index].file_id < 0)
    {
       image_attributes->detectors_per_sca = 0;
       image_attributes->sca_count = 0;
       image_attributes->detectors_per_line = 0;
       image_attributes->number_of_lines = 0;
       return SUCCESS;
    }

    /* populate the hdfio_ptr with HDF IDs for access to the image data */
    if (ias_l0r_establish_band_dataset(hdfio_ptr, band_number, FALSE,
        IAS_L0R_IMAGE_DATASET) == ERROR)
    {
        IAS_LOG_ERROR("Error establishing image dataset access");
        return ERROR;
    }
   
    /* To support not creating empty dataspaces,
    if the dataspace id is unset the size is 0 */
    if (hdfio_ptr->band_info[band_attributes->band_index].
        image_dataspace_id < 0)
    {
        image_attributes->detectors_per_sca = 0;
        image_attributes->sca_count = 0;
        image_attributes->detectors_per_line = 0;
        image_attributes->number_of_lines = 0;
    }
    else
    {
        dim = H5Sget_simple_extent_dims(
            hdfio_ptr->band_info[band_attributes->band_index].
            image_dataspace_id, dimSize,
            dimMax);
        if (dim != IAS_L0R_IMAGE_DIMENSIONS)
        {
            IAS_LOG_ERROR("Dataspace is not of the correct dimension");
            return ERROR;
        }

        image_attributes->detectors_per_sca =
            dimSize[IAS_L0R_IMAGE_DIMENSION_DETECTOR];
        image_attributes->sca_count = dimSize[IAS_L0R_IMAGE_DIMENSION_SCA];
        image_attributes->detectors_per_line =
            image_attributes->detectors_per_sca *
            image_attributes->sca_count;
        image_attributes->number_of_lines =
            dimSize[IAS_L0R_IMAGE_DIMENSION_LINE];
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_get_band_lines

 PURPOSE: Reads the lines as specified into the buffer
          passed in.  Buffer space must
          be allocated before calling.  The space must be large enough for the
          line count and line size specified.

 RETURNS: SUCCESS- Image data was read into the buffer
          ERROR- Image data could not be read into the buffer
******************************************************************************/
int ias_l0r_get_band_lines
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: band number */
    const uint32_t line_number_start, /* I: First line to read */
    const int line_count,  /* I: Number of lines to read */
    const int line_size,   /* I: Number of pixels per line for which space
                                 has been allocated for */
    uint16_t *image_lines /* O: Image data, complete lines of data
                                ordered by [SCA][LINE][DETECTOR] */
)
{
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;

    hsize_t count = 0;

    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes for band #%i",
            band_number);
        return ERROR;
    }

    if (line_size < (band_attributes->scas *
                     band_attributes->detectors_per_sca))
    {
        IAS_LOG_ERROR("Line size passed of %i is too small"
                      " A size of at least %i is needed for band %i",
                      line_size, (band_attributes->scas *
                     band_attributes->detectors_per_sca), band_number );
        return ERROR;
    }

    /* modify dimensions for the different sized bands */
    count =  band_attributes->detectors_per_sca;

    return ias_l0r_image_read(l0r, band_number, 0,  band_attributes->scas,
        line_number_start, line_count, 0, count, IAS_L0R_IMAGE_DATASET,
        image_lines);

}

/******************************************************************************
 NAME: ias_l0r_get_top_detector_offsets

 PURPOSE: Reads the offsets before each sensor
          for a band as specified.  Buffer space must be allocated before
          calling.  The space must be large enough for the
          offset size specified.

 RETURNS: SUCCESS- Offsets were read into the buffer
          ERROR- Offsets could not be read into the buffer
******************************************************************************/
int ias_l0r_get_top_detector_offsets
(
    L0RIO *file,           /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    uint16_t *offsets,     /* O: offset data  */
    const int offsets_size /* I: number of offsets which storage has been
                             allocated for */
)
{
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;

    landsat8_attributes = ias_sat_attr_get_attributes();
    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get sat attributes");
        return ERROR;
    }

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    if (ias_sat_attr_band_classification_matches(band_number, IAS_VRP_BAND))
    {
        IAS_LOG_ERROR("You passed band #%d.  This is a VRP band."
            "VRPs can not have offsets", band_number);
        return ERROR;
    }

    if (offsets_size < (band_attributes->scas *
                     band_attributes->detectors_per_sca))
    {
        IAS_LOG_ERROR("Offset size passed of %i is too small"
                      " A size of at least %i is needed for band %i",
                      offsets_size, (band_attributes->scas *
                     band_attributes->detectors_per_sca), band_number );
        return ERROR;
    }

    return ias_l0r_image_read(file, band_number, 0, band_attributes->scas, 0,
        1, 0, band_attributes->detectors_per_sca,
        IAS_L0R_OFFSET_DATASET, offsets);
}

/******************************************************************************
 NAME: ias_l0r_get_bottom_detector_offsets

 PURPOSE: Reads the offsets after each sensor
          for a band as specified.  Buffer space must be allocated before
          calling.  The space must be large enough for the
          offset size specified.

 RETURNS: SUCCESS- Offsets were read into the buffer
          ERROR- Offsets could not be read into the buffer
******************************************************************************/
int ias_l0r_get_bottom_detector_offsets
(
    L0RIO *file,           /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    uint16_t *offsets,     /* O: offset data  */
    const int offsets_size /* I: number of offsets which storage has been
                             allocated for */
)
{
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;

    landsat8_attributes = ias_sat_attr_get_attributes();
    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get sat attributes");
        return ERROR;
    }

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    if (ias_sat_attr_band_classification_matches(band_number, IAS_VRP_BAND))
    {
        IAS_LOG_ERROR("You passed band #%d.  This is a VRP band."
            "VRPs can not have offsets", band_number);
        return ERROR;
    }

    if (offsets_size < (band_attributes->scas *
                     band_attributes->detectors_per_sca))
    {
        IAS_LOG_ERROR("Offset size passed of %i is too small"
                      " A size of at least %i is needed for band %i",
                      offsets_size, (band_attributes->scas *
                     band_attributes->detectors_per_sca), band_number );
        return ERROR;
    }

    return ias_l0r_image_read(file, band_number, 0, band_attributes->scas, 1,
        1, 0, band_attributes->detectors_per_sca,
        IAS_L0R_OFFSET_DATASET, offsets);
}

/******************************************************************************
 NAME: ias_l0r_get_top_detector_offsets_sca

 PURPOSE: Reads the offsets before each sensor
          for a SCA as specified.  Buffer space must be allocated before
          calling.  The space must be large enough for the
          offset size specified.

 RETURNS: SUCCESS- Offsets were read into the buffer
          ERROR- Offsets could not be read into the buffer
******************************************************************************/
int ias_l0r_get_top_detector_offsets_sca
(
    L0RIO *file,           /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    const int sca_index,   /* I: SCA number to read */
    uint16_t *offsets,     /* O: offset data  */
    const int offsets_size /* I: number of offsets which storage has been
                             allocated for */
)
{
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;

    landsat8_attributes = ias_sat_attr_get_attributes();
    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get sat attributes");
        return ERROR;
    }

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    if (ias_sat_attr_band_classification_matches(band_number, IAS_VRP_BAND))
    {
        IAS_LOG_ERROR("You passed band #%d.  This is a VRP band."
            "VRPs can not have offsets", band_number);
        return ERROR;
    }

    if (offsets_size < (band_attributes->detectors_per_sca))
    {
        IAS_LOG_ERROR("Offset size passed of %i is too small"
                      " A size of at least %i is needed for band %i",
                      offsets_size, band_attributes->detectors_per_sca,
                      band_number );
        return ERROR;
    }

    return ias_l0r_image_read(file, band_number, sca_index, 1,
         0, 1, 0, band_attributes->detectors_per_sca,
         IAS_L0R_OFFSET_DATASET, offsets);
}

/******************************************************************************
 NAME: ias_l0r_get_bottom_detector_offsets_sca

 PURPOSE: Reads the offsets after each sensor
          for a SCA as specified.  Buffer space must be allocated before
          calling.  The space must be large enough for the
          offset size specified.

 RETURNS: SUCCESS- Offsets were read into the buffer
          ERROR- Offsets could not be read into the buffer
******************************************************************************/
int ias_l0r_get_bottom_detector_offsets_sca
(
    L0RIO *file,           /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    const int sca_index,   /* I: SCA number to read */
    uint16_t *offsets,     /* O: offset data  */
    const int offsets_size /* I: number of offsets which storage has been
                             allocated for */
)
{
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;

    landsat8_attributes = ias_sat_attr_get_attributes();
    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get sat attributes");
        return ERROR;
    }

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    if (ias_sat_attr_band_classification_matches(band_number, IAS_VRP_BAND))
    {
        IAS_LOG_ERROR("You passed band #%d.  This is a VRP band."
            "VRPs can not have offsets", band_number);
        return ERROR;
    }

    if (offsets_size < (band_attributes->detectors_per_sca))
    {
        IAS_LOG_ERROR("Offset size passed of %i is too small"
                      " A size of at least %i is needed for band %i",
                      offsets_size, band_attributes->detectors_per_sca,
                      band_number );
        return ERROR;
    }

    return ias_l0r_image_read(file, band_number, sca_index, 1,
         1, 1, 0, band_attributes->detectors_per_sca,
         IAS_L0R_OFFSET_DATASET, offsets);
}

/******************************************************************************
 NAME: ias_l0r_get_band_lines_sca

 PURPOSE: Reads the lines within a SCA as specified, into the buffer passed in.
          Buffer space must
          be allocated before calling.  The space must be large enough for the
         line count and line size specified.

 RETURNS: SUCCESS- Image data was read into the buffer
          ERROR- Image data could not be read into the buffer
******************************************************************************/
int ias_l0r_get_band_lines_sca
(
    L0RIO *file,                 /* I: structure for the file used in I/O */
    const int band_number,       /* I: band number to read */
    const int sca_index,         /* I: SCA to read */
    const uint32_t line_start,   /* I: First line to read */
    const uint32_t pixel_start,  /* I: First pixel to read */
    const uint32_t lines_to_read,/* I: Number of lines to read */
    const uint32_t pixels_to_read,/* I: Number of pixels to read */
    uint16_t *lines              /* O: Image data */
)
{
    return ias_l0r_image_read(file, band_number,
        sca_index, 1, line_start, lines_to_read,
        pixel_start, pixels_to_read, IAS_L0R_IMAGE_DATASET, lines);
}

/******************************************************************************
 NAME: ias_l0r_image_read

 PURPOSE: Internal routine to reads the lines within a SCA and pixel
          range as specified.

 RETURNS: SUCCESS- Image data was read into the buffer
          ERROR- Image data could not be read into the buffer
******************************************************************************/
static int ias_l0r_image_read
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    const int sca_start,   /* I: First SCA to read */
    const int sca_count,   /* I:Number of SCAs to read */
    const uint32_t line_start, /* I: First line to read */
    const uint32_t line_count, /* I: Number of lines the read*/
    const uint32_t pixel_start,/* I: First pixel to read */
    const uint32_t pixel_count,/* I: Number of pixels to read */
    const IAS_L0R_BAND_DATASET dataset_type, /* I: Image or Offset dataset */
    uint16_t *image /* O: pointer to memory where data should be placed */
)
{
    int dim;
    int result;

    hid_t *dataset_id = NULL;
    hid_t *dataspace_id = NULL;
    hid_t memoryspace_id;
    hid_t hyperslab_id;
    herr_t status;

    hsize_t startPoint[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t count[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t dimSize[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t dimMax[IAS_L0R_IMAGE_DIMENSIONS];

    startPoint[IAS_L0R_IMAGE_DIMENSION_SCA] = sca_start;
    startPoint[IAS_L0R_IMAGE_DIMENSION_LINE] = line_start;
    startPoint[IAS_L0R_IMAGE_DIMENSION_DETECTOR] = pixel_start;

    count[IAS_L0R_IMAGE_DIMENSION_SCA] = sca_count;
    count[IAS_L0R_IMAGE_DIMENSION_LINE] = line_count;
    count[IAS_L0R_IMAGE_DIMENSION_DETECTOR] = pixel_count;

    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;

    HDFIO *hdfio_ptr = NULL;
    if (l0r == NULL ||
        image == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    hdfio_ptr = l0r;

    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes for band #%i",
            band_number);
        return ERROR;
    }

    if (hdfio_ptr->band_info[band_attributes->band_index].access_mode !=
         IAS_READ &&
         hdfio_ptr->band_info[band_attributes->band_index].access_mode !=
         IAS_WRITE &&
         hdfio_ptr->band_info[band_attributes->band_index].access_mode !=
         IAS_UPDATE )
    {
        IAS_LOG_ERROR("Current access mode, %d, does not allow reading",
            hdfio_ptr->band_info[band_attributes->band_index].access_mode);
        return ERROR;
    }

    status = ias_l0r_establish_band_file(hdfio_ptr, band_number, FALSE);
    if (status == ERROR ||  hdfio_ptr->band_info[band_attributes->band_index].
        file_id <= 0)
    {
        IAS_LOG_ERROR("Error establishing access to band %d file",
            band_number);
        return ERROR;
    }

    /* establish the dataset based on what type of data is being read */
    switch (dataset_type)
    {
        case IAS_L0R_IMAGE_DATASET:
            result = ias_l0r_establish_band_dataset(hdfio_ptr,
                band_number, FALSE, IAS_L0R_IMAGE_DATASET);
            dataset_id = &hdfio_ptr->band_info[band_attributes->band_index].
                image_dataset_id;
            dataspace_id =
                &hdfio_ptr->band_info[band_attributes->band_index].
                image_dataspace_id;
        break;
        case IAS_L0R_OFFSET_DATASET:
            result = ias_l0r_establish_band_dataset(hdfio_ptr,
                band_number, FALSE, IAS_L0R_OFFSET_DATASET);
            dataset_id = &hdfio_ptr->band_info[band_attributes->band_index].
                offset_dataset_id;
            dataspace_id = &hdfio_ptr->band_info[band_attributes->band_index].
                offset_dataspace_id;
        break;
        default:
            IAS_LOG_ERROR("Unknown dataset type specified");
            return ERROR;
    }

    if (result != SUCCESS)
    {
        IAS_LOG_ERROR("Problems establishing dataset");
        return ERROR;
    }

    if (*dataset_id < 0 || *dataspace_id < 0)
    {
        IAS_LOG_ERROR("Attempting to read non-existent data for band %d",
            band_number);
        return ERROR;
    }

    /* get the dataspace specs */
    dim = H5Sget_simple_extent_dims(*dataspace_id, dimSize, dimMax);
    if (dim != IAS_L0R_IMAGE_DIMENSIONS)
    {
        IAS_LOG_ERROR("Dataspace is not of the correct dimension");
        return ERROR;
    }

    /* /get a copy of the dataspace for the hyperslab used in write */
    hyperslab_id = H5Scopy(*dataspace_id);
    if (hyperslab_id < 0)
    {
        IAS_LOG_ERROR("Copying the dataspace");
        return ERROR;
    }

    /* select the area in the file to read from */
    status = H5Sselect_hyperslab(hyperslab_id, H5S_SELECT_SET, startPoint,
       NULL, count, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error selecting hyperslab");
        return ERROR;
    }

    /* get a dataspace for the array containing the line */
    memoryspace_id = H5Screate_simple(IAS_L0R_IMAGE_DIMENSIONS, count, NULL);
    if (memoryspace_id < 0)
    {
        IAS_LOG_ERROR("Error creating dataspace");
        return ERROR;
    }
    /* file_space is set to the dataspace_id to write to the section selected */
    status = H5Dread(*dataset_id, H5T_NATIVE_UINT16, memoryspace_id,
        hyperslab_id, H5P_DEFAULT, image);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error reading dataset %d for band %d", *dataset_id,
            band_number);
        return ERROR;
    }

    /* close the dataspace */
    status = H5Sclose(memoryspace_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error closing the dataspace");
        return ERROR;
    }
    /* close the dataspace */
    status = H5Sclose(hyperslab_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error closing the dataspace");
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_set_band_lines

 PURPOSE: Calls ias_l0r_image_write
          to write the image lines as specified

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_band_lines
(
    L0RIO *file,              /* I: structure for the file used in I/O */
    const int band_number,    /* I: band number to write to */
    const uint32_t line_start,/* I: line to start writing */
    const int line_count,     /* I: the number of lines to write */
    const uint16_t *image     /* I: Data to write, expected to be complete
                                    lines of data ordered by 
                                    [SCA][LINE][DETECTOR] */
)
{
    hsize_t count = 0;
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;

    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    /* modify dimensions for the different sized bands */
    count =  band_attributes->detectors_per_sca;

     return ias_l0r_image_write(file, band_number, 0, band_attributes->scas,
        line_start, line_count, 0, count, IAS_L0R_IMAGE_DATASET, image);
}

/******************************************************************************
 NAME: ias_l0r_truncate_band_lines

 PURPOSE: truncates the band lines to 0 records for overwriting

 RETURNS: SUCCESS- The data was successfully truncated
          ERROR- The data could not be truncated
******************************************************************************/
int ias_l0r_truncate_band_lines
(
    L0RIO *l0r,              /* I: structure used with the L0R data */
    const int band_number    /* I: band number to truncate */
)
{
    hsize_t count = 0;
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;

    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    IAS_LOG_WARNING("Calling truncate function. This function does not reclaim"
        " the space previously used. Depending on the purpose of this data,"
        " running h5repack on the file is advisable");

    /* modify dimensions for the different sized bands */
    count =  band_attributes->detectors_per_sca;

     return ias_l0r_truncate_image(l0r, band_number, band_attributes->scas,
        count, IAS_L0R_IMAGE_DATASET);
}

/******************************************************************************
 NAME: ias_l0r_set_top_detector_offsets

 PURPOSE:Writes the offsets before the detectors
         for the band specified

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_top_detector_offsets
(
    L0RIO *file,            /* I: structure for the file used in I/O */
    const int band_number,  /* I: band number to write to */
    const uint16_t *offsets /* I: Data to write */
)
{
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;

    landsat8_attributes = ias_sat_attr_get_attributes();
    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get sat attributes");
        return ERROR;
    }

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    if (ias_sat_attr_band_classification_matches(band_number, IAS_VRP_BAND))
    {
        IAS_LOG_ERROR("You passed band #%d.  This is a VRP band."
            "VRPs can not have offsets", band_number);
        return ERROR;
    }

    return ias_l0r_image_write(file, band_number, 0, band_attributes->scas, 0,
        1, 0, band_attributes->detectors_per_sca,
        IAS_L0R_OFFSET_DATASET, offsets);
}

/******************************************************************************
 NAME: ias_l0r_set_bottom_detector_offsets

 PURPOSE: Writes the offsets after the detectors
          for the band specified

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_bottom_detector_offsets
(
    L0RIO *file,            /* I: structure for the file used in I/O */
    const int band_number,  /* I: band number to write to */
    const uint16_t *offsets /* I: Data to write */
)
{
    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;

    landsat8_attributes = ias_sat_attr_get_attributes();
    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get sat attributes");
        return ERROR;
    }

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    if (ias_sat_attr_band_classification_matches(band_number, IAS_VRP_BAND))
    {
        IAS_LOG_ERROR("You passed band #%d.  This is a VRP band."
            "VRPs can not have offsets", band_number);
        return ERROR;
    }

    return ias_l0r_image_write(file, band_number, 0, band_attributes->scas, 1,
        1, 0, band_attributes->detectors_per_sca,
        IAS_L0R_OFFSET_DATASET, offsets);
}

/******************************************************************************
 NAME: ias_l0r_set_band_lines_sca

 PURPOSE: Writes image data within a specified sca

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_band_lines_sca
(
    L0RIO *file,                /* I: structure for the file used in I/O */
    const int band_number,      /* I: band number to read */
    const int sca_index,        /* I: SCA to write to */
    const uint32_t line_start,  /* I: line to start writing at */
    const uint32_t pixel_start, /* I: pixel to start writing at */
    const uint32_t line_count,  /* I: number of lines to write */
    const uint32_t pixel_count, /* I: number of pixels to write */
    const uint16_t *image       /* I: Data to write for specified sca */
)
{
     return ias_l0r_image_write(file, band_number, sca_index, 1, line_start,
        line_count, pixel_start, pixel_count, IAS_L0R_IMAGE_DATASET,
        image);
}


/******************************************************************************
 NAME: ias_l0r_image_write

 PURPOSE: Internal routine to write the lines within a SCA and pixel
          range as specified.

 RETURNS: SUCCESS- Image data was written
          ERROR- Image data could not be written
******************************************************************************/
static int ias_l0r_image_write
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    const int sca_index,   /* I: First SCA to write */
    const int sca_count,   /* I: Number of SCAs to write */
    const uint32_t line_start,  /* I: First line to write */
    const uint32_t line_count,  /* I: Numbers lines to write */
    const uint32_t pixel_start, /* I: First pixel to write */
    const uint32_t pixel_count, /* I: Number of pixels to write */
    const IAS_L0R_BAND_DATASET dataset_type, /* I: Image or Offset dataset */
    const uint16_t *image  /* I: pointer to memory where data to write
                                 is located  */
)
{
    int dim;
    int result;
    herr_t status;
    hid_t *dataset_id = NULL;
    hid_t *dataspace_id = NULL;
    hid_t memoryspace_id;
    hid_t hyperslab_id;

    hsize_t startPoint[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t count[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t dimSize[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t dimMax[IAS_L0R_IMAGE_DIMENSIONS];

    startPoint[IAS_L0R_IMAGE_DIMENSION_SCA] = sca_index;
    startPoint[IAS_L0R_IMAGE_DIMENSION_LINE] = line_start;
    startPoint[IAS_L0R_IMAGE_DIMENSION_DETECTOR] = pixel_start;

    count[IAS_L0R_IMAGE_DIMENSION_SCA] = sca_count;
    count[IAS_L0R_IMAGE_DIMENSION_LINE] = line_count;
    count[IAS_L0R_IMAGE_DIMENSION_DETECTOR] = pixel_count;

    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;

    HDFIO *hdfio_ptr = NULL;
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    hdfio_ptr = l0r;

    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes");
        return ERROR;
    }

    if (hdfio_ptr->band_info[band_attributes->band_index].access_mode !=
        IAS_WRITE && hdfio_ptr->band_info[band_attributes->band_index].
        access_mode != IAS_UPDATE)
    {
        IAS_LOG_ERROR("Current access mode %d "
                      "for band %d does not allow writing",
            hdfio_ptr->band_info[band_attributes->band_index].
                access_mode, band_number);
        return ERROR;
    }

    status = ias_l0r_establish_band_file(hdfio_ptr, band_number, TRUE);
    if (status == ERROR || hdfio_ptr->band_info[band_attributes->band_index].
        file_id <= 0)
    {
        IAS_LOG_ERROR("Error establishing access to band %d file",
            band_number);
        return ERROR;
    }

    /* establish the dataset based on what type of data is being written */
    switch (dataset_type)
    {
        case IAS_L0R_IMAGE_DATASET:
            result = ias_l0r_establish_band_dataset(hdfio_ptr,
                band_number, TRUE, IAS_L0R_IMAGE_DATASET);
            dataset_id = &hdfio_ptr->band_info[band_attributes->band_index].
                image_dataset_id;
            dataspace_id = &hdfio_ptr->band_info[band_attributes->band_index].
                image_dataspace_id;
        break;
        case IAS_L0R_OFFSET_DATASET:
            result = ias_l0r_establish_band_dataset(hdfio_ptr,
                band_number, TRUE, IAS_L0R_OFFSET_DATASET);
            dataset_id = &hdfio_ptr->band_info[band_attributes->band_index].
                offset_dataset_id;
            dataspace_id =  &hdfio_ptr->band_info[band_attributes->band_index].
                offset_dataspace_id;
        break;
        default:
            IAS_LOG_ERROR("Problems establishing dataset");
            return ERROR;
    }
    if (result != SUCCESS)
    {
        IAS_LOG_ERROR("Problems establishing dataset");
        return ERROR;
    }

    /* get the dataspace specs */
    dim = H5Sget_simple_extent_dims(*dataspace_id, dimSize, dimMax);
    if (dim != IAS_L0R_IMAGE_DIMENSIONS)
    {
        IAS_LOG_ERROR("Dataspace is not of the correct dimension. %d vs %d",
            dim, IAS_L0R_IMAGE_DIMENSIONS);
        return ERROR;
    }

    /* If the dataset in the file is not large enough to hold the data written*/
    if (dimSize[IAS_L0R_IMAGE_DIMENSION_LINE] < (line_start + line_count))
    {
        dimSize[IAS_L0R_IMAGE_DIMENSION_LINE] = (line_start + line_count);
        status = H5Dset_extent(*dataset_id, dimSize);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error setting extent");
            return ERROR;
        }

        /* get a new dataspace for the dataset with the new extents  */
        status = H5Sclose(*dataspace_id);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing old dataspace");
            return ERROR;
        }
        *dataspace_id = H5Dget_space(*dataset_id);
        if (*dataspace_id < 0)
        {
            IAS_LOG_ERROR("Error getting the dataspace");
            return ERROR;
        }
    }
 
    /* get a copy of the dataspace for the hyperslab used in write */
    hyperslab_id = H5Scopy(*dataspace_id);
    if (hyperslab_id < 0)
    {
        IAS_LOG_ERROR("Copying the dataspace");
        return ERROR;
    }
 
    status = H5Sselect_hyperslab(hyperslab_id, H5S_SELECT_SET, startPoint,
       NULL, count, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error selecting hyperslab");
        return ERROR;
    }
 
    /* get a dataspace for the array containing the line */
    memoryspace_id = H5Screate_simple(IAS_L0R_IMAGE_DIMENSIONS, count, NULL);
    if (memoryspace_id < 0)
    {
        IAS_LOG_ERROR("Error creating dataset");
        return ERROR;
    }
    status = H5Dwrite(*dataset_id, H5T_NATIVE_UINT16, memoryspace_id,
        hyperslab_id, H5P_DEFAULT, image);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error writing dataset %d", *dataset_id);
        H5Sclose(memoryspace_id);
        return ERROR;
    }
 
    /* close the dataspace */
    status = H5Sclose(memoryspace_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error closing the dataspace");
        H5Sclose(hyperslab_id);
        return ERROR;
    }

    /* close the dataspace */
    status = H5Sclose(hyperslab_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error closing the dataspace");
        return ERROR;
    }
 
    return SUCCESS;
}


/******************************************************************************
 NAME: ias_l0r_truncate_image

 PURPOSE: Internal routine to truncate the lines within a SCA and pixel
          range as specified.

 NOTE: The HDF library doesn't allow creating an extent of size 0, so the
       band line size is set to 1.  This means the one line is essentially
       invalid until it is overwritten (perhaps there's a better way?).

 RETURNS: SUCCESS- Image data was truncated
          ERROR- Image data could not be truncated
******************************************************************************/
static int ias_l0r_truncate_image
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to truncate */
    const int sca_count, /* I: Number of SCAs in band */
    const uint32_t pixel_count, /* I: Number of pixels in each SCA */
    const IAS_L0R_BAND_DATASET dataset_type /* I: Image or Offset dataset */
)
{
    int access_mode;
    int band_index;
    int dim;
    int result;
    herr_t status;
    hid_t *dataset_id = NULL;
    hid_t *dataspace_id = NULL;
    hid_t hyperslab_id;

    hsize_t startPoint[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t count[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t dimSize[IAS_L0R_IMAGE_DIMENSIONS];
    hsize_t dimMax[IAS_L0R_IMAGE_DIMENSIONS];

    startPoint[IAS_L0R_IMAGE_DIMENSION_SCA] = 0; /* sca_index */
    startPoint[IAS_L0R_IMAGE_DIMENSION_LINE] = 0; /* line_start */
    startPoint[IAS_L0R_IMAGE_DIMENSION_DETECTOR] = 0; /* pixel_start */

    count[IAS_L0R_IMAGE_DIMENSION_SCA] = sca_count;
    count[IAS_L0R_IMAGE_DIMENSION_LINE] = 1; /* line_count must be > 0 */
    count[IAS_L0R_IMAGE_DIMENSION_DETECTOR] = pixel_count;

    const IAS_BAND_ATTRIBUTES *band_attributes = NULL;

    HDFIO *hdfio_ptr = NULL;
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received for band %d truncation",
           band_number );
        return ERROR;
    }
    hdfio_ptr = l0r;

    band_attributes = ias_sat_attr_get_band_attributes(band_number);

    if (band_attributes == NULL)
    {
        IAS_LOG_ERROR("Unable to get band attributes for band %d truncation",
          band_number );
        return ERROR;
    }

    /* check the access mode of the band  */
    band_index = band_attributes->band_index;
    access_mode = hdfio_ptr->band_info[band_index].access_mode;
    if ( ( access_mode != IAS_WRITE ) && ( access_mode != IAS_UPDATE) )
    {
        IAS_LOG_ERROR("Invalid access mode %d for band %d truncation",
            access_mode, band_number);
        return ERROR;
    }

    status = ias_l0r_establish_band_file(hdfio_ptr, band_number, TRUE);
    if ( (status == ERROR) || 
         (hdfio_ptr->band_info[band_index].file_id <= 0))
    {
        IAS_LOG_ERROR("Cannot establish access to band %d file for truncation.",
            band_number);
        return ERROR;
    }

    /* establish the dataset based on what type of data is being written */
    switch (dataset_type)
    {
        case IAS_L0R_IMAGE_DATASET:
            result = ias_l0r_establish_band_dataset(hdfio_ptr,
                band_number, TRUE, IAS_L0R_IMAGE_DATASET);
            dataset_id = &hdfio_ptr->band_info[band_index].
                image_dataset_id;
            dataspace_id = &hdfio_ptr->band_info[band_index].
                image_dataspace_id;
            break;
        case IAS_L0R_OFFSET_DATASET:
            result = ias_l0r_establish_band_dataset(hdfio_ptr,
                band_number, TRUE, IAS_L0R_OFFSET_DATASET);
            dataset_id = &hdfio_ptr->band_info[band_index].
                offset_dataset_id;
            dataspace_id =  &hdfio_ptr->band_info[band_index].
                offset_dataspace_id;
            break;
        default:
            IAS_LOG_ERROR("Invalid dataset type %d for band %d truncation",
               dataset_type, band_number );
            return ERROR;
    }
    if (result != SUCCESS)
    {
        IAS_LOG_ERROR("Cannot establish dataset for band %d truncation", 
            band_number );
        return ERROR;
    }

    /* get the dataspace specs */
    dim = H5Sget_simple_extent_dims(*dataspace_id, dimSize, dimMax);
    if (dim != IAS_L0R_IMAGE_DIMENSIONS)
    {
        IAS_LOG_ERROR("Dataspace is not of the correct dimension. %d vs %d",
            dim, IAS_L0R_IMAGE_DIMENSIONS);
        return ERROR;
    }

    /* set the number of lines to one (HDF lib does not allow 0) */
    dimSize[IAS_L0R_IMAGE_DIMENSION_LINE] = 1;
    status = H5Dset_extent(*dataset_id, dimSize);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error setting extent for band truncation");
        return ERROR;
    }

    /* get a new dataspace for the dataset with the new extents  */
    status = H5Sclose(*dataspace_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error closing old dataspace");
        return ERROR;
    }

    *dataspace_id = H5Dget_space(*dataset_id);
    if (*dataspace_id < 0)
    {
        IAS_LOG_ERROR("Error getting the dataspace");
        return ERROR;
    }

    /* get a copy of the dataspace for the hyperslab used in write */
    hyperslab_id = H5Scopy(*dataspace_id);
    if (hyperslab_id < 0)
    {
        IAS_LOG_ERROR("Copying the dataspace");
        return ERROR;
    }

    status = H5Sselect_hyperslab(hyperslab_id, H5S_SELECT_SET, startPoint,
       NULL, count, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error selecting hyperslab");
        return ERROR;
    }

    /* close the dataspace */
    status = H5Sclose(hyperslab_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error closing the dataspace");
        return ERROR;
    }

    return SUCCESS;
}

