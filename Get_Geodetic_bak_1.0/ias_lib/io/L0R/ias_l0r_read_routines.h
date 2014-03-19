/*-----------------------------------------------------------------------------

NAME: ias_l0r_read_routines.h

PURPOSE: Routines used for reading L0R data.

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/

#ifndef IAS_L0R_READ_H
#define IAS_L0R_READ_H

#include <stdint.h>
#include <time.h>

#include "ias_types.h"

/******************************************************************************
  General routines
******************************************************************************/
int ias_l0r_close
(
    L0RIO *l0r /* I: data structure for the l0r data being closed */
);

int ias_l0r_format_time
(
    const struct tm *timeptr, /* I: The time structure to be formatted */
    char *formatted_time,       /* O: The formatted time */
    int size     /* I: Size in bytes of the buffer allocated */
);

int ias_l0r_get_version_number
(
    unsigned int *version_number /* O: The L0Ra Format Version number */
);

L0RIO* ias_l0r_open
(
    const char *interval_id, /* I: Landsat interval ID or time used
                                   in l0r naming */
    const char *path,        /* I: path to the l0r location */
    const IAS_L0R_BAND_COMPRESSION compression
    /* I: flag indicating image datasets should  be compressed */
);

L0RIO* ias_l0r_open_from_filename
(
    const char *path_id, /* I: path where the L0R data is located and
                          Landsat interval id or time used with the L0R file
                          names combined and separated by "/" */
    const IAS_L0R_BAND_COMPRESSION compression /* I: flag indicating image
                                             datasets should be compressed */
);

int ias_l0r_present
(
    const char *path_id  /* I: path where to check if L0R data exist and
                          Landsat interval id or time used with the L0R file
                          names combined and separated by "/" */
);

/* ****************************************************************************
  Band file routines
 *****************************************************************************/

int ias_l0r_open_band
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: band number */
    const IAS_ACCESS_MODE file_access_mode /* I: file access mode */
);

int ias_l0r_close_band
(
    L0RIO *file,           /* I: structure for the file used in I/O */
    const int band_number  /* I: band number */
);

int ias_l0r_get_band_records_count
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: band number */
    int *size              /* O: number of records for the image
                                 dataset for this band */
);

int ias_l0r_get_band_offset_records_count
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: band number */
    int *size              /* O: number of records for the offset
                                 dataset for this band */
);

int ias_l0r_get_band_attributes
(
    L0RIO *l0r,            /* I: structure for the file used in I/O */
    const int band_number, /* I: band number */
    IAS_L0R_BAND_ATTRIBUTES *image_attributes /* O: attribute structure */
);

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
);

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
);

int ias_l0r_get_top_detector_offsets
(
    L0RIO *file,           /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    uint16_t *offsets,     /* O: offset data  */
    const int offsets_size /* I: number of offsets which storage has been
                             allocated for */
);

int ias_l0r_get_bottom_detector_offsets
(
    L0RIO *file,           /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    uint16_t *offsets,     /* O: offset data  */
    const int offsets_size /* I: number of offsets which storage has been
                             allocated for */
);

int ias_l0r_get_top_detector_offsets_sca
(
    L0RIO *file,           /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    const int sca_index,   /* I: SCA number to read */
    uint16_t *offsets,     /* O: offset data  */
    const int offsets_size /* I: number of offsets which storage has been
                             allocated for */
);

int ias_l0r_get_bottom_detector_offsets_sca
(
    L0RIO *file,           /* I: structure for the file used in I/O */
    const int band_number, /* I: band number to read */
    const int sca_index,   /* I: SCA number to read */
    uint16_t *offsets,     /* O: offset data  */
    const int offsets_size /* I: number of offsets which storage has been
                             allocated for */
);

int ias_l0r_get_band_present
(
    L0RIO *file,            /* I: structure for the file used in I/O */
    const int band_number,  /* I: band number */
    int *band_present       /* O: Contains 1 if the band is present,
                                  contains 0 if the band is not present */
);
/*******************************************************************************
*Metadata routines
*******************************************************************************/
int ias_l0r_open_metadata
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const IAS_ACCESS_MODE file_access_mode /* I: Mode to open file in */
);

int ias_l0r_close_metadata
(
    L0RIO *l0r /* I: structure for the file used in I/O */
);

int ias_l0r_get_file_metadata
(
     L0RIO *l0r, /* I: structure used with the l0r data to be read from */
     IAS_L0R_FILE_METADATA *mta_info_file /* O: File Metadata */
);

int ias_l0r_get_file_metadata_records_count
(
    L0RIO *l0r,        /* I: structure used with the l0r data */
    int *size          /* O: number of records in the file dataset */
);

int ias_l0r_get_filenames
(
     L0RIO *l0r,      /* I: structure used with the l0r data */
     char** filenames /* O: filenames populated by the read  */
);

int ias_l0r_get_filename_count
(
     L0RIO *l0r,         /* I: structure used with the l0r data */
     int *count /* O: Number of files */
);

int ias_l0r_get_metadata_filename
(
     L0RIO *l0r, /* I: structure used with the l0r data */
     char filename[IAS_L0R_FILE_NAME_LENGTH+1]  /* O: filename populated
                                                    by the read */
);

int ias_l0r_get_interval_metadata
(
    L0RIO *l0r, /* I: structure used with the l0r data */
    IAS_L0R_INTERVAL_METADATA *mta_info_interval /* O: Metadata read */
);

int ias_l0r_get_interval_metadata_records_count
(
    L0RIO *l0r,        /* I: structure used with the l0r data */
    int *size          /* O: number of records in the interval dataset */
);

int ias_l0r_get_scene_metadata
(
    L0RIO *l0r,      /* I: structure used with the l0r data */
    const int index, /* I: scene to begin the read at */
    const int count, /* I: the number of scenes to read */
    IAS_L0R_SCENE_METADATA *mta_info_scene /* O: Metadata read */
);

int ias_l0r_get_scene_metadata_by_id
(
    L0RIO *l0r,           /* I: structure used with the l0r data */
    const char *scene_id, /* I: string for the Landsat scene id */
    IAS_L0R_SCENE_METADATA *mta_info_scene /* O: Metadata read */
);

int ias_l0r_get_scene_metadata_records_count
(
    L0RIO *l0r,        /* I: structure used with the l0r data */
    int *size          /* O: number of records in the scene dataset */
);
/* ************************************************************************** */
/* OLI data routines */
/* ************************************************************************** */

int ias_l0r_open_header
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const IAS_ACCESS_MODE file_access_mode /* I: Mode to open file in */
);

int ias_l0r_close_header
(
    L0RIO *l0r /* I:structure used with the L0R data */
);

/*******************************************************************************
* Image header routines
*******************************************************************************/

int ias_l0r_get_oli_image_header
(
    L0RIO *l0r, /* I: structure used with the L0R data  */
    IAS_L0R_OLI_IMAGE_HEADER *image_header /* O: Data read */
);

int ias_l0r_is_oli_image_header_present
(
    L0RIO *l0r,        /* I: structure used with the L0R data */
    int *size          /* O: number of records in the image header dataset*/
);

/*******************************************************************************
* Frame header routines
*******************************************************************************/

int ias_l0r_get_oli_frame_headers
(
    L0RIO *l0r,                    /* I: structure used with the L0R data */
    const int start_record,        /* I: First record to read */
    const int num_records_to_read, /* I: Number to read */
    IAS_L0R_OLI_FRAME_HEADER *frame_header /* O: Data read */
);

int ias_l0r_get_oli_frame_headers_records_count
(
    L0RIO *l0r,        /* I: structure used with the L0R data */
    int *size          /* O: number of records in the OLI frame header dataset*/
);

int ias_l0r_get_tirs_frame_headers
(
    L0RIO *l0r,                    /* I: structure used with the L0R data */
    const int start_record,        /* I: First record to read */
    const int num_records_to_read, /* I: Number to read */
    IAS_L0R_TIRS_FRAME_HEADER *frame_header /* O: Data read */
);

int ias_l0r_get_tirs_frame_headers_records_count
(
    L0RIO *l0r,        /* I: structure used with the L0R data */
    int *size          /* O: number of records in  the TIRS frame header
                             dataset*/
);

/*******************************************************************************
*Ancillary
*******************************************************************************/
int ias_l0r_open_ancillary
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const IAS_ACCESS_MODE file_access_mode /* I: Mode to open file in */
);

int ias_l0r_close_ancillary
(
    L0RIO *l0r /* I: structure used with the L0R data */
);

int ias_l0r_get_ancillary_imu
(
    L0RIO *l0r,      /* I: structure used with the L0R data  */
    const int index, /* I: index to begin read at */
    const int count, /* I: number of items to read */
    IAS_L0R_IMU *imu /* O: buffer for the read data */
);

int ias_l0r_get_ancillary_imu_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_imu_latency
(
    L0RIO *l0r,      /* I: structure used with the L0R data  */
    const int index, /* I: index to begin read at */
    const int count, /* I: number of items to read */
    IAS_L0R_IMU_LATENCY *imu_latency /* O: buffer for the read data */
);

int ias_l0r_get_ancillary_imu_latency_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_gps_position
(
    L0RIO *l0r, /* I: structure used with the L0R data   */
    const int index, /* I: index to begin read at */
    const int count, /* I: the number of items to read */
    IAS_L0R_GPS_POSITION *gps_position /* O: buffer for the read data */
);

int ias_l0r_get_ancillary_gps_range
(
    L0RIO *l0r, /* I: structure used with the L0R data   */
    const int index, /* I: index to begin read at */
    const int count, /* I: the number of items to read */
    IAS_L0R_GPS_RANGE *gps_range /* O: buffer for the read data */
);

int ias_l0r_get_ancillary_gps_position_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_gps_range_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_attitude
(
    L0RIO *l0r, /* I: structure used with the L0R data   */
    const int index, /* I: index to being the read at */
    const int count, /* I: number of items to read */
    IAS_L0R_ATTITUDE *attitude /* O: buffer for read data */
);

int ias_l0r_get_ancillary_attitude_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_attitude_filter_states
(
    L0RIO *l0r, /* I: structure used with the L0R data   */
    const int index, /* I: index to being the read at */
    const int count, /* I: number of items to read */
    IAS_L0R_ATTITUDE_FILTER_STATES *attitude /* O: buffer for read data */
);

int ias_l0r_get_ancillary_attitude_filter_states_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_ephemeris
(
    L0RIO *l0r, /* I: structure used with the L0R data  */
    const int index, /* I: index to begin read at  */
    const int count, /* I: number of items to read */
    IAS_L0R_EPHEMERIS *ephemeris /* O: buffer for read data */
);

int ias_l0r_get_ancillary_ephemeris_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_oli_telemetry
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: the index to begin the read at */
    const int count, /* I: number of items to read */
    IAS_L0R_OLI_TELEMETRY *telemetry /* O: buffer to hold data read */
);

int ias_l0r_get_ancillary_oli_telemetry_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_tirs_telemetry
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: the index to begin the read at */
    const int count, /* I: number of items to read */
    IAS_L0R_TIRS_TELEMETRY *telemetry /* O: buffer to hold data read */
);

int ias_l0r_get_ancillary_tirs_telemetry_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_tirs_ssm
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: the index to begin the read at */
    const int count, /* I: number of items to read */
    IAS_L0R_TIRS_SSM_ENCODER *ssm_records /* O: buffer to hold data read */
);

int ias_l0r_get_ancillary_tirs_ssm_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size   /* O: number of records in this dataset */
);


int ias_l0r_get_ancillary_star_tracker_centroid
(
    L0RIO *l0r,/* I: structure used with the L0R data */
    const int index, /* I: index to begin read at */
    const int count, /* I: number of items to read */
    IAS_L0R_STAR_TRACKER_CENTROID *centroid /* O: buffer for read data */
);

int ias_l0r_get_ancillary_star_tracker_centroid_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_star_tracker_quaternion
(
    L0RIO *l0r,/* I: structure used with the L0R data */
    const int index, /* I: index to begin read at */
    const int count, /* I: number of items to read */
    IAS_L0R_STAR_TRACKER_QUATERNION *quaternion /* O: buffer for read data */
);

int ias_l0r_get_ancillary_star_tracker_quaternion_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_gyro_temperature
(
    L0RIO *l0r, /* I: pointer to structure used in I/O */
    const int index, /* I: index to begin read at */
    const int count,/* I: number of items to read */
    IAS_L0R_GYRO_TEMPERATURE *buffer/* O: buffer for read data */
);

int ias_l0r_get_ancillary_gyro_temperature_records_count
(
    L0RIO *l0r, /* I: pointer to structure used in I/O */
    int *size         /* O: number of records in this dataset */
);

int ias_l0r_get_ancillary_oli_tirs_temperature
(
    L0RIO *l0r, /* I: pointer to structure used in I/O */
    const int index,/* I: index to begin read at */
    const int count,/* I: number of items to read */
    IAS_L0R_OLI_TIRS_TEMPERATURE *buffer/* O: buffer for read data */
);

int ias_l0r_get_ancillary_oli_tirs_temperature_records_count
(
    L0RIO *l0r, /* I: pointer to structure used in I/O */
    int *size         /* O: number of records in this dataset */
);

#endif
