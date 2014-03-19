/*-----------------------------------------------------------------------------

NAME: ias_l0r_write_routines.h

PURPOSE: Routines used for writing L0R data

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/

#ifndef IAS_L0R_WRITE_H
#define IAS_L0R_WRITE_H

#include <stdint.h>

int ias_l0r_copy_ancillary
(
    L0RIO *l0r_source, /* I: structure used with the input L0R data */
    L0RIO *l0r_destination /* I: structure used with the output L0R data */
);

int ias_l0r_set_band_lines
(
    L0RIO *file,              /* I: structure for the file used in I/O */
    const int band_number,    /* I: band number to write to */
    const uint32_t line_start,/* I: line to start writing */
    const int line_count,     /* I: the number of lines to write */
    const uint16_t *image     /* I: Data to write, complete lines of data
                                    ordered by [SCA][LINE][DETECTOR] */
);

int ias_l0r_truncate_band_lines
(
    L0RIO *l0r,              /* I: structure used with the L0R data */
    const int band_number    /* I: band number to truncate */
);

int ias_l0r_set_top_detector_offsets
(
    L0RIO *file,            /* I: structure for the file used in I/O */
    const int band_number,  /* I: band number to write to */
    const uint16_t *offsets /* I: Data to write */
);

int ias_l0r_set_bottom_detector_offsets
(
    L0RIO *file,            /* I: structure for the file used in I/O */
    const int band_number,  /* I: band number to write to */
    const uint16_t *offsets /* I: Data to write */
);

int ias_l0r_set_band_lines_sca
(
    L0RIO *file,                /* I: structure for the file used in I/O */
    const int band_number,      /* I: band number to read */
    const int sca_index,        /* I: SCA to write to */
    const uint32_t line_start,  /* I: line to start writing at */
    const uint32_t pixel_start, /* I: pixel to start writing at */
    const uint32_t line_count,  /* I: number of lines to write */
    const uint32_t pixel_count, /* I: number of pixels to write */
    const uint16_t *image       /* I: Data to write */
);

int ias_l0r_set_file_metadata
(
     L0RIO *l0r, /* I: structure used with the l0r data */
     const IAS_L0R_FILE_METADATA *mta_file  /* I: Data to write */
);

int ias_l0r_set_interval_metadata
(
    L0RIO *l0r, /* I: structure used with the l0r data */
    const IAS_L0R_INTERVAL_METADATA *mta_info_interval   /* I: Data to write */
);

int ias_l0r_set_scene_metadata
(
    L0RIO *l0r,      /* I: structure used with the l0r data   */
    const int index, /* I: first scene to write */
    const int count, /* I: number of scenes to write */
    const IAS_L0R_SCENE_METADATA *mta_info_scene  /* I: Data to write */
);

int ias_l0r_delete_scene_metadata
(
    L0RIO *l0r /* I: structure used with the l0r data   */
);

int ias_l0r_set_oli_image_header
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const IAS_L0R_OLI_IMAGE_HEADER *image_header /* I: image header data
                                                       to append */
);

int ias_l0r_append_oli_frame_headers
(
    L0RIO *l0r, /* I: append the line to the table in this file */
    const int num_lines_to_be_written, /* I: number of lines to write */
    const IAS_L0R_OLI_FRAME_HEADER *frame_header /* I: the frame
                                                       header data to append */
);

int ias_l0r_append_tirs_frame_headers
(
    L0RIO *l0r, /* I: append the frame to the table in this file */
    const int num_lines_to_be_written, /* I: number of lines to write */
    const IAS_L0R_TIRS_FRAME_HEADER *frame_header   /* I: the frame header
                                                          data to append */
);

int ias_l0r_set_oli_frame_headers
(
    L0RIO *l0r,                     /* I: structure used with the L0R data */
    const int start_record,         /* I: first record to write */
    const int num_records_to_write, /* I: number to write */
    const IAS_L0R_OLI_FRAME_HEADER *frame_header /* I: data to write */
);

int ias_l0r_truncate_oli_frame_headers
(
    L0RIO *l0r,                     /* I: structure used with the L0R data */
    const int num_records           /* I: desired table size */
);

int ias_l0r_set_tirs_frame_headers
(
    L0RIO *l0r,      /* I: structure used with the L0R data */
    const int index, /* I: first record to write */
    const int count, /* I: number to write */
    const IAS_L0R_TIRS_FRAME_HEADER *buffer /* I: data to write */
);

int ias_l0r_truncate_tirs_frame_headers
(
    L0RIO *l0r,                     /* I: structure used with the L0R data */
    const int num_records           /* I: desired table size */
);

/***************************************
*append routines
****************************************/

int ias_l0r_append_ancillary_attitude
(
    L0RIO *l0r,      /* I: structure used with the L0R data */
    const int count, /* I: number of items to append */
    const IAS_L0R_ATTITUDE *attitude /* I: data to append */
);

int ias_l0r_append_ancillary_ephemeris
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count, /* I: number of items to append */
    const IAS_L0R_EPHEMERIS *ephemeris /* I: data to append */
);

int ias_l0r_append_ancillary_gps_position
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count, /* I: number of items to append */
    const IAS_L0R_GPS_POSITION *gps_position /* I: data to append */
);

int ias_l0r_append_ancillary_imu
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count, /* I: number of items to append */
    const IAS_L0R_IMU *imu /* I: data to append */
);

int ias_l0r_append_ancillary_imu_latency
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count, /* I: number of items to append */
    const IAS_L0R_IMU_LATENCY *imu_latency /* I: data to append */
);

int ias_l0r_append_ancillary_oli_telemetry
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count, /* I: number of items to append */
    const IAS_L0R_OLI_TELEMETRY *telemetry /* I: data to append */
);

int ias_l0r_append_ancillary_tirs_telemetry
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count, /* I: number of items to append */
    const IAS_L0R_TIRS_TELEMETRY *telemetry /* I: data to append */
);

int ias_l0r_append_ancillary_star_tracker_centroid
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count, /* I: number of items to append */
    const IAS_L0R_STAR_TRACKER_CENTROID *centroid /* I: data to append */
);

int ias_l0r_append_ancillary_star_tracker_quaternion
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count, /* I: number of items to append */
    const IAS_L0R_STAR_TRACKER_QUATERNION *centroid /* I: data to append */
);

int ias_l0r_append_ancillary_temperatures_gyro
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count, /* I: number of items to append */
    const IAS_L0R_GYRO_TEMPERATURE *temperatures_gyro /* I: data to append */
);

int ias_l0r_append_ancillary_temperatures_oli_tirs
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count,  /* I: number of items to append */
    const IAS_L0R_OLI_TIRS_TEMPERATURE *temperatures_oli /* I: data to append */
);

/* ************************************************************************* */
/* ancillary write routines */
/* ************************************************************************** */

int ias_l0r_set_ancillary_attitude
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: index to begin write at */
    const int count, /* I: number of items to write */
    const IAS_L0R_ATTITUDE *attitude /* I: data to be written */
);

int ias_l0r_set_ancillary_attitude_filter_states
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: index to begin write at */
    const int count, /* number of items to write  */
    const IAS_L0R_ATTITUDE_FILTER_STATES *attitude /* I: data to be written */
);

int ias_l0r_set_ancillary_imu
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: index to begin write at */
    const int count, /* I: number of items to write */
    const IAS_L0R_IMU *imu /* I: data to be written */
);

int ias_l0r_set_ancillary_imu_latency
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: index to begin write at */
    const int count, /* I: number of items to write */
    const IAS_L0R_IMU_LATENCY *imu_latency /* I: data to be written */
);

int ias_l0r_set_ancillary_oli_telemetry
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: position to start writing records at  */
    const int count, /* I: number of records to be written */
    const IAS_L0R_OLI_TELEMETRY *telemetry /* I: data to be written */
);

int ias_l0r_set_ancillary_tirs_telemetry
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: position to start writing records at  */
    const int count, /* I: number of records to be written */
    const IAS_L0R_TIRS_TELEMETRY *telemetry /* I: data to be written */
);

int ias_l0r_set_ancillary_gps_position
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: index to begin write at */
    const int count, /* number of items to be written */
    const IAS_L0R_GPS_POSITION *gps_position /* I: data to be written */
);

int ias_l0r_set_ancillary_gps_range
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: index to begin the write at */
    const int count, /* number of items to be written */
    const IAS_L0R_GPS_RANGE *gps_position /* I: data to write */
);

int ias_l0r_set_ancillary_ephemeris
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: index to begin the write at */
    const int count, /* number of items to be written */
    const IAS_L0R_EPHEMERIS *ephemeris /* I: data to write */
);

int ias_l0r_set_ancillary_star_tracker_centroid
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: index to begin the write at */
    const int count, /* I: number of items to write */
    const IAS_L0R_STAR_TRACKER_CENTROID *centroid /* I: data to write */
);

int ias_l0r_set_ancillary_star_tracker_quaternion
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: index to begin the write at */
    const int count, /* I: number of items to write */
    const IAS_L0R_STAR_TRACKER_QUATERNION *quaternion /* I: data to write */
);

int ias_l0r_set_ancillary_temperatures_gyro
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: index to begin the write at */
    const int count, /* I: number of items to write */
    const IAS_L0R_GYRO_TEMPERATURE *temperatures_gyro /* I: data to write */
);

int ias_l0r_set_ancillary_temperatures_oli_tirs
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: index to begin the write at */
    const int count, /* I: number of items to write */
    const IAS_L0R_OLI_TIRS_TEMPERATURE *temperatures_oli_tirs
    /* I: data to write */
);

#endif
