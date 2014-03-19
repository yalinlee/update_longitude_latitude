/*-----------------------------------------------------------------------------

NAME: ias_l0r.c

PURPOSE: Implementation of functions used for initialization of L0R datasets

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ias_logging.h"
#include "ias_l0r.h"
#include "ias_l0r_hdf.h"
#include "ias_types.h"
#include "ias_l0r_version.h"

/******************************************************************************
 ias_l0r private routines
******************************************************************************/
/******************************************************************************
 NAME: ias_l0r_cleanup_string_bank

 PURPOSE: Internal function to cleanup all of the HDF type IDs allocated
          for strings.

 RETURNS: SUCCESS- Cleanup completed
          ERROR- A failure occurred when performing cleanup
******************************************************************************/
int ias_l0r_cleanup_string_bank
(
    HDFIO *hdfio_ptr /* I: structure for the file used in I/O */
)
{
    int i = 0;
    herr_t hdf_status = -1;
    int error_count = 0;

    for (i = 0; i < IAS_L0R_MAX_NUMBER_OF_STRINGS; i++)
    {
        hdf_status = H5Tclose(hdfio_ptr->type_id_file_mta_strings[i]);
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR("Unable to close datatype");
            error_count++;
        }
        hdf_status = H5Tclose(hdfio_ptr->type_id_interval_mta_strings[i]);
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR("Unable to close datatype");
            error_count++;
        }
        hdf_status = H5Tclose(hdfio_ptr->type_id_scene_mta_strings[i]);
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR("Unable to close datatype");
            error_count++;
        }
    }
    if (error_count > 0)
    {
        return ERROR;
    }
    return SUCCESS;
}


/******************************************************************************
 NAME: ias_l0r_close

 PURPOSE: Terminate access to L0R data and
           clean up any resources used for access.

 RETURNS: SUCCESS- Access terminated
          ERROR- A failure occurred when terminating access
******************************************************************************/
int ias_l0r_close
(
    L0RIO *l0r /* I: structure for the file used in I/O */
)
{
    HDFIO  *hdfio_ptr = NULL;
    int band_count;
    int band_list[IAS_MAX_TOTAL_BANDS];
    int error_count = 0;
    int i = 0;

    /* verify the pointer points to something */
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Null pointer passed");
        return ERROR;
    }
    hdfio_ptr = l0r;

    /* get the normal band list */
    if (ias_sat_attr_get_sensor_band_numbers(IAS_MAX_SENSORS, IAS_NORMAL_BAND,
            0, band_list, IAS_MAX_TOTAL_BANDS, &band_count) != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to get normal band number list");
        return ERROR;
    }

    for (i = 0; i < band_count; i++)
    {
        if (ias_l0r_close_band(l0r, band_list[i]) == ERROR)
        {
            error_count += 1;
            IAS_LOG_ERROR("Error closing normal band %i",i);
        }
    }

    /* get the blind band list */
    if (ias_sat_attr_get_sensor_band_numbers(IAS_MAX_SENSORS, IAS_BLIND_BAND,
            0, band_list, IAS_MAX_TOTAL_BANDS, &band_count) != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to get blind band number list");
        return ERROR;
    }

    for (i = 0; i < band_count; i++)
    {
        if (ias_l0r_close_band(l0r, band_list[i]) == ERROR)
        {
            error_count += 1;
            IAS_LOG_ERROR("Error closing blind band %i",i);
        }
    }

    /* get the VRP band list */
    if (ias_sat_attr_get_sensor_band_numbers(IAS_MAX_SENSORS, IAS_VRP_BAND,
            0, band_list, IAS_MAX_TOTAL_BANDS, &band_count) != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to get VRP band number list");
        return ERROR;
    }

    for (i = 0; i < band_count; i++)
    {
        if (ias_l0r_close_band(l0r, band_list[i]) == ERROR)
        {
            error_count += 1;
            IAS_LOG_ERROR("Error closing VRP band %i",i);
        }
    }

    /* close any header resources that may still be open */
    if (ias_l0r_close_header(l0r) == ERROR)
    {
        IAS_LOG_ERROR("Error closing header data resources");
        error_count += 1;
    }

    /* close any ancillary resources that may still be open */
    if (ias_l0r_close_ancillary(l0r) == ERROR)
    {
        IAS_LOG_ERROR("Error closing ancillary data resources");
        error_count += 1;
    }

    /* close any metadata resources that may still be open */
    if (ias_l0r_close_metadata(l0r) == ERROR)
    {
        IAS_LOG_ERROR("Error closing metadata resources");
        error_count += 1;
    }

    /* free the memory used to hold data used with the L0Ra files */
    if (hdfio_ptr != NULL)
    {
        /* first free the memory allocated BAND_INFO */
        free(hdfio_ptr->band_info);
        /* then free the rest of the structure */
        free(hdfio_ptr);
        hdfio_ptr = NULL;
    }

    if (error_count > 0)
    {
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_get_version_number

 PURPOSE: Gets the version number of the L0R library

 RETURNS: SUCCESS- Version retrieved
          ERROR- Unable to retrieve version
******************************************************************************/
int ias_l0r_get_version_number
(
    unsigned int *version_number  /* O: The library version number */
)
{
    if (version_number == NULL)
    {
        IAS_LOG_ERROR("Null pointer passed");
        return ERROR;
    }
    else
    {
        *version_number = IAS_L0R_LIBRARY_VERSION;
        return SUCCESS;
    }
}

/******************************************************************************
 NAME: ias_l0r_open

 PURPOSE: Initialize access to L0R data and resources used for access.

 RETURNS: pointer for the data structure: if open succeeded
          NULL: if the open failed.

 NOTES:
******************************************************************************/
L0RIO* ias_l0r_open
(
    const char *interval_id, /* I: Landsat interval ID or time used
                                                     in l0r naming */
    const char *path, /* I: path to the l0r location */
    const IAS_L0R_BAND_COMPRESSION compression /* I: flag indicating
        image datasets should  be compressed */
)
{
    int i;
    int status;
    static int types_set = ERROR;
    HDFIO *hdfio_ptr = NULL;
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;

    if (interval_id == NULL || path == NULL)
    {
        IAS_LOG_ERROR("Interval ID and/or Path are NULL");
        return NULL;
    }

    landsat8_attributes = ias_sat_attr_get_attributes();
    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Error getting attributes");
        return NULL;
    }
    hdfio_ptr =  malloc(sizeof(HDFIO));

    if (hdfio_ptr == NULL)
    {
        IAS_LOG_ERROR("Error allocating resources with malloc");
        return NULL;
    }

    /* Allocate memory for the BAND_INFO structure */
    hdfio_ptr->band_info = (BAND_INFO*)malloc(sizeof(BAND_INFO) *
        landsat8_attributes->total_bands);

    if (hdfio_ptr->band_info == NULL)
    {
        IAS_LOG_ERROR("Error allocating resources with malloc");
        free(hdfio_ptr);
        return NULL;
    }

    for (i = 0; i < landsat8_attributes->total_bands; i++)
    {
        hdfio_ptr->band_info[i].file_id = -1;
        hdfio_ptr->band_info[i].access_mode = -1;
        hdfio_ptr->band_info[i].group_id = -1;

        hdfio_ptr->band_info[i].image_dataset_id = -1;
        hdfio_ptr->band_info[i].image_dataspace_id = -1;

        hdfio_ptr->band_info[i].offset_dataset_id = -1;
        hdfio_ptr->band_info[i].offset_dataspace_id = -1;
    }

    hdfio_ptr->compression = compression;

    /* initialize the ids used with ancillary data */
    hdfio_ptr->file_id_ancillary  = -1;
    hdfio_ptr->access_mode_ancillary = -1;
    hdfio_ptr->table_id_attitude = -1;
    hdfio_ptr->table_id_attitude_filter = -1;
    hdfio_ptr->table_id_ephemeris = -1;
    hdfio_ptr->table_id_gps_position = -1;
    hdfio_ptr->table_id_gps_range = -1;
    hdfio_ptr->table_id_imu = -1;
    hdfio_ptr->table_id_imu_latency = -1;
    hdfio_ptr->table_id_telemetry_group_3 = -1;
    hdfio_ptr->table_id_telemetry_group_4 = -1;
    hdfio_ptr->table_id_telemetry_group_5 = -1;
    hdfio_ptr->table_id_tirs_telemetry = -1;
    hdfio_ptr->table_id_star_tracker_centroid = -1;
    hdfio_ptr->table_id_star_tracker_quaternion = -1;
    hdfio_ptr->table_id_temperatures_gyro = -1;
    hdfio_ptr->table_id_temperatures_oli_tirs = -1;

    /* initialize the ids used with complex types */
    hdfio_ptr->type_id_gyro_sample = -1;
    /* initialize the ids used for string types */
    for (i = 0; i < IAS_L0R_MAX_NUMBER_OF_STRINGS; i++)
    {
        hdfio_ptr->type_id_file_mta_strings[i] = -1;
        hdfio_ptr->type_id_interval_mta_strings[i] = -1;
        hdfio_ptr->type_id_scene_mta_strings[i] = -1;
    }

    /* initialize the metadata ids */
    hdfio_ptr->file_id_metadata   = -1;
    hdfio_ptr->access_mode_metadata = -1;
    hdfio_ptr->table_id_mta_file     = -1;
    hdfio_ptr->table_id_mta_interval = -1;
    hdfio_ptr->table_id_mta_scene    = -1;

    hdfio_ptr->file_id_header = -1;
    hdfio_ptr->access_mode_header = -1;
    hdfio_ptr->table_id_oli_frame_header = -1;
    hdfio_ptr->table_id_oli_image_header = -1;
    hdfio_ptr->table_id_tirs_frame_header = -1;

    /* category inits only need to be set once per execution, set them on
       the first init  */
    if (types_set == ERROR)
    {
         /* Create a complex data type to use for each gyro sample.
            Need to do this before calling ias_l0r_anc_init below
            because it needs that type id.  The fields present
            in the type get setup inside the init function */
        hdfio_ptr->type_id_gyro_sample = H5Tcreate(H5T_COMPOUND,
            sizeof(IAS_L0R_IMU_SAMPLE));

        status = ias_l0r_anc_init(hdfio_ptr);
        if (status == ERROR)
        {
            IAS_LOG_ERROR("Unable to set Ancillary types");
            free(hdfio_ptr->band_info);
            free(hdfio_ptr);
            return NULL;
        }
        status = ias_l0r_header_init();
        if (status == ERROR)
        {
            IAS_LOG_ERROR("Unable to set Header types");
            free(hdfio_ptr->band_info);
            free(hdfio_ptr);
            return NULL;
        }

         /* Create data types to use for each string.
            Need to do this before calling ias_l0r_mta_init below
            because it needs those type ids.  The fields present
            in the types get resized inside the init function. */
        for (i = 0; i < IAS_L0R_MAX_NUMBER_OF_STRINGS; i++)
        {
            hdfio_ptr->type_id_file_mta_strings[i] = 
                H5Tcopy(H5T_C_S1);
            if (hdfio_ptr->type_id_file_mta_strings[i] < 0)
            {
                IAS_LOG_ERROR("Unable to create datatype");
                ias_l0r_cleanup_string_bank(hdfio_ptr);
                free(hdfio_ptr->band_info);
                free(hdfio_ptr);
                return NULL;
            }
            hdfio_ptr->type_id_interval_mta_strings[i] =
                H5Tcopy(H5T_C_S1);
            if (hdfio_ptr->type_id_interval_mta_strings[i] < 0)
            {
                IAS_LOG_ERROR("Unable to create datatype");
                ias_l0r_cleanup_string_bank(hdfio_ptr);
                free(hdfio_ptr->band_info);
                free(hdfio_ptr);
                return NULL;
            }
            hdfio_ptr->type_id_scene_mta_strings[i] =
                H5Tcopy(H5T_C_S1);
            if ( hdfio_ptr->type_id_scene_mta_strings[i] < 0)
            {
                IAS_LOG_ERROR("Unable to create datatype");
                ias_l0r_cleanup_string_bank(hdfio_ptr);
                free(hdfio_ptr->band_info);
                free(hdfio_ptr);
                return NULL;
            }
        }
        status = ias_l0r_mta_init(hdfio_ptr);
        if (status == ERROR)
        {
            IAS_LOG_ERROR("Unable to set Metadata types");
            ias_l0r_cleanup_string_bank(hdfio_ptr);
            free(hdfio_ptr->band_info);
            free(hdfio_ptr);
            return NULL;
        }
        types_set = SUCCESS;
    }

    /* populate the struct with info related to the L0Ra files */
    strncpy(hdfio_ptr->l0r_name_prefix, interval_id, IAS_L0R_FILE_NAME_LENGTH);

    struct stat stat_buf;
    if (stat(path, &stat_buf) < 0)
    {
        IAS_LOG_ERROR("%s does not exist", path);
        free(hdfio_ptr->band_info);
        free(hdfio_ptr);
        return NULL;
    }
    strncpy(hdfio_ptr->path, path, PATH_MAX);

    return hdfio_ptr;
}

/******************************************************************************
 NAME: ias_l0r_open_from_filename

 PURPOSE: Convenience function which allows the calling of ias_l0r_open with
          path and interval id in a single string

 RETURNS: values are the same as for ias_l0r_open
******************************************************************************/
L0RIO* ias_l0r_open_from_filename
(
    const char *path_id, /* I: path where the L0R data is located and
                          Landsat interval id or time used with the L0R file
                          names combined and separated by "/" */
    const IAS_L0R_BAND_COMPRESSION compression /* I: flag indicating image
                                             datasets should be compressed */
)
{
    char *token = NULL; /* location of the character in the string that
                          separates the path from the id */
    char path[PATH_MAX] = "";
    char id[IAS_L0R_INTERVAL_ID_LENGTH+1] = "";
    int split_location = 0; /* location of character which divides
                               path from ID */


    if (path_id == NULL)
    {
        IAS_LOG_ERROR("NULL pointer for path_id passed");
        return NULL;
    }

    /* find the location of the final "/" which separates the path and id */
    token = strrchr(path_id,'/');

    if (token == NULL)
    {
        IAS_LOG_ERROR("Invalid path_id.  No slash was found");
        return NULL;
    }

    /* check to make sure the slash found wasn't a trailing slash */
    if (*(token+1) == '\0')
    {
        IAS_LOG_ERROR("No trailing slashes are allowed.  A trailing slash "
            "indicates a directory. ");
        return NULL;
    }

    split_location = token-path_id;
    if (split_location > PATH_MAX)
    {
        IAS_LOG_ERROR("Path id too long. Max is %i", PATH_MAX);
        return NULL;
    }

    /* extract the path */
    strncat(path, path_id, split_location);
    /* extract the id */
    strncat(id, token+1, IAS_L0R_INTERVAL_ID_LENGTH);

    return ias_l0r_open(id,path,compression);
}

/******************************************************************************
 NAME: ias_l0r_present

 PURPOSE: Convenience function which determines if any L0R files for the named
          path and id exist.  A L0R dataset could already been opened,
          but if no data has yet been written, no files will yet exist

 RETURNS: TRUE: A L0R file does exist for the named path and id
          FALSE: No files exist for the named path and id, or an error occurred
******************************************************************************/
int ias_l0r_present
(
    const char *path_id  /* I: path where to check if L0R data exist and
                          id used with the L0R file names combined and
                          separated by "/" */
)
{
    const IAS_SATELLITE_ATTRIBUTES *landsat8_attributes = NULL;

    int i = 0;
    struct stat stat_buf;
    char temp[PATH_MAX + IAS_L0R_INTERVAL_ID_LENGTH + 1] = "";


    if ( path_id == NULL)
    {
        IAS_LOG_ERROR("ID/Path is NULL");
        return FALSE;
    }

    landsat8_attributes = ias_sat_attr_get_attributes();
    if (landsat8_attributes == NULL)
    {
        IAS_LOG_ERROR("Error getting attributes");
        return FALSE;
    }

    /* Check for the Ancillary file */
    snprintf(temp, PATH_MAX + IAS_L0R_INTERVAL_ID_LENGTH,
        "%s_ANC.h5", path_id);
    if (stat(temp, &stat_buf) == 0)
    {
       return TRUE;
    }

    /* Check for the Metadata file */
    strcpy(temp,"");
    snprintf(temp, PATH_MAX + IAS_L0R_INTERVAL_ID_LENGTH,
        "%s_MTA.h5", path_id);
    if (stat(temp, &stat_buf) == 0)
    {
       return TRUE;
    }

    /* Check for any of the band files */
    for( i = 0; i < landsat8_attributes->total_bands; i++)
    {
        strcpy(temp,"");
        snprintf(temp, PATH_MAX + IAS_L0R_INTERVAL_ID_LENGTH,
            "%s_B%d.h5", path_id, i);
        if (stat(temp, &stat_buf) == 0)
        {
           return TRUE;
        }
    }

    return FALSE;
}
