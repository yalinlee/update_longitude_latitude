/******************************************************************************
Name: ias_l1g_add_band_to_cache (not static, but private to library)
      ias_l1g_get_band_count
      ias_l1g_get_band_list
      ias_l1g_get_band_size
      ias_l1g_is_band_present
      ias_l1g_open_image

Purpose: Contains ias_l1g_open_image and other related routines

******************************************************************************/
#include <stdlib.h>         /* for malloc */
#include <string.h>
#include "hdf5.h"
#include "hdf5_hl.h"        /* HDF5 LT functions */
#include "ias_types.h"      /* IAS_DATA_TYPE, IAS_ACCESS_MODE definitions */
#include "ias_const.h"      /* SUCCESS/ERROR definitions */
#include "ias_logging.h"
#include "ias_miscellaneous.h" /* check_file_type routine */
#include "ias_l1g.h"        /* L1G structure definition */
#include "ias_l1g_private.h"


#define FILE_FORMAT_VERSION_NAME "File Format Version"
/* name for the file format version attribute */

#define L1G_FILE_TYPE "L1G File"
/* name for the file type attribute */

/******************************************************************************
Name: initialize_l1g

Purpose: Initializes the contents of an L1GIO structure

Returns:
    nothing
******************************************************************************/
static void initialize_l1g
(
    L1GIO *l1g_file      /* O: HDF IO structure */
)
{
    memset(l1g_file, 0, sizeof(*l1g_file));
    l1g_file->file_format_version = FILE_FORMAT_VERSION;
    l1g_file->filename = NULL;
    l1g_file->access_mode = -1;
    l1g_file->file_id = -1;
    l1g_file->file_metadata_present = FALSE;
    l1g_file->band_metadata_valid = FALSE;
    ias_linked_list_initialize_node(&l1g_file->band_datasets);
    ias_linked_list_initialize_node(&l1g_file->band_metadata);
    ias_linked_list_initialize_node(&l1g_file->open_bands);
}

/******************************************************************************
Name: ias_l1g_add_band_to_cache

Purpose: Adds the size information for a band to the band dataset cache.

Returns:
    SUCCESS if the band was successfully added to the cache
    ERROR if the band was not added to the cache
******************************************************************************/
int ias_l1g_add_band_to_cache
(
    L1GIO *l1g_file,        /* I: pointer to HDF5 file structure */
    int band_number,        /* I: band number being added */
    IAS_DATA_TYPE data_type,/* I: data type in the band */
    int sca_count,          /* I: number of scas in the band */
    int line_count,         /* I: number of lines in the band */
    int sample_count        /* I: number of samples in the band */
)
{
    BAND_DATASET_CACHE_NODE *band;

    /* verify the input parameter is valid */
    if (!l1g_file)
    {
        IAS_LOG_ERROR("NULL pointer provided for the L1G file");
        return ERROR;
    }

    /* allocate and initialize a band dataset cache node */
    band = malloc(sizeof(*band));
    if (!band)
    {
        IAS_LOG_ERROR("Allocating memory for band cache");
        return ERROR;
    }
    ias_linked_list_initialize_node(&band->node);

    /* populate the cache node */
    band->band_number = band_number;
    band->data_type = data_type;
    band->sca_count = sca_count;
    band->line_count = line_count;
    band->sample_count = sample_count;

    /* add the cache node to the end of the cache linked list */
    ias_linked_list_add_tail(&l1g_file->band_datasets, &band->node);

    return SUCCESS;
}

/******************************************************************************
Name: ias_l1g_get_band_count

Purpose: Returns the number of bands present in the file

Returns:
    The number of bands present in the file (or ERROR in the unlikely case
    that NULL is passed in for the L1G file)
******************************************************************************/
int ias_l1g_get_band_count
(
    L1GIO *l1g_file         /* I: L1G info structure for the image */
)
{
    /* verify the input parameter is valid */
    if (!l1g_file)
    {
        IAS_LOG_ERROR("NULL pointer provided for the L1G file");
        return ERROR;
    }

    return ias_linked_list_count_nodes(&l1g_file->band_datasets);
}

/******************************************************************************
Name: find_band

Purpose: Searches the band dataset cache for the requested band and returns
    a pointer to the cache entry.

Returns:
    A pointer to the band dataset cache node if found, NULL otherwise
******************************************************************************/
static BAND_DATASET_CACHE_NODE *find_band
(
    L1GIO *l1g_file,        /* I: file to search for the band */
    int band_number         /* I: band number to find */
)
{
    BAND_DATASET_CACHE_NODE *current;

    /* search the metadata cache for the correct band */
    GET_OBJECT_FOR_EACH_ENTRY(current, &l1g_file->band_datasets,
                BAND_DATASET_CACHE_NODE, node)
    {
        if (band_number == current->band_number)
            return current;
    }

    /* the band wasn't found */
    return NULL;
}

/******************************************************************************
Name: ias_l1g_get_band_list

Purpose: returns the list of band numbers present in an open L1G file.

Returns:
    SUCCESS if the band list size is large enough for all bands found
    ERROR if the band list size was too small for all bands
******************************************************************************/
int ias_l1g_get_band_list
(
    L1GIO *l1g_file,            /* I: file to search for the band */
    int *band_number_list,      /* O: array of band numbers found in file */
    int band_number_list_size,  /* I: size of band_number_list array */
    int *number_of_bands        /* O: pointer to the number of bands returned
                                      in the band_number_list */
)
{
    BAND_DATASET_CACHE_NODE *current;
    int band_count = 0;
    *number_of_bands = 0;

    GET_OBJECT_FOR_EACH_ENTRY(current, &l1g_file->band_datasets,
                BAND_DATASET_CACHE_NODE, node)
    {
        if (band_count < band_number_list_size)
        {
            band_number_list[band_count] = current->band_number;
        }
        band_count++;
    }

    if (band_count > band_number_list_size)
    {
        IAS_LOG_ERROR("Number of bands found = %d and list size is only %d", 
                      band_count, band_number_list_size);
        return ERROR;
    }

    *number_of_bands = band_count;
    return SUCCESS;
}


/******************************************************************************
Name: ias_l1g_is_band_present

Purpose: Allows checking whether a band is present in the file.

Returns:
    1 if the band is present
    0 if the band is not present

Notes:
    - This routine is included in this file to allow the find_band routine
      to remain static.
******************************************************************************/
int ias_l1g_is_band_present
(
    L1GIO *l1g_file,        /* I: L1G info structure for the image */
    int band_number         /* I: band number to check */
)
{
    BAND_DATASET_CACHE_NODE *current;

    current = find_band(l1g_file, band_number);
    if (current)
        return 1;
    else
        return 0;
}

/******************************************************************************
Name: ias_l1g_get_band_size

Purpose: For a given band number, return the size related information (data
    type, number of scas, lines, and samples).

Returns:
    SUCCESS if the band size information was successfully retrieved
    ERROR if the band size information could not be retrieved

Notes:
    - This routine is included in this file to allow the find_band routine
      to remain static.
******************************************************************************/
int ias_l1g_get_band_size
(
    L1GIO *l1g_file,        /* I: L1G info structure for the image */
    int band_number,        /* I: band number to get information for */
    IAS_DATA_TYPE *data_type, /* O: data type for the imagery */
    int *sca_count,         /* O: SCAs in the band */
    int *line_count,        /* O: lines in the band */
    int *sample_count       /* O: samples in the band */
)
{
    BAND_DATASET_CACHE_NODE *current;

    /* search the metadata for the correct band */
    current = find_band(l1g_file, band_number);
    if (current)
    {
        /* the band was found, so return the proper data */
        *data_type = current->data_type;
        *sca_count = current->sca_count;
        *line_count = current->line_count;
        *sample_count = current->sample_count;

        return SUCCESS;
    }

    /* the band wasn't found */
    return ERROR;
    
}

/******************************************************************************
Name: iterate_datasets

Purpose: This is a callback function for the H5Literate call done when an
    image is opened in read or update mode.  Its purpose is to discover what
    datasets are present in the file being opened.

Returns:
    As required by the HDF5 library, returns -1 if an error occurs or 0 if
    successful.
******************************************************************************/
static herr_t iterate_datasets
(
    hid_t group_id,             /* I: HDF5 group id that is being iterated */
    const char *member_name,    /* I: name of the current dataset */
    const H5L_info_t *info,     /* I: info about the link */
    void *operator_data         /* I: L1GIO pointer for file being iterated */
)
{
    L1GIO *l1g_file = (L1GIO *)operator_data;

    /* verify the input parameter is valid */
    if (!l1g_file)
    {
        IAS_LOG_ERROR("NULL pointer provided for the L1G file");
        return -1;
    }

    if (strcmp(member_name, FILE_METADATA_TABLE) == 0)
    {
        /* the current dataset is the file metadata, so flag it as present */
        l1g_file->file_metadata_present = TRUE;
    }
    else if (strcmp(member_name, BAND_METADATA_TABLE) == 0)
    {
        /* the current dataset is the band metadata, so flag it as present */
        l1g_file->band_metadata_in_file = 1;
    }
    else if (member_name[0] == 'B')
    {
        /* the dataset begins with a 'B' so assume it is a band imagery 
           dataset */
        IAS_DATA_TYPE data_type;
        int band_number;
        int sca_count;
        int line_count;
        int sample_count;
        int saved_access_mode;
        L1G_BAND_IO *l1g_band;

        /* parse the band number from the name */
        if (sscanf(member_name, "B%d", &band_number) != 1)
        {
            IAS_LOG_ERROR("Unrecognized dataset name of %s in file %s",
                          member_name, l1g_file->filename);
            return -1;
        }

        /* save the access mode and temporarily set it to read mode since
           trying to use open_band in update mode will cause problems */
        saved_access_mode = l1g_file->access_mode;
        l1g_file->access_mode = IAS_READ;

        /* open the band to get the size info for the band */
        l1g_band = ias_l1g_open_band(l1g_file, band_number, &data_type,
                              &sca_count, &line_count, &sample_count);
        /* restore the access mode */
        l1g_file->access_mode = saved_access_mode;
        if (l1g_band == NULL)
        {
            /* open band failed, so return an error */
            return -1;
        }

        /* close the band again since only one band can be open at a time */
        ias_l1g_close_band(l1g_band);

        /* add the band size info to the band cache */
        if (ias_l1g_add_band_to_cache(l1g_file, band_number, data_type,
                    sca_count, line_count, sample_count) != SUCCESS)
        {
            /* adding the data to the cache failed, so stop iterating */
            return -1;
        }
    }
    else
    {
        /* an unexpected dataset was encountered.  Just provide a warning
           about it and allow iterating to continue. */
        IAS_LOG_WARNING("Unrecognized dataset name '%s' in %s", member_name,
                        l1g_file->filename);
    }
    return 0;
}

/******************************************************************************
Name: ias_l1g_open_image

Purpose: Open the specified 1G image

Returns:
    NULL if an error occurred
    pointer to an L1GIO structure used to access the image if succeeded

******************************************************************************/
L1GIO *ias_l1g_open_image
(
    const char *image_filename, /* I: Path & name of the 1G image file */
    IAS_ACCESS_MODE access_mode /* I: Requested access mode of image */
)
{
    L1GIO *l1g_file = NULL;    /* structure for the HDF5 file */
    int file_format_version[1] = {FILE_FORMAT_VERSION};

    /* allocate space for the data structure */
    l1g_file = malloc(sizeof(*l1g_file));
    if (!l1g_file)
    {
        IAS_LOG_ERROR("Allocating memory for L1G file structure");
        return NULL;
    }

    /* initialize some fields in the l1g_file structure */
    initialize_l1g(l1g_file);
    l1g_file->access_mode = access_mode;

    /* save the filename */
    l1g_file->filename = strdup(image_filename);
    if (!l1g_file->filename)
    {
        free(l1g_file);
        IAS_LOG_ERROR("Allocating memory for filename");
        return NULL;
    }

    if (access_mode == IAS_WRITE)
    {
        /* file metadata is not present in a newly written file */
        l1g_file->file_metadata_present = FALSE;
        l1g_file->band_metadata_valid = TRUE;
        l1g_file->band_metadata_in_file = FALSE;

        /* open the HDF5 file in write mode, truncating the original file */
        l1g_file->file_id = H5Fcreate(image_filename, H5F_ACC_TRUNC, 
                                      H5P_DEFAULT, H5P_DEFAULT);
        if (l1g_file->file_id < 0)
        {
            free(l1g_file->filename);
            free(l1g_file);
            IAS_LOG_ERROR("Opening file %s for writing", image_filename);
            return NULL;
        }

        /* create and set the file format version attribute */
        l1g_file->file_format_version = FILE_FORMAT_VERSION;
        if (H5LTset_attribute_int(l1g_file->file_id, "/",
                    "File Format Version", file_format_version, 1) < 0)
        {
            H5Fclose(l1g_file->file_id);
            free(l1g_file->filename);
            free(l1g_file);
            IAS_LOG_ERROR("Writing format version to file %s", image_filename);
            return NULL;
        }

        /* create and set the file type attribute */
        if (H5LTset_attribute_string(l1g_file->file_id, "/",
                              IAS_FILE_TYPE_ATTRIBUTE, L1G_FILE_TYPE) < 0)
        {
            H5Fclose(l1g_file->file_id);
            free(l1g_file->filename);
            free(l1g_file);
            IAS_LOG_ERROR("Writing file type attribute to file %s",
                          image_filename);
            return NULL;
        }
    }
    else
    {
        /* opening an existing in read or update mode */
        int mode;
        int rank;
        hsize_t dims[1];
        H5T_class_t class;
        size_t size;
        hid_t root_group;

        if (access_mode == IAS_UPDATE)
            mode = H5F_ACC_RDWR;
        else
            mode = H5F_ACC_RDONLY;

        /* open the HDF5 file, which is assumed to already exist.  If
           the open fails, it either does not exist, or if does exist,
           it cannot be opened by this function */
        l1g_file->file_id = H5Fopen(image_filename, mode, H5P_DEFAULT);
        if (l1g_file->file_id < 0)
        {
            free(l1g_file->filename);
            free(l1g_file);
            IAS_LOG_ERROR("File %s either does not exist or it cannot be "
                "opened", image_filename);
            return NULL;
        }

        /* verify the file format version attribute is the expected type and
           size */
        if ((H5LTget_attribute_ndims(l1g_file->file_id, "/",
                                    FILE_FORMAT_VERSION_NAME, &rank) < 0)
             || (rank != 1)
             || (H5LTget_attribute_info(l1g_file->file_id, "/",
                                   FILE_FORMAT_VERSION_NAME,
                                   dims, &class, &size) < 0)
             || (dims[0] != 1) || (class != H5T_INTEGER) 
             || (size != sizeof(int)))
        {
            H5Fclose(l1g_file->file_id);
            free(l1g_file->filename);
            free(l1g_file);
            IAS_LOG_ERROR("Unexpected characteristics for the file format "
                          "version in %s", image_filename);
            return NULL;
        }

        /* read the file format version from the file */
        if (H5LTget_attribute_int(l1g_file->file_id, "/",
                                  FILE_FORMAT_VERSION_NAME,
                                  &l1g_file->file_format_version) < 0)
        {
            H5Fclose(l1g_file->file_id);
            free(l1g_file->filename);
            free(l1g_file);
            IAS_LOG_ERROR("Reading file format version from %s",
                          image_filename);
            return NULL;
        }

        /* open the root group so it can be iterated over */
        root_group = H5Gopen(l1g_file->file_id, "/", H5P_DEFAULT);
        if (root_group < 0)
        {
            H5Fclose(l1g_file->file_id);
            free(l1g_file->filename);
            free(l1g_file);
            IAS_LOG_ERROR("Opening root group in %s", image_filename);
            return NULL;
        }

        /* discover what is included in the file that is being reopened */
        if (H5Literate(root_group, H5_INDEX_NAME, H5_ITER_NATIVE, NULL,
                       iterate_datasets, l1g_file) < 0)
        {
            IAS_LOG_ERROR("Identifying bands in the file %s", 
                          l1g_file->filename);
            H5Gclose(root_group);
            H5Fclose(l1g_file->file_id);
            BAND_DATASET_LINKED_LIST_DELETE(&l1g_file->band_datasets);
            free(l1g_file->filename);
            free(l1g_file);
            return NULL;
        }

        H5Gclose(root_group);
    }

    return l1g_file;
}

/*************************************************************************

NAME: ias_l1g_is_l1g_file

PURPOSE: Determine weather the file an L1G file 

RETURNS: 1 (TRUE) -- is an L1G file
         0 (FALSE) -- is not an L1G file

**************************************************************************/
int ias_l1g_is_l1g_file
(
    const char *l1g_filename /* I: HDF input file name                    */
)
{
    return ias_misc_check_file_type(l1g_filename, L1G_FILE_TYPE);
}
