/*-----------------------------------------------------------------------------

NAME: ias_l0r_hdf.h

PURPOSE: Routines related to using HDF functionality in the L0R Lib

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/

#ifndef IPE_L0R_HDF_H
#define IPE_L0R_HDF_H

#include <hdf5.h>
#include <hdf5_hl.h>
#include <limits.h>

#include "ias_l0r_constants.h"
#include "ias_satellite_attributes.h"

#define IAS_L0R_HDF_PATH_MAX 256

typedef struct
{
    hid_t file_id;        /* HDF ID used to access files */
    int8_t access_mode;  /* read/write/closed */

    hid_t group_id;  /* ID for currently opened group */
    hid_t image_dataset_id; /*ID for currently open dataset in image band file*/
    hid_t image_dataspace_id; /* dataspace ID for currently open dataspace in
                                image band file */
    hid_t offset_dataset_id;  /* dataset ID for offset dataset in
                                image band file */
    hid_t offset_dataspace_id; /* dataspace ID for offset dataset in
                                 image band file */

}BAND_INFO;

typedef struct L0RIO
{
    BAND_INFO *band_info;
    char path[PATH_MAX]; /* Path to L0R dataset */
    IAS_L0R_BAND_COMPRESSION compression; /* Flag indicating compression */

    /*Landsat scene ID or time used to create most of the L0R file name*/
    char l0r_name_prefix[IAS_L0R_FILE_NAME_LENGTH];
    /*ids for the tables for the ancillary data*/
    hid_t file_id_ancillary;        /* Ancillary file ID */
    int8_t access_mode_ancillary;   /*read/write/update (closed -1)*/
    hid_t table_id_attitude;        /* Table ID for acs_attitude */
    hid_t table_id_attitude_filter; /* Table ID for attitude filter data */
    hid_t table_id_ephemeris;       /* Table ID for ephemeris data */
    hid_t table_id_gps_position;    /* Table ID for gps ancillary data */
    hid_t table_id_gps_range;       /* Table ID for gps range data */
    hid_t table_id_imu;             /* Table ID for imu ancillary data */
    hid_t table_id_imu_latency;             /* Table ID for imu latency data */
    hid_t table_id_telemetry_group_3;/* Table ID for OLI Telemetry Group 3 */
    hid_t table_id_telemetry_group_4;/* Table ID for OLI Telemetry Group 4 */
    hid_t table_id_telemetry_group_5;/* Table ID for OLI Telemetry Group 5 */
    hid_t table_id_tirs_telemetry;   /* Table ID for TIRS Telemetry */
    hid_t table_id_star_tracker_centroid;
    /* Table ID for Startracker centroid */
    hid_t table_id_star_tracker_quaternion;
    /* Table ID for Startracker quaternion */
    hid_t table_id_temperatures_gyro;
    /* ID for Gyro temperature ancillary data */
    hid_t table_id_temperatures_oli_tirs;
    /* ID for OLI temperature ancillary data */

    /* Type IDs used for complex data types and string datatypes */
    hid_t type_id_gyro_sample; /* Type ID for complex gyro sample type */
    hid_t type_id_file_mta_strings[IAS_L0R_MAX_NUMBER_OF_STRINGS];
    hid_t type_id_interval_mta_strings[IAS_L0R_MAX_NUMBER_OF_STRINGS];
    hid_t type_id_scene_mta_strings[IAS_L0R_MAX_NUMBER_OF_STRINGS];
 
    /* ids used for metadata */
    hid_t file_id_metadata;       /* File ID for metadata file */
    int8_t access_mode_metadata;  /* read/write/update (closed -1) */
    hid_t table_id_mta_file;      /* Table ID for file metadata */
    hid_t table_id_mta_interval;  /* Table ID for interval metadata */
    hid_t table_id_mta_scene;     /* Table ID for scene metadata */

    int8_t access_mode_header;        /* read/write/update (closed -1) */
    hid_t file_id_header;             /* File ID for header file */
    hid_t table_id_oli_frame_header;  /* Table ID for frame header OLI data */
    hid_t table_id_oli_image_header;  /* Table ID for image header OLI data */
    hid_t table_id_tirs_frame_header; /* Table ID for frame header TIRS data */
}HDFIO;

/******************************************************************************
*Routines used to initialize items for each category of data store in HDF
*files
******************************************************************************/

int ias_l0r_anc_init(HDFIO* hdfio_ptr);


int ias_l0r_header_init(void);


int ias_l0r_mta_init(HDFIO* hdfio_ptr);

int ias_l0r_hdf_establish_file
(
    const char *path, /* I: Path of the file */
    const char *filename, /* I: Name of the file */
    hid_t *file_id, /* O: The HDF ID of the file established */
    const IAS_ACCESS_MODE file_mode_wanted, /* I: Read Only / Write mode */
    const int create_if_absent /* I: if the file doesn't exist
                                 should it be created */
);

herr_t ias_l0r_hdf_establish_groups
(
    const hid_t file_id, /* I: HDF file ID */
    const char *path /* I: path of new group */
);

hid_t ias_l0r_hdf_establish_table
(
    const char *path, /* I: Path within the HDF file */
    const char *table_name, /* I: Table to create */
    const hid_t file_id, /* I: HDF ID for the file to work with */
    hid_t *table_id, /* O: Table ID for the new table */
    const size_t table_size, /* I: Number of columns for table */
    const size_t struct_size, /* I: Size in bytes of source data structure */
    const char** field_names, /* I: Column names */
    const size_t *offset, /* I: Offsets in bytes of each of the columns */
    const hid_t *field_type /* I: HDF Types for each of the columns */
);

int ias_l0r_hdf_object_exists
(
    const hid_t file_id, /* I: HDF file ID */
    const char *object_to_search_for /* I: Object Name to find */
);


int ias_l0r_hdf_table_records_count
(
    const hid_t file_id, /* I: HDF file ID */
    const char *table, /* I: Table Name */
    int *size          /* O: Number of records in the table */
);

#endif
