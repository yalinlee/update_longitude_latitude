/*-----------------------------------------------------------------------------

NAME: ias_l0r_anc.c

PURPOSE: Functions related to reading and writing ancillary data contained
         in the L0R

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* HDF library includes */
#include <hdf5.h>
#include <hdf5_hl.h>

/* project includes */
#include "ias_logging.h"
#include "ias_l0r.h"
#include "ias_l0r_hdf.h"
#include "ias_types.h"

/*******************************************************************************
*constant values for the sizes/offsets/names/etc used with tables
*******************************************************************************/
#define GROUP_NAME_ACS "ACS"
#define GROUP_NAME_ATTITUDE "Attitude"
#define GROUP_NAME_IMU "IMU"
#define GROUP_NAME_OLI "OLI"
#define GROUP_NAME_TIRS "TIRS"
#define GROUP_NAME_TELEMETRY "Telemetry"
#define GROUP_NAME_SPACECRAFT "Spacecraft"
#define GROUP_NAME_TEMPERATURES "Temperatures"

#define TABLE_NAME_ATTITUDE "Attitude"
#define TABLE_NAME_ATTITUDE_FILTER "Attitude_Filter"
#define TABLE_NAME_EPHEMERIS "Ephemeris"
#define TABLE_NAME_GPS_POSITION "GPS_Position"
#define TABLE_NAME_GPS_RANGE "GPS_Range"
#define TABLE_NAME_IMU "Gyro"
#define TABLE_NAME_IMU_LATENCY "Latency"
#define TABLE_NAME_OLI_TELEMETRY_GROUP_3 "Telemetry_Group_3"
#define TABLE_NAME_OLI_TELEMETRY_GROUP_4 "Telemetry_Group_4"
#define TABLE_NAME_OLI_TELEMETRY_GROUP_5 "Telemetry_Group_5"
#define TABLE_NAME_TIRS_TELEMETRY "TIRS_Telemetry"
#define TABLE_NAME_STAR_TRACKER_CENTROID "Star_Tracker_Centroid"
#define TABLE_NAME_STAR_TRACKER_QUATERNION "Star_Tracker_Quaternion"
#define TABLE_NAME_TEMPERATURES_GYRO "Gyro"
#define TABLE_NAME_TEMPERATURES_OLI_TIRS "OLI_TIRS"

#define TABLE_SIZE_ATTITUDE 8
#define TABLE_SIZE_ATTITUDE_FILTER 23
#define TABLE_SIZE_EPHEMERIS 16
#define TABLE_SIZE_IMU 55
#define TABLE_SIZE_IMU_LATENCY 5
#define TABLE_SIZE_GPS_POSITION 77
#define TABLE_SIZE_GPS_RANGE 103 
#define TABLE_SIZE_TELEMETRY_GROUP_3 41
#define TABLE_SIZE_TELEMETRY_GROUP_4 39
#define TABLE_SIZE_TELEMETRY_GROUP_5 46
#define TABLE_SIZE_TIRS_TELEMETRY 122 
#define TABLE_SIZE_STAR_TRACKER_CENTROID 39 
#define TABLE_SIZE_STAR_TRACKER_QUATERNION 47
#define TABLE_SIZE_TEMPERATURES_GYRO 36
#define TABLE_SIZE_TEMPERATURES_OLI_TIRS 70

/*******************************************************************************
*private routine prototypes
*******************************************************************************/
static hid_t ias_l0r_establish_ancillary_file(HDFIO *hdfio_ptr,
    int create_if_absent);

static int ias_l0r_anc_read(L0RIO *l0r, const int index,
    const int count, const char *table_name, const size_t size,
    const size_t *offset, const size_t *size_list, void *buffer);

static int ias_l0r_anc_write(L0RIO *l0r, const int index,
    const int count, const void *buffer, const char *table_name,
    const size_t size, const size_t *offset, const size_t *size_list);

static int ias_l0r_append_ancillary_records(L0RIO *l0r, const int count,
    const void *buffer, const char *table_name,
    const size_t size, const size_t *offset, const size_t *size_list);

static int ias_l0r_get_anc_table_records_count(HDFIO *hdfio_ptr,
    const char *table_name, int *size);


/***************************************
*ATTITUDE constants
******************************************************************************/
static const hid_t  *field_type_attitude = NULL;
static hid_t attitude_types[TABLE_SIZE_ATTITUDE];

static const char *FIELD_NAMES_ATTITUDE[TABLE_SIZE_ATTITUDE] =
{
    "l0r_time_days_from_J2000",
    "l0r_time_seconds_of_day",
    "seconds_original",
    "inertial_to_body_x",
    "inertial_to_body_y",
    "inertial_to_body_z",
    "inertial_to_body_scalar",
    "warning_flag"
};
static const IAS_L0R_ATTITUDE *IAS_L0R_ATTITUDE_PTR = NULL;
static const size_t IAS_L0R_ATTITUDE_SIZE =  sizeof(IAS_L0R_ATTITUDE);
static const size_t IAS_L0R_ATTITUDE_SIZES[TABLE_SIZE_ATTITUDE] =
{
    sizeof( (*IAS_L0R_ATTITUDE_PTR).l0r_time.days_from_J2000),
    sizeof( (*IAS_L0R_ATTITUDE_PTR).l0r_time.seconds_of_day),
    sizeof( (*IAS_L0R_ATTITUDE_PTR).time_tag_sec_orig),
    sizeof( (*IAS_L0R_ATTITUDE_PTR).inertial_to_body.vector.x),
    sizeof( (*IAS_L0R_ATTITUDE_PTR).inertial_to_body.vector.y),
    sizeof( (*IAS_L0R_ATTITUDE_PTR).inertial_to_body.vector.z),
    sizeof( (*IAS_L0R_ATTITUDE_PTR).inertial_to_body.scalar),
    sizeof( (*IAS_L0R_ATTITUDE_PTR).warning_flag)
};
 
static const size_t OFFSET_ATTITUDE[TABLE_SIZE_ATTITUDE] =
{
    HOFFSET( IAS_L0R_ATTITUDE,l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_ATTITUDE,l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_ATTITUDE,time_tag_sec_orig),
    HOFFSET( IAS_L0R_ATTITUDE,inertial_to_body.vector.x),
    HOFFSET( IAS_L0R_ATTITUDE,inertial_to_body.vector.y),
    HOFFSET( IAS_L0R_ATTITUDE,inertial_to_body.vector.z),
    HOFFSET( IAS_L0R_ATTITUDE,inertial_to_body.scalar),
    HOFFSET( IAS_L0R_ATTITUDE,warning_flag)
};

/******************************************************************************
*ATTITUDE_FILTER constants
******************************************************************************/
static const hid_t  *field_type_attitude_filter = NULL;
static hid_t attitude_filter_types[TABLE_SIZE_ATTITUDE_FILTER];

static const char *FIELD_NAMES_ATTITUDE_FILTER[TABLE_SIZE_ATTITUDE_FILTER] =
{
    "l0r_time_days_from_J2000", /* days from J2000 for attitude estimate */
    "l0r_time_seconds_of_day", /* seconds of the day */
    "seconds_original",
    "subseconds_original",
    "gyro_combined_bias_rad_sec_x",
    "gyro_combined_bias_rad_sec_y",
    "gyro_combined_bias_rad_sec_z",
    "gyro_scale_factor_x",
    "gyro_scale_factor_y",
    "gyro_scale_factor_z",
    "gyro_x_misalignment_along_y_rad", /* Gyro X axis misalignment along Y */
    "gyro_x_misalignment_along_z_rad", /* Gyro X axis misalignment along Z */
    "gyro_y_misalignment_along_x_rad", /* Gyro Y axis misalignment along X */
    "gyro_y_misalignment_along_z_rad", /* Gyro Y axis misalignment along Z */
    "gyro_z_misalignment_along_x_rad", /* Gyro Z axis misalignment along X */
    "gyro_z_misalignment_along_y_rad", /* Gyro Z axis misalignment along Y */
    "kalman_filter_error_rad_x", /* kalman filter attitude error */
    "kalman_filter_error_rad_y", /* kalman filter attitude error */
    "kalman_filter_error_rad_z", /* kalman filter attitude error */
    "covariance_diagonal_x", /* Quaternion covariance x component */
    "covariance_diagonal_y", /* Quaternion covariance y component */
    "covariance_diagonal_z", /* Quaternion covariance z component */
    "warning_flag"
};

static const IAS_L0R_ATTITUDE_FILTER_STATES* IAS_L0R_ATTITUDE_FILTER_PTR;
static const size_t IAS_L0R_ATTITUDE_FILTER_SIZE =
    sizeof(IAS_L0R_ATTITUDE_FILTER_STATES);
static const size_t IAS_L0R_ATTITUDE_FILTER_SIZES[TABLE_SIZE_ATTITUDE_FILTER] =
{
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).l0r_time.days_from_J2000),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).l0r_time.seconds_of_day),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).time_tag_sec_orig),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).time_tag_subseconds_orig),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_combined_bias_rad_sec.x),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_combined_bias_rad_sec.y),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_combined_bias_rad_sec.z),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_scale_factor.x),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_scale_factor.y),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_scale_factor.z),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_x_misalignment_along_y_rad),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_x_misalignment_along_z_rad),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_y_misalignment_along_x_rad),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_y_misalignment_along_z_rad),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_z_misalignment_along_x_rad),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).gyro_z_misalignment_along_y_rad),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).kalman_filter_error_rad.x),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).kalman_filter_error_rad.y),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).kalman_filter_error_rad.z),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).covariance_diagonal.x),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).covariance_diagonal.y),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).covariance_diagonal.z),
    sizeof( (*IAS_L0R_ATTITUDE_FILTER_PTR).warning_flag)
};

static const size_t OFFSET_ATTITUDE_FILTER[TABLE_SIZE_ATTITUDE_FILTER] =
{
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,time_tag_sec_orig),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,time_tag_subseconds_orig),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_combined_bias_rad_sec.x),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_combined_bias_rad_sec.y),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_combined_bias_rad_sec.z),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_scale_factor.x),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_scale_factor.y),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_scale_factor.z),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_x_misalignment_along_y_rad),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_x_misalignment_along_z_rad),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_y_misalignment_along_x_rad),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_y_misalignment_along_z_rad),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_z_misalignment_along_x_rad),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,gyro_z_misalignment_along_y_rad),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,kalman_filter_error_rad.x),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,kalman_filter_error_rad.y),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,kalman_filter_error_rad.z),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,covariance_diagonal.x),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,covariance_diagonal.y),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,covariance_diagonal.z),
    HOFFSET( IAS_L0R_ATTITUDE_FILTER_STATES,warning_flag)
};


/******************************************************************************
*Ephemeris table data constants
******************************************************************************/
static const hid_t  *field_type_ephemeris = NULL;
static hid_t ephemeris_types[TABLE_SIZE_EPHEMERIS];

static const char *FIELD_NAMES_EPHEMERIS[TABLE_SIZE_EPHEMERIS] =
{
    "l0r_time_days_from_J2000", /* days from J2000 for attitude estimate */
    "l0r_time_seconds_of_day", /* seconds of the day */
    "seconds_original",
    "ecef_x_position_meters",
    "ecef_y_position_meters",
    "ecef_z_position_meters",
    "ecef_x_velocity_meters_per_sec",
    "ecef_y_velocity_meters_per_sec",
    "ecef_z_velocity_meters_per_sec",
    "orbit_determination_x_position_error_meters",
    "orbit_determination_y_position_error_meters",
    "orbit_determination_z_position_error_meters",
    "orbit_determination_x_velocity_error_meters_per_sec",
    "orbit_determination_y_velocity_error_meters_per_sec",
    "orbit_determination_z_velocity_error_meters_per_sec",
    "warning_flag"
};
static const IAS_L0R_EPHEMERIS *IAS_L0R_EPHEMERIS_PTR = NULL;
static const size_t IAS_L0R_EPHEMERIS_SIZE =  sizeof(IAS_L0R_EPHEMERIS);
static const size_t IAS_L0R_EPHEMERIS_SIZES[TABLE_SIZE_EPHEMERIS] =
{
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).l0r_time.days_from_J2000),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).l0r_time.seconds_of_day),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).time_tag_sec_orig),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).ecef_position_meters.x),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).ecef_position_meters.y),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).ecef_position_meters.z),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).ecef_velocity_meters_per_sec.x),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).ecef_velocity_meters_per_sec.y),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).ecef_velocity_meters_per_sec.z),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).
        orbit_determination_position_error_meters.x),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).
        orbit_determination_position_error_meters.y),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).
        orbit_determination_position_error_meters.z),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).
        orbit_determination_velocity_error_meters_per_sec.x),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).
        orbit_determination_velocity_error_meters_per_sec.y),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).
        orbit_determination_velocity_error_meters_per_sec.z),
    sizeof( (*IAS_L0R_EPHEMERIS_PTR).warning_flag)
};

static const size_t OFFSET_EPHEMERIS[TABLE_SIZE_EPHEMERIS] =
{
    HOFFSET( IAS_L0R_EPHEMERIS,l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_EPHEMERIS,l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_EPHEMERIS,time_tag_sec_orig),
    HOFFSET( IAS_L0R_EPHEMERIS,ecef_position_meters.x),
    HOFFSET( IAS_L0R_EPHEMERIS,ecef_position_meters.y),
    HOFFSET( IAS_L0R_EPHEMERIS,ecef_position_meters.z),
    HOFFSET( IAS_L0R_EPHEMERIS,ecef_velocity_meters_per_sec.x),
    HOFFSET( IAS_L0R_EPHEMERIS,ecef_velocity_meters_per_sec.y),
    HOFFSET( IAS_L0R_EPHEMERIS,ecef_velocity_meters_per_sec.z),
    HOFFSET( IAS_L0R_EPHEMERIS,orbit_determination_position_error_meters.x),
    HOFFSET( IAS_L0R_EPHEMERIS,orbit_determination_position_error_meters.y),
    HOFFSET( IAS_L0R_EPHEMERIS,orbit_determination_position_error_meters.z),
    HOFFSET( IAS_L0R_EPHEMERIS,
        orbit_determination_velocity_error_meters_per_sec.x),
    HOFFSET( IAS_L0R_EPHEMERIS,
        orbit_determination_velocity_error_meters_per_sec.y),
    HOFFSET( IAS_L0R_EPHEMERIS,
        orbit_determination_velocity_error_meters_per_sec.z),
    HOFFSET( IAS_L0R_EPHEMERIS,warning_flag)
};

/******************************************************************************
*GPS_POSITION table data constants
******************************************************************************/
static const hid_t *field_type_gps_position = NULL;
static hid_t gps_position_types[TABLE_SIZE_GPS_POSITION];

static const char *FIELD_NAMES_GPS_POSITION[TABLE_SIZE_GPS_POSITION] =
{
    "l0r_time_days_from_J2000", /* days from J2000 for attitude estimate */
    "l0r_time_seconds_of_day", /* seconds of the day */
    "month",
    "day",
    "year",
    "hours",
    "minutes",
    "seconds",
    "nanoseconds",
    "function",
    "sub_function",
    "latitude",
    "longitude",
    "height_uncorrected",
    "height_corrected",
    "velocity",
    "heading",
    "current_dop",
    "dop_type",
    "num_visible_satellites",
    "num_satellites_tracked",
    "tracked_sat_1_sat_id",
    "tracked_sat_1_track_mode",
    "tracked_sat_1_signal_strength",
    "tracked_sat_1_channel_status_flags",
    "tracked_sat_2_sat_id",
    "tracked_sat_2_track_mode",
    "tracked_sat_2_signal_strength",
    "tracked_sat_2_channel_status_flags",
    "tracked_sat_3_sat_id",
    "tracked_sat_3_track_mode",
    "tracked_sat_3_signal_strength",
    "tracked_sat_3_channel_status_flags",
    "tracked_sat_4_sat_id",
    "tracked_sat_4_track_mode",
    "tracked_sat_4_signal_strength",
    "tracked_sat_4_channel_status_flags",
    "tracked_sat_5_sat_id",
    "tracked_sat_5_track_mode",
    "tracked_sat_5_signal_strength",
    "tracked_sat_5_channel_status_flags",
    "tracked_sat_6_sat_id",
    "tracked_sat_6_track_mode",
    "tracked_sat_6_signal_strength",
    "tracked_sat_6_channel_status_flags",
    "tracked_sat_7_sat_id",
    "tracked_sat_7_track_mode",
    "tracked_sat_7_signal_strength",
    "tracked_sat_7_channel_status_flags",
    "tracked_sat_8_sat_id",
    "tracked_sat_8_track_mode",
    "tracked_sat_8_signal_strength",
    "tracked_sat_8_channel_status_flags",
    "tracked_sat_9_sat_id",
    "tracked_sat_9_track_mode",
    "tracked_sat_9_signal_strength",
    "tracked_sat_9_channel_status_flags",
    "tracked_sat_10_sat_id",
    "tracked_sat_10_track_mode",
    "tracked_sat_10_signal_strength",
    "tracked_sat_10_channel_status_flags",
    "tracked_sat_11_sat_id",
    "tracked_sat_11_track_mode",
    "tracked_sat_11_signal_strength",
    "tracked_sat_11_channel_status_flags",
    "tracked_sat_12_sat_id",
    "tracked_sat_12_track_mode",
    "tracked_sat_12_signal_strength",
    "tracked_sat_12_channel_status_flags",
    "receiver_status_flags",
    "ecef_x_pos",
    "ecef_y_pos",
    "ecef_z_pos",
    "ecef_x_vel",
    "ecef_y_vel",
    "ecef_z_vel",
    "warning_flag"
};

static const IAS_L0R_GPS_POSITION  *oli_l0r_gps_ptr = NULL;
static const size_t IAS_L0R_GPS_SIZE =  sizeof(IAS_L0R_GPS_POSITION);
static const size_t IAS_L0R_GPS_SIZES[TABLE_SIZE_GPS_POSITION] =
{
    sizeof( (*oli_l0r_gps_ptr).l0r_time.days_from_J2000),
    sizeof( (*oli_l0r_gps_ptr).l0r_time.seconds_of_day),
    sizeof( (*oli_l0r_gps_ptr).month),
    sizeof( (*oli_l0r_gps_ptr).day),
    sizeof( (*oli_l0r_gps_ptr).year),
    sizeof( (*oli_l0r_gps_ptr).hours),
    sizeof( (*oli_l0r_gps_ptr).minutes),
    sizeof( (*oli_l0r_gps_ptr).seconds),
    sizeof( (*oli_l0r_gps_ptr).nanoseconds),
    sizeof( (*oli_l0r_gps_ptr).function),
    sizeof( (*oli_l0r_gps_ptr).sub_function),
    sizeof( (*oli_l0r_gps_ptr).point_arc_secs.lat),
    sizeof( (*oli_l0r_gps_ptr).point_arc_secs.lng),
    sizeof( (*oli_l0r_gps_ptr).height_uncorrected_meters),
    sizeof( (*oli_l0r_gps_ptr).height_corrected_meters),
    sizeof( (*oli_l0r_gps_ptr).velocity_meters_per_sec),
    sizeof( (*oli_l0r_gps_ptr).heading_degrees),
    sizeof( (*oli_l0r_gps_ptr).current_dop),
    sizeof( (*oli_l0r_gps_ptr).dop_type),
    sizeof( (*oli_l0r_gps_ptr).num_visible_satellites),
    sizeof( (*oli_l0r_gps_ptr).num_satellites_tracked),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[0].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[0].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[0].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[0].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[1].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[1].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[1].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[1].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[2].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[2].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[2].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[2].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[3].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[3].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[3].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[3].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[4].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[4].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[4].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[4].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[5].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[5].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[5].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[5].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[6].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[6].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[6].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[6].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[7].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[7].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[7].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[7].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[8].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[8].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[8].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[8].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[9].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[9].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[9].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[9].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[10].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[10].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[10].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[10].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[11].id),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[11].track_mode),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[11].signal_strength),
    sizeof( (*oli_l0r_gps_ptr).tracked_satellites[11].channel_status_flags),
    sizeof( (*oli_l0r_gps_ptr).receiver_status_flags),
    sizeof( (*oli_l0r_gps_ptr).ecef_position_meters.x),
    sizeof( (*oli_l0r_gps_ptr).ecef_position_meters.y),
    sizeof( (*oli_l0r_gps_ptr).ecef_position_meters.z),
    sizeof( (*oli_l0r_gps_ptr).ecef_velocity_meters_per_sec.x),
    sizeof( (*oli_l0r_gps_ptr).ecef_velocity_meters_per_sec.y),
    sizeof( (*oli_l0r_gps_ptr).ecef_velocity_meters_per_sec.z),
    sizeof( (*oli_l0r_gps_ptr).warning_flag)
};

static const size_t OFFSET_GPS_POSITION[TABLE_SIZE_GPS_POSITION] =
{
    HOFFSET( IAS_L0R_GPS_POSITION,l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_GPS_POSITION,l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_GPS_POSITION,month),
    HOFFSET( IAS_L0R_GPS_POSITION,day),
    HOFFSET( IAS_L0R_GPS_POSITION,year),
    HOFFSET( IAS_L0R_GPS_POSITION,hours),
    HOFFSET( IAS_L0R_GPS_POSITION,minutes),
    HOFFSET( IAS_L0R_GPS_POSITION,seconds),
    HOFFSET( IAS_L0R_GPS_POSITION,nanoseconds),
    HOFFSET( IAS_L0R_GPS_POSITION,function),
    HOFFSET( IAS_L0R_GPS_POSITION,sub_function),
    HOFFSET( IAS_L0R_GPS_POSITION,point_arc_secs.lat),
    HOFFSET( IAS_L0R_GPS_POSITION,point_arc_secs.lng),
    HOFFSET( IAS_L0R_GPS_POSITION,height_uncorrected_meters),
    HOFFSET( IAS_L0R_GPS_POSITION,height_corrected_meters),
    HOFFSET( IAS_L0R_GPS_POSITION,velocity_meters_per_sec),
    HOFFSET( IAS_L0R_GPS_POSITION,heading_degrees),
    HOFFSET( IAS_L0R_GPS_POSITION,current_dop),
    HOFFSET( IAS_L0R_GPS_POSITION,dop_type),
    HOFFSET( IAS_L0R_GPS_POSITION,num_visible_satellites),
    HOFFSET( IAS_L0R_GPS_POSITION,num_satellites_tracked),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[0].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[0].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[0].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[0].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[1].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[1].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[1].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[1].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[2].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[2].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[2].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[2].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[3].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[3].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[3].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[3].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[4].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[4].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[4].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[4].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[5].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[5].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[5].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[5].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[6].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[6].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[6].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[6].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[7].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[7].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[7].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[7].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[8].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[8].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[8].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[8].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[9].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[9].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[9].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[9].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[10].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[10].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[10].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[10].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[11].id),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[11].track_mode),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[11].signal_strength),
    HOFFSET( IAS_L0R_GPS_POSITION,tracked_satellites[11].channel_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,receiver_status_flags),
    HOFFSET( IAS_L0R_GPS_POSITION,ecef_position_meters.x),
    HOFFSET( IAS_L0R_GPS_POSITION,ecef_position_meters.y),
    HOFFSET( IAS_L0R_GPS_POSITION,ecef_position_meters.z),
    HOFFSET( IAS_L0R_GPS_POSITION,ecef_velocity_meters_per_sec.x),
    HOFFSET( IAS_L0R_GPS_POSITION,ecef_velocity_meters_per_sec.y),
    HOFFSET( IAS_L0R_GPS_POSITION,ecef_velocity_meters_per_sec.z),
    HOFFSET( IAS_L0R_GPS_POSITION,warning_flag)
};

/******************************************************************************
*GPS_RANGE table data constants
******************************************************************************/
static const hid_t  *field_type_gps_range = NULL;
static hid_t gps_range_types[TABLE_SIZE_GPS_RANGE];

static const char *FIELD_NAMES_GPS_RANGE[TABLE_SIZE_GPS_RANGE] =
{
    "l0r_time_days_from_J2000", /* days from J2000 for attitude estimate */
    "l0r_time_seconds_of_day", /* seconds of the day */
    "seconds",
    "nanoseconds",
    "function",
    "sub_function",
    "id_1",
    "tracking_mode_1",
    "gps_time_seconds_1",
    "gps_time_nanoseconds_1",
    "raw_code_phase_1",
    "integrated_carrier_phase_cycles_1",
    "integrated_carrier_phase_deg_1",
    "code_discriminator_output_1",
    "id_2",
    "tracking_mode_2",
    "gps_time_seconds_2",
    "gps_time_nanoseconds_2",
    "raw_code_phase_2",
    "integrated_carrier_phase_cycles_2",
    "integrated_carrier_phase_deg_2",
    "code_discriminator_output_2",
    "id_3",
    "tracking_mode_3",
    "gps_time_seconds_3",
    "gps_time_nanoseconds_3",
    "raw_code_phase_3",
    "integrated_carrier_phase_cycles_3",
    "integrated_carrier_phase_deg_3",
    "code_discriminator_output_3",
    "id_4",
    "tracking_mode_4",
    "gps_time_seconds_4",
    "gps_time_nanoseconds_4",
    "raw_code_phase_4",
    "integrated_carrier_phase_cycles_4",
    "integrated_carrier_phase_deg_4",
    "code_discriminator_output_4",
    "id_5",
    "tracking_mode_5",
    "gps_time_seconds_5",
    "gps_time_nanoseconds_5",
    "raw_code_phase_5",
    "integrated_carrier_phase_cycles_5",
    "integrated_carrier_phase_deg_5",
    "code_discriminator_output_5",
    "id_6",
    "tracking_mode_6",
    "gps_time_seconds_6",
    "gps_time_nanoseconds_6",
    "raw_code_phase_6",
    "integrated_carrier_phase_cycles_6",
    "integrated_carrier_phase_deg_6",
    "code_discriminator_output_6",
    "id_7",
    "tracking_mode_7",
    "gps_time_seconds_7",
    "gps_time_nanoseconds_7",
    "raw_code_phase_7",
    "integrated_carrier_phase_cycles_7",
    "integrated_carrier_phase_deg_7",
    "code_discriminator_output_7",
    "id_8",
    "tracking_mode_8",
    "gps_time_seconds_8",
    "gps_time_nanoseconds_8",
    "raw_code_phase_8",
    "integrated_carrier_phase_cycles_8",
    "integrated_carrier_phase_deg_8",
    "code_discriminator_output_8",
    "id_9",
    "tracking_mode_9",
    "gps_time_seconds_9",
    "gps_time_nanoseconds_9",
    "raw_code_phase_9",
    "integrated_carrier_phase_cycles_9",
    "integrated_carrier_phase_deg_9",
    "code_discriminator_output_9",
    "id_10",
    "tracking_mode_10",
    "gps_time_seconds_10",
    "gps_time_nanoseconds_10",
    "raw_code_phase_10",
    "integrated_carrier_phase_cycles_10",
    "integrated_carrier_phase_deg_10",
    "code_discriminator_output_10",
    "id_11",
    "tracking_mode_11",
    "gps_time_seconds_11",
    "gps_time_nanoseconds_11",
    "raw_code_phase_11",
    "integrated_carrier_phase_cycles_11",
    "integrated_carrier_phase_deg_11",
    "code_discriminator_output_11",
    "id_12",
    "tracking_mode_12",
    "gps_time_seconds_12",
    "gps_time_nanoseconds_12",
    "raw_code_phase_12",
    "integrated_carrier_phase_cycles_12",
    "integrated_carrier_phase_deg_12",
    "code_discriminator_output_12",
    "warning_flag"
};
static const IAS_L0R_GPS_RANGE *IAS_L0R_GPS_RANGE_PTR = NULL;
static const size_t IAS_L0R_GPS_RANGE_SIZE =  sizeof(IAS_L0R_GPS_RANGE);
static const size_t IAS_L0R_GPS_RANGE_SIZES[TABLE_SIZE_GPS_RANGE] =
{
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).l0r_time.days_from_J2000),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).l0r_time.seconds_of_day),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).function),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).sub_function),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[0].id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[0].tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[0].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[0].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[0].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[0].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[0].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[0].
        code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[1].
        id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[1].
        tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[1].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[1].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[1].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[1].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[1].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[1].
        code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[2].
        id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[2].
        tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[2].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[2].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[2].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[2].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[2].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[2]
        .code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[3].
        id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[3].
        tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[3].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[3].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[3].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[3].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[3].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[3].
        code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[4].
        id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[4].
        tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[4].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[4].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[4].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[4].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[4].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[4].
        code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[5].
        id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[5].
        tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[5].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[5].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[5].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[5].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[5].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[5].
        code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[6].
        id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[6].
        tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[6].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[6].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[6].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[6].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[6].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[6].
        code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[7].
        id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[7].
        tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[7].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[7].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[7].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[7].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[7].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[7].
        code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[8].
        id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[8].
        tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[8].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[8].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[8].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[8].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[8].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[8].
        code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[9].
        id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[9].
        tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[9].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[9].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[9].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[9].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[9].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[9].
        code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[10].
        id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[10].
        tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[10].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[10].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[10].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[10].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[10].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[10].
        code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[11].
        id),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[11].
        tracking_mode),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[11].
        gps_time_seconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[11].
        gps_time_nanoseconds),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[11].
        raw_code_phase),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[11].
        integrated_carrier_phase_cycles),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[11].
        integrated_carrier_phase_deg),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).tracked_satellites[11].
        code_discriminator_output),
    sizeof( (*IAS_L0R_GPS_RANGE_PTR).warning_flag)
};
static const size_t OFFSET_GPS_RANGE[TABLE_SIZE_GPS_RANGE] =
{
    HOFFSET( IAS_L0R_GPS_RANGE,l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_GPS_RANGE,l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_GPS_RANGE,seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,function),
    HOFFSET( IAS_L0R_GPS_RANGE,sub_function),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[0].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[0].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[0].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[0].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[0].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[0].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[0].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[0].code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[1].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[1].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[1].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[1].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[1].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[1].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[1].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[1].code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[2].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[2].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[2].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[2].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[2].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[2].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[2].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[2].code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[3].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[3].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[3].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[3].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[3].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[3].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[3].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[3].code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[4].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[4].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[4].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[4].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[4].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[4].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[4].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[4].code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[5].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[5].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[5].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[5].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[5].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[5].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[5].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[5].code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[6].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[6].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[6].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[6].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[6].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[6].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[6].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[6].code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[7].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[7].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[7].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[7].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[7].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[7].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[7].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[7].code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[8].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[8].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[8].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[8].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[8].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[8].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[8].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[8].code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[9].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[9].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[9].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[9].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[9].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[9].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[9].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[9].code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[10].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[10].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[10].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[10].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[10].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[10].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[10].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[10].
        code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[11].id),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[11].tracking_mode),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[11].gps_time_seconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[11].gps_time_nanoseconds),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[11].raw_code_phase),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[11].
        integrated_carrier_phase_cycles),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[11].
        integrated_carrier_phase_deg),
    HOFFSET( IAS_L0R_GPS_RANGE,tracked_satellites[11].
        code_discriminator_output),
    HOFFSET( IAS_L0R_GPS_RANGE,warning_flag)
};

/******************************************************************************
*IMU GYRO table data constants
******************************************************************************/
static const hid_t  *field_type_imu = NULL;
static hid_t imu_types[TABLE_SIZE_IMU];

static const char *field_names_imu[TABLE_SIZE_IMU] =
{
    "l0r_time_days_from_J2000",
    "l0r_time_seconds_of_day",
    "seconds_original",
    "subseconds_original",
    "gyro_sample_1",
    "gyro_sample_2",
    "gyro_sample_3",
    "gyro_sample_4",
    "gyro_sample_5",
    "gyro_sample_6",
    "gyro_sample_7",
    "gyro_sample_8",
    "gyro_sample_9",
    "gyro_sample_10",
    "gyro_sample_11",
    "gyro_sample_12",
    "gyro_sample_13",
    "gyro_sample_14",
    "gyro_sample_15",
    "gyro_sample_16",
    "gyro_sample_17",
    "gyro_sample_18",
    "gyro_sample_19",
    "gyro_sample_20",
    "gyro_sample_21",
    "gyro_sample_22",
    "gyro_sample_23",
    "gyro_sample_24",
    "gyro_sample_25",
    "gyro_sample_26",
    "gyro_sample_27",
    "gyro_sample_28",
    "gyro_sample_29",
    "gyro_sample_30",
    "gyro_sample_31",
    "gyro_sample_32",
    "gyro_sample_33",
    "gyro_sample_34",
    "gyro_sample_35",
    "gyro_sample_36",
    "gyro_sample_37",
    "gyro_sample_38",
    "gyro_sample_39",
    "gyro_sample_40",
    "gyro_sample_41",
    "gyro_sample_42",
    "gyro_sample_43",
    "gyro_sample_44",
    "gyro_sample_45",
    "gyro_sample_46",
    "gyro_sample_47",
    "gyro_sample_48",
    "gyro_sample_49",
    "gyro_sample_50",
    "warning_flag"
};
static const IAS_L0R_IMU *IAS_L0R_IMU_PTR = NULL;
static const size_t IAS_L0R_IMU_SIZE =  sizeof(IAS_L0R_IMU);
static const size_t IAS_L0R_IMU_SIZES[TABLE_SIZE_IMU] =
{
    sizeof( (*IAS_L0R_IMU_PTR).l0r_time.days_from_J2000),
    sizeof( (*IAS_L0R_IMU_PTR).l0r_time.seconds_of_day),
    sizeof( (*IAS_L0R_IMU_PTR).time_tag_sec_orig),
    sizeof( (*IAS_L0R_IMU_PTR).time_tag_subseconds_orig),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[0]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[1]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[2]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[3]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[4]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[5]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[6]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[7]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[8]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[9]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[10]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[11]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[12]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[13]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[14]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[15]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[16]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[17]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[18]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[19]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[20]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[21]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[22]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[23]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[24]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[25]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[26]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[27]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[28]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[29]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[30]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[31]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[32]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[33]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[34]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[35]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[36]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[37]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[38]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[39]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[40]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[41]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[42]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[43]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[44]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[45]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[46]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[47]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[48]),
    sizeof( (*IAS_L0R_IMU_PTR).gyro_samples[49]),
    sizeof( (*IAS_L0R_IMU_PTR).warning_flag)
};

static const size_t OFFSET_IMU[TABLE_SIZE_IMU] =
{
    HOFFSET( IAS_L0R_IMU, l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_IMU, l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_IMU, time_tag_sec_orig),
    HOFFSET( IAS_L0R_IMU, time_tag_subseconds_orig),
    HOFFSET( IAS_L0R_IMU, gyro_samples[0]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[1]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[2]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[3]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[4]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[5]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[6]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[7]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[8]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[9]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[10]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[11]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[12]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[13]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[14]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[15]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[16]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[17]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[18]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[19]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[20]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[21]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[22]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[23]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[24]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[25]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[26]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[27]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[28]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[29]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[30]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[31]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[32]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[33]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[34]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[35]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[36]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[37]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[38]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[39]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[40]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[41]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[42]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[43]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[44]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[45]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[46]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[47]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[48]),
    HOFFSET( IAS_L0R_IMU, gyro_samples[49]),
    HOFFSET( IAS_L0R_IMU, warning_flag)
};

/******************************************************************************
*IMU Latency table data constants
******************************************************************************/
static const hid_t  *field_type_imu_latency = NULL;
static hid_t imu_latency_types[TABLE_SIZE_IMU_LATENCY];

static const char *field_names_imu_latency[TABLE_SIZE_IMU_LATENCY] =
{
    "l0r_time_days_from_J2000",
    "l0r_time_seconds_of_day",
    "fine_ad_solution_time",
    "measured_imu_latency",
    "warning_flag"
};

static const IAS_L0R_IMU_LATENCY *IAS_L0R_IMU_LATENCY_PTR = NULL;
static const size_t IAS_L0R_IMU_LATENCY_SIZE =  sizeof(IAS_L0R_IMU_LATENCY);
static const size_t IAS_L0R_IMU_LATENCY_SIZES[TABLE_SIZE_IMU_LATENCY] =
{
    sizeof( (*IAS_L0R_IMU_LATENCY_PTR).l0r_time.days_from_J2000),
    sizeof( (*IAS_L0R_IMU_LATENCY_PTR).l0r_time.seconds_of_day),
    sizeof( (*IAS_L0R_IMU_LATENCY_PTR).fine_ad_solution_time),
    sizeof( (*IAS_L0R_IMU_LATENCY_PTR).measured_imu_latency),
    sizeof( (*IAS_L0R_IMU_LATENCY_PTR).warning_flag)
};

static const size_t OFFSET_IMU_LATENCY[TABLE_SIZE_IMU_LATENCY] =
{
    HOFFSET( IAS_L0R_IMU_LATENCY, l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_IMU_LATENCY, l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_IMU_LATENCY, fine_ad_solution_time),
    HOFFSET( IAS_L0R_IMU_LATENCY, measured_imu_latency),
    HOFFSET( IAS_L0R_IMU_LATENCY, warning_flag)
};

/******************************************************************************
* OLI telemetry data
******************************************************************************/
static const hid_t  *field_type_telemetry_group_3 = NULL;
static hid_t telemetry_group_3_types
                                [TABLE_SIZE_TELEMETRY_GROUP_3];

static const char *FIELD_NAMES_TELEMETRY_GROUP_3
                                [TABLE_SIZE_TELEMETRY_GROUP_3] =
{
    "l0r_time_days_from_J2000", /* days from J2000 for attitude estimate */
    "l0r_time_seconds_of_day", /* seconds of the day */
    "days_original",
    "milliseconds_original",
    "microseconds_original",
    "sync_word",
    "id",
    "stim_lamp_output_current_amps",
    "stim_lamp_bulb_a_volts",
    "stim_lamp_bulb_b_volts",
    "stim_lamp_thermistor1_celsius",
    "stim_lamp_thermistor2_celsius",
    "stim_lamp_photodiode1_micro_amps",
    "stim_lamp_photodiode2_micro_amps",
    "focus_motor_lvdt_1",
    "focus_motor_lvdt_2",
    "focus_motor_lvdt_3",
    "pos_z_minus_y_temp_celsius",
    "bench_temp_1_celsius",
    "bench_temp_2_celsius",
    "bench_temp_3_celsius",
    "bench_temp_4_celsius",
    "bench_temp_5_celsius",
    "bench_temp_7_celsius",
    "bench_temp_8_celsius",
    "fpm_7_temp_celsius",
    "calibration_assembly_a_temp_celsius",
    "pos_z_pos_y_temp_celsius",
    "tert_mirror_temp_celsius",
    "fp_chassis_temp_celsius",
    "pos_y_temp_celsius",
    "fp_evap_temp_celsius",
    "fp_window_temp_celsius",
    "minus_z_pos_y_temp_celsius",
    "minus_z_minus_y_temp_celsius",
    "minus_y_temp_celsius",
    "fpm_14_temp_celsius",
    "lvps_temp_celsius",
    "reserved",
    "spare",
    "warning_flag"
};
static const IAS_L0R_OLI_TELEMETRY_GROUP_3* IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR;
static const size_t IAS_L0R_OLI_TELEMETRY_GROUP_3_SIZE =
                                sizeof(IAS_L0R_OLI_TELEMETRY_GROUP_3);
static const size_t IAS_L0R_OLI_TELEMETRY_GROUP_3_SIZES
                                [TABLE_SIZE_TELEMETRY_GROUP_3] =
{
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).
                                    l0r_time.days_from_J2000),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).l0r_time.seconds_of_day),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).days_orig),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).milliseconds_orig),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).microseconds_orig),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).sync_word),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).id),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).stim_lamp_output_current_amps),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).stim_lamp_bulb_a_volts),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).stim_lamp_bulb_b_volts),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).stim_lamp_thermistor1_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).stim_lamp_thermistor2_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).
        stim_lamp_photodiode1_micro_amps),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).
        stim_lamp_photodiode2_micro_amps),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).focus_motor_lvdt[0]),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).focus_motor_lvdt[1]),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).focus_motor_lvdt[2]),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).pos_z_minus_y_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).bench_temps_celsius[0]),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).bench_temps_celsius[1]),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).bench_temps_celsius[2]),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).bench_temps_celsius[3]),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).bench_temps_celsius[4]),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).bench_temps_celsius[5]),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).bench_temps_celsius[6]),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).fpm_7_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).
        calibration_assembly_a_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).pos_z_pos_y_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).tertiary_mirror_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).fp_chassis_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).pos_y_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).fp_evap_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).fp_window_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).minus_z_pos_y_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).minus_z_minus_y_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).minus_y_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).fpm_14_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).lvps_temp_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).reserved),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).spare),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_3_PTR).warning_flag)
};
 
static const size_t OFFSET_TELEMETRY_GROUP_3[TABLE_SIZE_TELEMETRY_GROUP_3] =
{
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, days_orig),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, milliseconds_orig),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, microseconds_orig),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, sync_word),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, id),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, stim_lamp_output_current_amps),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, stim_lamp_bulb_a_volts),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, stim_lamp_bulb_b_volts),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, stim_lamp_thermistor1_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, stim_lamp_thermistor2_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, stim_lamp_photodiode1_micro_amps),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, stim_lamp_photodiode2_micro_amps),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, focus_motor_lvdt[0]),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, focus_motor_lvdt[1]),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, focus_motor_lvdt[2]),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, pos_z_minus_y_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, bench_temps_celsius[0]),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, bench_temps_celsius[1]),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, bench_temps_celsius[2]),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, bench_temps_celsius[3]),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, bench_temps_celsius[4]),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, bench_temps_celsius[5]),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, bench_temps_celsius[6]),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, fpm_7_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3,
        calibration_assembly_a_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, pos_z_pos_y_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, tertiary_mirror_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, fp_chassis_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, pos_y_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, fp_evap_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, fp_window_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, minus_z_pos_y_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, minus_z_minus_y_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, minus_y_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, fpm_14_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, lvps_temp_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, reserved),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, spare),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_3, warning_flag)
};

static const hid_t *field_type_telemetry_oli_group_4;
static hid_t telemetry_group_4_types[TABLE_SIZE_TELEMETRY_GROUP_4];

static const char *FIELD_NAMES_PAYLOAD_OLI_GROUP_4
                                [TABLE_SIZE_TELEMETRY_GROUP_4] =
{
    "l0r_time_days_from_J2000", /* days from J2000 for attitude estimate */
    "l0r_time_seconds_of_day", /* seconds of the day */
    "days_original",
    "milliseconds_original",
    "microseconds_original",
    "sync_word",
    "id",
    "mech_command_reject_count",
    "mech_command_accept_count",
    "shutter_active",
    "last_command_opcode",
    "diffuser_active",
    "shutter_commanded_moves",
    "focus_motor_flags",
    "diffuser_commanded_moves",
    "focus_motor_pulse_time_step_sec",
    "focus_motor_pulse_length_sec",
    "focus_motor_pulses",
    "focus_mechanism_lvdt_relay_status",
    "focus_mechanism_motor_relay_status",
    "shutter_motor_pulse_length_sec",
    "shutter_status_flags",
    "diffuser_status_flags",
    "shutter_motor_pulse_time_sec",
    "diffuser_motor_pulse_time_sec",
    "diffuser_motor_pulse_length_sec",
    "shutter_move_count",
    "shutter_resolver_position",
    "diffuser_move_count",
    "diffuser_resolver_position",
    "diffuser_flags",
    "stl_command_rejected_count",
    "stl_command_accepted_count",
    "stl_power_flags",
    "stl_last_accepted_command",
    "stl_flags",
    "reserved",
    "spare",
    "warning_flag"
};
static const IAS_L0R_OLI_TELEMETRY_GROUP_4* IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR;
static const size_t IAS_L0R_OLI_TELEMETRY_GROUP_4_SIZE =
                                sizeof(IAS_L0R_OLI_TELEMETRY_GROUP_4);
static const size_t IAS_L0R_OLI_TELEMETRY_GROUP_4_SIZES
                                [TABLE_SIZE_TELEMETRY_GROUP_4] =
{
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).l0r_time.
                                            days_from_J2000),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).l0r_time.
                                            seconds_of_day),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).days_orig),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).milliseconds_orig),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).microseconds_orig),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).sync_word),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).id),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).mech_command_reject_count),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).mech_command_accept_count),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).shutter_active),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).last_command_opcode),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).diffuser_active),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).shutter_commanded_moves),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).focus_motor_flags),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).diffuser_commanded_moves),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).
        focus_motor_pulse_time_step_sec),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).focus_motor_pulse_length_sec),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).focus_motor_pulses),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).
        focus_mechanism_lvdt_relay_status),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).
        focus_mechanism_motor_relay_status),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).
        shutter_motor_pulse_length_sec),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).shutter_status_flags),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).diffuser_status_flags),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).shutter_motor_pulse_time_sec),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).diffuser_motor_pulse_time_sec),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).
        diffuser_motor_pulse_length_sec),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).shutter_move_count),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).shutter_resolver_position),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).diffuser_move_count),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).diffuser_resolver_position),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).diffuser_flags),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).stl_command_rejected_count),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).stl_command_accepted_count),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).stl_power_flags),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).stl_last_accepted_command),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).stl_flags),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).reserved),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).spare),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_4_PTR).warning_flag)
};
 
static const size_t OFFSET_PAYLOAD_OLI_GROUP_4[TABLE_SIZE_TELEMETRY_GROUP_4] =
{
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,days_orig),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,milliseconds_orig),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,microseconds_orig),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,sync_word),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,id),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,mech_command_reject_count),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,mech_command_accept_count),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,shutter_active),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,last_command_opcode),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,diffuser_active),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,shutter_commanded_moves),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,focus_motor_flags),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,diffuser_commanded_moves),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,focus_motor_pulse_time_step_sec),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,focus_motor_pulse_length_sec),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,focus_motor_pulses),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,focus_mechanism_lvdt_relay_status),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,focus_mechanism_motor_relay_status),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,shutter_motor_pulse_length_sec),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,shutter_status_flags),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,diffuser_status_flags),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,shutter_motor_pulse_time_sec),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,diffuser_motor_pulse_time_sec),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,diffuser_motor_pulse_length_sec),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,shutter_move_count),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,shutter_resolver_position),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,diffuser_move_count),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,diffuser_resolver_position),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,diffuser_flags),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,stl_command_rejected_count),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,stl_command_accepted_count),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,stl_power_flags),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,stl_last_accepted_command),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,stl_flags),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,reserved),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,spare),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_4,warning_flag)
};

static const hid_t *field_type_telemetry_oli_group_5;
static hid_t telemetry_group_5_types[TABLE_SIZE_TELEMETRY_GROUP_5];
static const char *FIELD_NAMES_PAYLOAD_OLI_GROUP_5
    [TABLE_SIZE_TELEMETRY_GROUP_5] =
{
    "l0r_time_days_from_J2000", /* days from J2000 for attitude estimate */
    "l0r_time_seconds_of_day", /* seconds of the day */
    "days_original",
    "milliseconds_original",
    "microseconds_original",
    "sync_word",
    "id",
    "fpe_command_reject_count",
    "fpe_command_accept_count",
    "safe_mode_consecutive_requests",
    "last_command_opcode",
    "single_bit_edac_errors_detected",
    "consecutive_unacknowledged_requests",
    "fpe_message_errors_detected",
    "multi_bit_edac_errors_detected",
    "messages_forwarded_to_fpe",
    "command_sequence_count",
    "messages_reject_invalid_mode",
    "fpe_telemetry_valid",
    "dlvps_relay_pos_28vdc_voltage",
    "dlvps_pos_5v_voltage",
    "dlvps_pos_15v_voltage",
    "dlvps_neg_15v_voltage",
    "dlvps_pos_3_3v_voltage",
    "alvps_hv_bias_pos_85v_voltage",
    "alvps_pos_12v_voltage",
    "alvps_pos_7_5v_voltage",
    "alvps_neg_2_5v_voltage",
    "alvps_pos_12v_current_amps",
    "alvps_pos_7_5v_current_amps",
    "alvps_pos_2_5v_current_amps",
    "lvps_temperature_sensor_celsius",
    "ctlr_temperature_sensor_celsius",
    "ana_0_temperature_sensor_celsius",
    "ana_1_temperature_sensor_celsius",
    "ana_0_ch_0_vpa_bias_voltage",
    "ana_0_ch_1_vpa_bias_voltage",
    "ana_0_ch_2_vpa_bias_voltage",
    "ana_0_ch_3_vpa_bias_voltage",
    "ana_0_ch_4_vpa_bias_voltage",
    "ana_0_ch_5_vpa_bias_voltage",
    "ana_0_ch_6_vpa_bias_voltage",
    "ana_0_ch_7_vpa_bias_voltage",
    "reserved",
    "spare",
    "warning_flag"
};
static const IAS_L0R_OLI_TELEMETRY_GROUP_5* IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR;
static const size_t IAS_L0R_OLI_TELEMETRY_GROUP_5_SIZE =
    sizeof(IAS_L0R_OLI_TELEMETRY_GROUP_5);
static const size_t IAS_L0R_OLI_TELEMETRY_GROUP_5_SIZES
    [TABLE_SIZE_TELEMETRY_GROUP_5] =
{
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).l0r_time.days_from_J2000),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).l0r_time.
        seconds_of_day),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        days_orig),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        milliseconds_orig),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        microseconds_orig),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        sync_word),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        id),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        fpe_command_reject_count),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        fpe_command_accept_count),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        safe_mode_consecutive_requests),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        last_command_opcode),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        single_bit_edac_errors_detected),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        consecutive_unacknowledged_requests),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        fpe_message_errors_detected),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        multi_bit_edac_errors_detected),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        messages_forwarded_to_fpe),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        command_sequence_count),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        messages_reject_invalid_mode),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        fpe_telemetry_valid),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        dlvps_relay_pos_28vdc_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        dlvps_pos_5v_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        dlvps_pos_15v_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        dlvps_neg_15v_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        dlvps_pos_3_3v_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        alvps_hv_bias_pos_85v_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        alvps_pos_12v_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        alvps_pos_7_5v_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        alvps_neg_2_5v_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        alvps_pos_12v_current_amps),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        alvps_pos_7_5v_current_amps),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        alvps_pos_2_5v_current_amps),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        lvps_temperature_sensor_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        ctlr_temperature_sensor_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        ana_0_temperature_sensor_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        ana_1_temperature_sensor_celsius),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        ana_0_ch_0_vpa_bias_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        ana_0_ch_1_vpa_bias_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        ana_0_ch_2_vpa_bias_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        ana_0_ch_3_vpa_bias_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        ana_0_ch_4_vpa_bias_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        ana_0_ch_5_vpa_bias_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        ana_0_ch_6_vpa_bias_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        ana_0_ch_7_vpa_bias_voltage),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        reserved),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).
        spare),
    sizeof( (*IAS_L0R_OLI_TELEMETRY_GROUP_5_PTR).warning_flag)
};
 
static const size_t OFFSET_PAYLOAD_OLI_GROUP_5[TABLE_SIZE_TELEMETRY_GROUP_5] =
{
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, days_orig),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, milliseconds_orig),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, microseconds_orig),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, sync_word),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, id),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, fpe_command_reject_count),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, fpe_command_accept_count),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, safe_mode_consecutive_requests),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, last_command_opcode),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, single_bit_edac_errors_detected),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5,
        consecutive_unacknowledged_requests),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, fpe_message_errors_detected),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, multi_bit_edac_errors_detected),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, messages_forwarded_to_fpe),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, command_sequence_count),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, messages_reject_invalid_mode),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, fpe_telemetry_valid),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, dlvps_relay_pos_28vdc_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, dlvps_pos_5v_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, dlvps_pos_15v_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, dlvps_neg_15v_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, dlvps_pos_3_3v_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, alvps_hv_bias_pos_85v_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, alvps_pos_12v_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, alvps_pos_7_5v_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, alvps_neg_2_5v_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, alvps_pos_12v_current_amps),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, alvps_pos_7_5v_current_amps),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, alvps_pos_2_5v_current_amps),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, lvps_temperature_sensor_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, ctlr_temperature_sensor_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, ana_0_temperature_sensor_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, ana_1_temperature_sensor_celsius),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, ana_0_ch_0_vpa_bias_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, ana_0_ch_1_vpa_bias_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, ana_0_ch_2_vpa_bias_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, ana_0_ch_3_vpa_bias_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, ana_0_ch_4_vpa_bias_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, ana_0_ch_5_vpa_bias_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, ana_0_ch_6_vpa_bias_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, ana_0_ch_7_vpa_bias_voltage),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, reserved),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, spare),
    HOFFSET( IAS_L0R_OLI_TELEMETRY_GROUP_5, warning_flag)
};

/******************************************************************************
* TIRS telemetry data
******************************************************************************/
static const hid_t  *field_type_tirs_telemetry = NULL;
static hid_t tirs_telemetry_types[TABLE_SIZE_TIRS_TELEMETRY];

static const char *FIELD_NAMES_TIRS_TELEMETRY[TABLE_SIZE_TIRS_TELEMETRY] =
{
    "l0r_time_days_from_J2000", /* days from J2000 */
    "l0r_time_seconds_of_day", /* seconds of the day */
    "unaccepted_command_count",
    "accepted_command_count",
    "pulse_per_second_count",
    "tod_command_counter",
    "day",
    "millisecond",
    "mc_encoder_flags",
    "science_data_frame_capture_count",
    "science_acquisition_frame_rate",
    "active_timing_table_pattern",
    "mode_register",
    "timing_table_pattern_id_1",
    "timing_table_pattern_id_2",
    "timing_table_pattern_id_3",
    "ssm_position_sel",
    "ssm_mech_mode",
    "ssm_encoder_position_sample",
    "bbcal_op7_a_celsius",
    "bbcal_op7_b_celsius",
    "bbcal_supp_1_celsius",
    "blackbody_calibrator_celsius",
    "cold_stage_heat_strap_cf_if_celsius",
    "cryo_diode_t3_measured_celsius",
    "cryo_diode_t4_measured_celsius",
    "cryo_shroud_outer_at_tunnel_celsius",
    "cryo_shroud_outer_flange_celsius",
    "fixed_baff_nadir_aft_hot_corner_celsius",
    "fixed_baff_nadir_aft_space_corner_celsius",
    "fixed_baff_nadir_fwd_hot_corner_celsius",
    "fixed_baff_nadir_fwd_space_corner_celsius",
    "fp_a_asic_celsius",
    "fp_b_asic_celsius",
    "fpe1_fpe_a_asic_celsius",
    "fpe2_fpe_b_asic_celsius",
    "fp_f2_fine_sensor_1_celsius",
    "fp_f4_fine_sensor_3_celsius",
    "fp_f6_fine_sensor_1_celsius",
    "fp_f7_fine_sensor_2_celsius",
    "fp_op6_a_celsius",
    "fp_op6_b_celsius",
    "optical_deck_celsius",
    "spare_4_thermistor_celsius",
    "spare_5_thermistor_celsius",
    "ssm_bearing_aft_celsius",
    "ssm_bearing_fwd_celsius",
    "ssm_bearing_housing_d4_aft_hot_side_celsius",
    "ssm_bearing_housing_d5_fwd_hot_side_celsius",
    "ssm_bearing_housing_d6_aft_space_side_celsius",
    "ssm_bearing_housing_d7_fwd_space_side_celsius",
    "ssm_bh_op5_a_celsius",
    "ssm_bh_op5_b_celsius",
    "ssm_encoder_remote_elec_celsius",
    "ssm_enc_read_head_sensor_1_celsius",
    "ssm_motor_housing_celsius",
    "structure_foot_a_neg_z_celsius",
    "structure_foot_c_pos_z_celsius",
    "structure_nadir_aperture_celsius",
    "tcb_board_celsius",
    "telescope_aft_barrel_neg_z_celsius",
    "telescope_aft_barrel_pos_z_celsius",
    "telescope_aft_op3_a_celsius",
    "telescope_aft_op3_b_celsius",
    "telescope_fwd_barrel_neg_z_celsius",
    "telescope_fwd_barrel_pos_z_celsius",
    "telescope_fwd_op4_a_celsius",
    "telescope_fwd_op4_b_celsius",
    "telescope_stage_op2_a_celsius",
    "telescope_stage_op2_b_celsius",
    "fp_a_mon_pos_12v_volts",
    "fp_a_a_vpd_current_1_amps_1",
    "fp_a_a_vpd_current_1_amps_2",
    "fp_a_a_vpd_current_1_amps_3",
    "fp_a_detector_substrate_conn_for_sca_a_roic_volts",
    "fp_a_detector_substrate_conn_for_sca_b_roic_volts",
    "fp_a_detector_substrate_conn_for_sca_c_roic_volts",
    "fp_a_digi_supply_mon_pos_5_5_for_sca_c_roic_volts",
    "fp_a_supply_mon_pos_5_5_for_sca_a_roic_volts",
    "fp_a_supply_mon_pos_5_5_for_sca_b_roic_volts",
    "fp_a_supply_mon_pos_5_5_for_sca_c_roic_volts",
    "fp_a_output_ref_level_mon_5_5_for_sca_c_roic_volts",
    "fp_a_supply_10v_for_sca_a_current_mon_amps",
    "fp_a_supply_10v_for_sca_b_current_mon_amps",
    "fp_a_supply_10v_for_sca_c_current_mon_amps",
    "fp_a_output_driver_pos_5_5_for_sca_c_roic_volts",
    "fp_a_output_ref_level_1_6_for_sca_c_roic_volts",
    "fp_a_channel_ref_suppy_1_6_for_sca_c_roic_volts",
    "vpe_a_sca_a_video_ref",
    "vpe_a_sca_b_video_ref",
    "vpe_a_sca_c_video_ref",
    "fp_b_mon_pos_12v_volts",
    "fp_b_a_vpd_current_1_amps_1",
    "fp_b_a_vpd_current_1_amps_2",
    "fp_b_a_vpd_current_1_amps_3",
    "fp_b_detector_substrate_conn_for_sca_a_roic_volts",
    "fp_b_detector_substrate_conn_for_sca_b_roic_volts",
    "fp_b_detector_substrate_conn_for_sca_c_roic_volts",
    "fp_b_digi_supply_mon_pos_5_5_for_sca_c_roic_volts",
    "fp_b_supply_mon_pos_5_5_for_sca_a_roic_volts",
    "fp_b_supply_mon_pos_5_5_for_sca_b_roic_volts",
    "fp_b_supply_mon_pos_5_5_for_sca_c_roic_volts",
    "fp_b_output_ref_level_mon_5_5_for_sca_c_roic_volts",
    "fp_b_supply_10v_for_sca_a_current_mon_amps",
    "fp_b_supply_10v_for_sca_b_current_mon_amps",
    "fp_b_supply_10v_for_sca_c_current_mon_amps",
    "fp_b_output_driver_pos_5_5_for_sca_c_roic_volts",
    "fp_b_output_ref_level_1_6_for_sca_c_roic_volts",
    "fp_b_channel_ref_suppy_1_6_for_sca_c_roic_volts",
    "vpe_b_sca_a_video_ref",
    "vpe_b_sca_b_video_ref",
    "vpe_b_sca_c_video_ref",
    "cosine_motor_drive_for_mce_current_amps",
    "sine_motor_drive_for_mce_current_amps",
    "hsib_3_3_current_mon_amps",
    "cosine_dac_telemetry_for_mce_volts",
    "sine_dac_telemetry_for_mce_volts",
    "elec_enabled_flags",
    "reserved_block_2",
    "reserved_block_3",
    "reserved_block_4",
    "warning_flag"
};
static const IAS_L0R_TIRS_TELEMETRY *IAS_L0R_TIRS_TELEMETRY_PTR = NULL;
static const size_t IAS_L0R_TIRS_TELEMETRY_SIZE =
    sizeof(IAS_L0R_TIRS_TELEMETRY);
static const size_t IAS_L0R_TIRS_TELEMETRY_SIZES[TABLE_SIZE_TIRS_TELEMETRY] =
{
    sizeof( (*IAS_L0R_TIRS_TELEMETRY_PTR).l0r_time.days_from_J2000),
    sizeof( (*IAS_L0R_TIRS_TELEMETRY_PTR).l0r_time.seconds_of_day),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).command.unaccepted_command_count),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).command.accepted_command_count),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).command.pulse_per_second_count),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).command.tod_command_counter),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).command.day),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).command.millisecond),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).command.mc_encoder_flags),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        command.science_data_frame_capture_count),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        command.science_acquisition_frame_rate),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        command.active_timing_table_pattern),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).command.mode_register),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        command.timing_table_pattern_id[0]),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        command.timing_table_pattern_id[1]),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        command.timing_table_pattern_id[2]),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).command.ssm_position_sel),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).command.ssm_mech_mode),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).command.ssm_encoder_position_sample),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.bbcal_op7_a_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.bbcal_op7_b_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.bbcal_supp_1_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.blackbody_calibrator_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.cold_stage_heat_strap_cf_if_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.cryo_diode_t3_measured_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.cryo_diode_t4_measured_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.cryo_shroud_outer_at_tunnel_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.cryo_shroud_outer_flange_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fixed_baff_nadir_aft_hot_corner_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fixed_baff_nadir_aft_space_corner_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fixed_baff_nadir_fwd_hot_corner_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fixed_baff_nadir_fwd_space_corner_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fp_a_asic_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fp_b_asic_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fpe1_fpe_a_asic_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fpe2_fpe_b_asic_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fp_f2_fine_sensor_1_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fp_f4_fine_sensor_3_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fp_f6_fine_sensor_1_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fp_f7_fine_sensor_2_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fp_op6_a_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.fp_op6_b_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.optical_deck_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.spare_4_thermistor_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.spare_5_thermistor_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.ssm_bearing_aft_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.ssm_bearing_fwd_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.ssm_bearing_housing_d4_aft_hot_side_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.ssm_bearing_housing_d5_fwd_hot_side_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.ssm_bearing_housing_d6_aft_space_side_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.ssm_bearing_housing_d7_fwd_space_side_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.ssm_bh_op5_a_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.ssm_bh_op5_b_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.ssm_encoder_remote_elec_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.ssm_enc_read_head_sensor_1_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.ssm_motor_housing_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.structure_foot_a_neg_z_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.structure_foot_c_pos_z_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.structure_nadir_aperture_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.tcb_board_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.telescope_aft_barrel_neg_z_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.telescope_aft_barrel_pos_z_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.telescope_aft_op3_a_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.telescope_aft_op3_b_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.telescope_fwd_barrel_neg_z_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.telescope_fwd_barrel_pos_z_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.telescope_fwd_op4_a_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.telescope_fwd_op4_b_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.telescope_stage_op2_a_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        temperature.telescope_stage_op2_b_celsius),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[0].mon_pos_12v_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[0].a_vpd_current_1_amps[0]),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[0].a_vpd_current_1_amps[1]),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[0].a_vpd_current_1_amps[2]),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[0].
        detector_substrate_conn_for_sca_a_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[0].
        detector_substrate_conn_for_sca_b_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[0].
        detector_substrate_conn_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[0].
        digi_supply_mon_pos_5_5_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[0].supply_mon_pos_5_5_for_sca_a_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[0].supply_mon_pos_5_5_for_sca_b_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[0].supply_mon_pos_5_5_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[0].
        output_ref_level_mon_5_5_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[0].supply_10v_for_sca_a_current_mon_amps),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[0].supply_10v_for_sca_b_current_mon_amps),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[0].supply_10v_for_sca_c_current_mon_amps),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[0].
        output_driver_pos_5_5_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[0].
        output_ref_level_1_6_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[0].
        channel_ref_suppy_1_6_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[0].
        vpe_sca_a_video_ref),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[0].
        vpe_sca_b_video_ref),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[0].
        vpe_sca_c_video_ref),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[1].mon_pos_12v_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[1].a_vpd_current_1_amps[0]),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[1].a_vpd_current_1_amps[1]),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[1].a_vpd_current_1_amps[2]),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[1].
        detector_substrate_conn_for_sca_a_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[1].
        detector_substrate_conn_for_sca_b_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[1].
        detector_substrate_conn_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[1].
        digi_supply_mon_pos_5_5_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[1].supply_mon_pos_5_5_for_sca_a_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[1].supply_mon_pos_5_5_for_sca_b_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[1].supply_mon_pos_5_5_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[1].
        output_ref_level_mon_5_5_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[1].supply_10v_for_sca_a_current_mon_amps),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[1].supply_10v_for_sca_b_current_mon_amps),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.focal_plane_boards[1].supply_10v_for_sca_c_current_mon_amps),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[1].
        output_driver_pos_5_5_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[1].
        output_ref_level_1_6_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[1].
        channel_ref_suppy_1_6_for_sca_c_roic_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[1].
        vpe_sca_a_video_ref),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[1].
        vpe_sca_b_video_ref),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).circuit.focal_plane_boards[1].
        vpe_sca_c_video_ref),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.cosine_motor_drive_for_mce_current_amps),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.sine_motor_drive_for_mce_current_amps),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.hsib_3_3_current_mon_amps),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.cosine_dac_telemetry_for_mce_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.sine_dac_telemetry_for_mce_volts),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).
        circuit.elec_enabled_flags),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).reserved_block_2),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).reserved_block_3),
    sizeof ((*IAS_L0R_TIRS_TELEMETRY_PTR).reserved_block_4),
    sizeof( (*IAS_L0R_TIRS_TELEMETRY_PTR).warning_flag)
};
 
static const size_t OFFSET_TIRS_TELEMETRY[TABLE_SIZE_TIRS_TELEMETRY] =
{
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        command.unaccepted_command_count),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        command.accepted_command_count),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        command.pulse_per_second_count),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        command.tod_command_counter),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, command.day),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, command.millisecond),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, command.mc_encoder_flags),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        command.science_data_frame_capture_count),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        command.science_acquisition_frame_rate),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        command.active_timing_table_pattern),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, command.mode_register),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        command.timing_table_pattern_id[0]),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        command.timing_table_pattern_id[1]),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        command.timing_table_pattern_id[2]),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, command.ssm_position_sel),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, command.ssm_mech_mode),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        command.ssm_encoder_position_sample),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.bbcal_op7_a_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.bbcal_op7_b_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.bbcal_supp_1_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        temperature.blackbody_calibrator_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        temperature.cold_stage_heat_strap_cf_if_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        temperature.cryo_diode_t3_measured_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        temperature.cryo_diode_t4_measured_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        temperature.cryo_shroud_outer_at_tunnel_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        temperature.cryo_shroud_outer_flange_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        temperature.fixed_baff_nadir_aft_hot_corner_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        temperature.fixed_baff_nadir_aft_space_corner_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        temperature.fixed_baff_nadir_fwd_hot_corner_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        temperature.fixed_baff_nadir_fwd_space_corner_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.fp_a_asic_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.fp_b_asic_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.fpe1_fpe_a_asic_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.fpe2_fpe_b_asic_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.fp_f2_fine_sensor_1_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.fp_f4_fine_sensor_3_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.fp_f6_fine_sensor_1_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.fp_f7_fine_sensor_2_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.fp_op6_a_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.fp_op6_b_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.optical_deck_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.spare_4_thermistor_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.spare_5_thermistor_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.ssm_bearing_aft_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.ssm_bearing_fwd_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.ssm_bearing_housing_d4_aft_hot_side_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.ssm_bearing_housing_d5_fwd_hot_side_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.ssm_bearing_housing_d6_aft_space_side_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.ssm_bearing_housing_d7_fwd_space_side_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.ssm_bh_op5_a_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.ssm_bh_op5_b_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.ssm_encoder_remote_elec_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.ssm_enc_read_head_sensor_1_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.ssm_motor_housing_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.structure_foot_a_neg_z_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.structure_foot_c_pos_z_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.structure_nadir_aperture_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.tcb_board_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.telescope_aft_barrel_neg_z_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.telescope_aft_barrel_pos_z_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.telescope_aft_op3_a_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.telescope_aft_op3_b_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.telescope_fwd_barrel_neg_z_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
         temperature.telescope_fwd_barrel_pos_z_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.telescope_fwd_op4_a_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.telescope_fwd_op4_b_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.telescope_stage_op2_a_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, temperature.telescope_stage_op2_b_celsius),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[0].mon_pos_12v_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[0].a_vpd_current_1_amps[0]),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[0].a_vpd_current_1_amps[1]),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[0].a_vpd_current_1_amps[2]),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[0].
        detector_substrate_conn_for_sca_a_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[0].
        detector_substrate_conn_for_sca_b_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[0].
        detector_substrate_conn_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[0].
        digi_supply_mon_pos_5_5_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[0].supply_mon_pos_5_5_for_sca_a_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[0].supply_mon_pos_5_5_for_sca_b_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[0].supply_mon_pos_5_5_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[0].
        output_ref_level_mon_5_5_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[0].supply_10v_for_sca_a_current_mon_amps),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[0].supply_10v_for_sca_b_current_mon_amps),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[0].supply_10v_for_sca_c_current_mon_amps),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[0].
        output_driver_pos_5_5_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[0].
        output_ref_level_1_6_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[0].
        channel_ref_suppy_1_6_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[0].
        vpe_sca_a_video_ref),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[0].
        vpe_sca_b_video_ref),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[0].
        vpe_sca_c_video_ref),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[1].mon_pos_12v_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[1].a_vpd_current_1_amps[0]),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[1].a_vpd_current_1_amps[1]),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[1].a_vpd_current_1_amps[2]),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[1].
        detector_substrate_conn_for_sca_a_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[1].
        detector_substrate_conn_for_sca_b_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[1].
        detector_substrate_conn_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[1].
        digi_supply_mon_pos_5_5_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[1].supply_mon_pos_5_5_for_sca_a_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[1].supply_mon_pos_5_5_for_sca_b_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[1].supply_mon_pos_5_5_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[1].
        output_ref_level_mon_5_5_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[1].supply_10v_for_sca_a_current_mon_amps),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[1].supply_10v_for_sca_b_current_mon_amps),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.focal_plane_boards[1].supply_10v_for_sca_c_current_mon_amps),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[1].
        output_driver_pos_5_5_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[1].
        output_ref_level_1_6_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[1].
        channel_ref_suppy_1_6_for_sca_c_roic_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[1].
        vpe_sca_a_video_ref),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[1].
        vpe_sca_b_video_ref),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, circuit.focal_plane_boards[1].
        vpe_sca_c_video_ref),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.cosine_motor_drive_for_mce_current_amps),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.sine_motor_drive_for_mce_current_amps),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.hsib_3_3_current_mon_amps),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.cosine_dac_telemetry_for_mce_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.sine_dac_telemetry_for_mce_volts),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY,
        circuit.elec_enabled_flags),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, reserved_block_2),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, reserved_block_3),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, reserved_block_4),
    HOFFSET( IAS_L0R_TIRS_TELEMETRY, warning_flag)
};

/******************************************************************************
*Star Tracker table data
******************************************************************************/
static const hid_t *field_type_star_tracker_centroid;
static hid_t star_tracker_centroid_types[TABLE_SIZE_STAR_TRACKER_CENTROID];

static const char *field_names_star_tracker_centroid
                                [TABLE_SIZE_STAR_TRACKER_CENTROID] =
{
    "quaternion_index",
    "star_1_valid",
    "star_1_id", /* star ID, star validity and star catalog ID */
    "star_1_position_arcsec_x",
    "star_1_position_arcsec_y",
    "star_1_background_bias", /* 0 - 1023 */
    "star_1_intensity_mi", /* Star intensity_mi in range 1-7.3 */
    "star_2_valid",
    "star_2_id", /* star ID, star validity and star catalog ID */
    "star_2_position_arcsec_x",
    "star_2_position_arcsec_y",
    "star_2_background_bias", /* 0 - 1023 */
    "star_2_intensity_mi", /* Star intensity_mi in range 1-7.3 */
    "star_3_valid",
    "star_3_id", /* star ID, star validity and star catalog ID */
    "star_3_position_arcsec_x",
    "star_3_position_arcsec_y",
    "star_3_background_bias", /* 0 - 1023 */
    "star_3_intensity_mi", /* Star intensity_mi in range 1-7.3 */
    "star_4_valid",
    "star_4_id", /* star ID, star validity and star catalog ID */
    "star_4_position_arcsec_x",
    "star_4_position_arcsec_y",
    "star_4_background_bias", /* 0 - 1023 */
    "star_4_intensity_mi", /* Star intensity_mi in range 1-7.3 */
    "star_5_valid",
    "star_5_id", /* star ID, star validity and star catalog ID */
    "star_5_position_arcsec_x",
    "star_5_position_arcsec_y",
    "star_5_background_bias", /* 0 - 1023 */
    "star_5_intensity_mi", /* Star intensity_mi in range 1-7.3 */
    "star_6_valid",
    "star_6_id", /* star ID, star validity and star catalog ID */
    "star_6_position_arcsec_x",
    "star_6_position_arcsec_y",
    "star_6_background_bias", /* 0 - 1023 */
    "star_6_intensity_mi", /* Star intensity_mi in range 1-7.3 */\
    "effective_focal_length",
    "warning_flag"
};
static const IAS_L0R_STAR_TRACKER_CENTROID* IAS_L0R_STAR_TRACKER_CENTROID_PTR;
static const size_t IAS_L0R_STAR_TRACKER_CENTROID_SIZE =
    sizeof(IAS_L0R_STAR_TRACKER_CENTROID);
static const size_t IAS_L0R_STAR_TRACKER_CENTROID_SIZES
                            [TABLE_SIZE_STAR_TRACKER_CENTROID] =
{
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).quaternion_index),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[0].valid),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[0].id),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[0].position_arcsec.x),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[0].position_arcsec.y),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[0].background_bias),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[0].intensity_mi),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[1].valid),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[1].id),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[1].position_arcsec.x),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[1].position_arcsec.y),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[1].background_bias),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[1].intensity_mi),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[2].valid),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[2].id),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[2].position_arcsec.x),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[2].position_arcsec.y),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[2].background_bias),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[2].intensity_mi),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[3].valid),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[3].id),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[3].position_arcsec.x),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[3].position_arcsec.y),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[3].background_bias),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[3].intensity_mi),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[4].valid),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[4].id),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[4].position_arcsec.x),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[4].position_arcsec.y),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[4].background_bias),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[4].intensity_mi),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[5].valid),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[5].id),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[5].position_arcsec.x),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[5].position_arcsec.y),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[5].background_bias),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).stars[5].intensity_mi),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).effective_focal_length),
    sizeof( (*IAS_L0R_STAR_TRACKER_CENTROID_PTR).warning_flag)
};

static const size_t OFFSET_STAR_TRACKER_CENTROID
    [TABLE_SIZE_STAR_TRACKER_CENTROID] =
{
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,quaternion_index),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[0].valid),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[0].id),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[0].position_arcsec.x),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[0].position_arcsec.y),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[0].background_bias),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[0].intensity_mi),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[1].valid),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[1].id),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[1].position_arcsec.x),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[1].position_arcsec.y),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[1].background_bias),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[1].intensity_mi),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[2].valid),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[2].id),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[2].position_arcsec.x),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[2].position_arcsec.y),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[2].background_bias),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[2].intensity_mi),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[3].valid),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[3].id),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[3].position_arcsec.x),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[3].position_arcsec.y),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[3].background_bias),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[3].intensity_mi),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[4].valid),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[4].id),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[4].position_arcsec.x),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[4].position_arcsec.y),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[4].background_bias),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[4].intensity_mi),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[5].valid),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[5].id),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[5].position_arcsec.x),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[5].position_arcsec.y),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[5].background_bias),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,stars[5].intensity_mi),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,effective_focal_length),
    HOFFSET( IAS_L0R_STAR_TRACKER_CENTROID,warning_flag)
};

/******************************************************************************
*Star Tracker quaternion information
******************************************************************************/
static const hid_t *field_type_star_tracker_quaternion;
static hid_t star_tracker_quaternion_types[TABLE_SIZE_STAR_TRACKER_QUATERNION];

static const char *field_names_star_tracker_quaternion
                            [TABLE_SIZE_STAR_TRACKER_QUATERNION] =
{
    "quaternion_index",
    "l0r_time_days_from_J2000",
    "l0r_time_seconds_of_day",
    "udl_time_sec_original",
    "udl_time_sub_sec_original",
    "sta_time_tag",
    "status_flags_1",
    "status_flags_2",
    "last_processed_command",
    "virtual_tracker_0_state",
    "virtual_tracker_1_state",
    "virtual_tracker_2_state",
    "virtual_tracker_3_state",
    "virtual_tracker_4_state",
    "virtual_tracker_5_state",
    "command_flags",
    "time_message_value",
    "camera_id",
    "sw_version",
    "quaternion_seconds",
    "quaternion_element1", /* double quaternion_element1; */
    "quaternion_element2",
    "quaternion_element3",
    "quaternion_element4",
    "loss_function_value",
    "atm_frame_count",
    "total_sa_writes",
    "total_sa_reads",
    "sa_15_writes",
    "sa_15_reads",
    "sa_26_writes",
    "sa_29_reads",
    "status_flags_3",
    "adm_separation_tolerance_arc_secs",
    "adm_position_tolerance_arc_secs",
    "adm_mag_tolerance",
    "hot_pixel_count",
    "hot_pixel_threshold",
    "track_mode_pixel_threshold",
    "acquisition_mode_pixel_threshold",
    "tec_setpoint",
    "boresight_x",
    "boresight_y",
    "ccd_temperature_celsius",
    "lens_cell_temperature_celsius",
    "reserved",
    "warning_flag"
};


static const IAS_L0R_STAR_TRACKER_QUATERNION*
    IAS_L0R_STAR_TRACKER_QUATERNION_PTR;
static const size_t IAS_L0R_STAR_TRACKER_QUATERNION_SIZE =
                            sizeof(IAS_L0R_STAR_TRACKER_QUATERNION);
static const size_t IAS_L0R_STAR_TRACKER_QUATERNION_SIZES
                            [TABLE_SIZE_STAR_TRACKER_QUATERNION] =
{
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).quaternion_index),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).l0r_time.
                                                        days_from_J2000),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).l0r_time.seconds_of_day),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).udl_time_sec_orig),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).udl_time_subsec_orig),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).sta_time_tag),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).status_flags_1),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).status_flags_2),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).last_processed_command),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).virtual_tracker_states[0]),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).virtual_tracker_states[1]),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).virtual_tracker_states[2]),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).virtual_tracker_states[3]),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).virtual_tracker_states[4]),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).virtual_tracker_states[5]),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).command_flags),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).time_message_value),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).camera_id),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).sw_version),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).
                                        quaternion_seconds),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).quaternion_element_1),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).quaternion_element_2),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).quaternion_element_3),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).quaternion_element_4),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).loss_function_value),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).atm_frame_count),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).total_sa_writes),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).total_sa_reads),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).sa_15_writes),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).sa_15_reads),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).sa_26_writes),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).sa_29_reads),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).status_flags_3),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).
        adm_separation_tolerance_arc_secs),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).
        adm_position_tolerance_arc_secs),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).adm_mag_tolerance),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).hot_pixel_count),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).hot_pixel_threshold),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR)
                                            .track_mode_pixel_threshold),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR)
                                            .acquisition_mode_pixel_threshold),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).tec_setpoint),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).boresight.x),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).boresight.y),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).ccd_temperature_celsius),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).
        lens_cell_temperature_celsius),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).reserved),
    sizeof( (*IAS_L0R_STAR_TRACKER_QUATERNION_PTR).warning_flag)
};

static const size_t OFFSET_STAR_TRACKER_QUATERNION
                        [TABLE_SIZE_STAR_TRACKER_QUATERNION] =
{
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,quaternion_index),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,udl_time_sec_orig),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,udl_time_subsec_orig),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,sta_time_tag),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,status_flags_1),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,status_flags_2),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,last_processed_command),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,virtual_tracker_states[0]),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,virtual_tracker_states[1]),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,virtual_tracker_states[2]),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,virtual_tracker_states[3]),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,virtual_tracker_states[4]),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,virtual_tracker_states[5]),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,command_flags),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,time_message_value),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,camera_id),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,sw_version),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,quaternion_seconds),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,quaternion_element_1),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,quaternion_element_2),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,quaternion_element_3),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,quaternion_element_4),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,loss_function_value),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,atm_frame_count),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,total_sa_writes),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,total_sa_reads),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,sa_15_writes),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,sa_15_reads),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,sa_26_writes),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,sa_29_reads),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,status_flags_3),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,adm_separation_tolerance_arc_secs),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,adm_position_tolerance_arc_secs),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,adm_mag_tolerance),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,hot_pixel_count),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,hot_pixel_threshold),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,track_mode_pixel_threshold),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,acquisition_mode_pixel_threshold),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,tec_setpoint),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,boresight.x),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,boresight.y),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,ccd_temperature_celsius),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,lens_cell_temperature_celsius),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,reserved),
    HOFFSET( IAS_L0R_STAR_TRACKER_QUATERNION,warning_flag)
};

static const hid_t  *field_type_temperatures_gyro = NULL;
static hid_t temperatures_gyro_types[TABLE_SIZE_TEMPERATURES_GYRO];

static const char *field_names_temperatures_gyro[TABLE_SIZE_TEMPERATURES_GYRO] =
{
    "l0r_time_days_from_J2000",
    "l0r_time_seconds_of_day",
    "gyro_a_filtered_resonator",
    "gyro_a_filtered_derivative_of_resonator",
    "gyro_a_filtered_electronics",
    "gyro_a_filtered_derivative_of_electronics",
    "gyro_a_filtered_diode",
    "gyro_a_filtered_derivative_of_diode",
    "gyro_a_filtered_case",
    "gyro_a_filtered_derivative_of_case",
    "gyro_b_filtered_resonator",
    "gyro_b_filtered_derivative_of_resonator",
    "gyro_b_filtered_electronics",
    "gyro_b_filtered_derivative_of_electronics",
    "gyro_b_filtered_diode",
    "gyro_b_filtered_derivative_of_diode",
    "gyro_b_filtered_case",
    "gyro_b_filtered_derivative_of_case",
    "gyro_c_filtered_resonator",
    "gyro_c_filtered_derivative_of_resonator",
    "gyro_c_filtered_electronics",
    "gyro_c_filtered_derivative_of_electronics",
    "gyro_c_filtered_diode",
    "gyro_c_filtered_derivative_of_diode",
    "gyro_c_filtered_case",
    "gyro_c_filtered_derivative_of_case",
    "gyro_d_filtered_resonator",
    "gyro_d_filtered_derivative_of_resonator",
    "gyro_d_filtered_electronics",
    "gyro_d_filtered_derivative_of_electronics",
    "gyro_d_filtered_diode",
    "gyro_d_filtered_derivative_of_diode",
    "gyro_d_filtered_case",
    "gyro_d_filtered_derivative_of_case",
    "reserved",
    "warning_flag"
};

static const IAS_L0R_GYRO_TEMPERATURE *IAS_L0R_GYRO_TEMPERATURE_PTR = NULL;
static const size_t IAS_L0R_GYRO_TEMPERATURE_SIZE =
    sizeof(IAS_L0R_GYRO_TEMPERATURE);
static const size_t IAS_L0R_GYRO_TEMPERATURE_SIZES
    [TABLE_SIZE_TEMPERATURES_GYRO] =
{
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        
        l0r_time.
        days_from_J2000),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        l0r_time.seconds_of_day),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_A_FILTERED_RESONATOR]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_A_FILTERED_DERIVATIVE_OF_RESONATOR]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_A_FILTERED_ELECTRONICS]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_A_FILTERED_DERIVATIVE_OF_ELECTRONICS]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_A_FILTERED_DIODE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_A_FILTERED_DERIVATIVE_OF_DIODE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_A_FILTERED_CASE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_A_FILTERED_DERIVATIVE_OF_CASE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_B_FILTERED_RESONATOR]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_B_FILTERED_DERIVATIVE_OF_RESONATOR]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_B_FILTERED_ELECTRONICS]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_B_FILTERED_DERIVATIVE_OF_ELECTRONICS]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_B_FILTERED_DIODE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_B_FILTERED_DERIVATIVE_OF_DIODE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_B_FILTERED_CASE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_B_FILTERED_DERIVATIVE_OF_CASE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_C_FILTERED_RESONATOR]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_C_FILTERED_DERIVATIVE_OF_RESONATOR]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_C_FILTERED_ELECTRONICS]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_C_FILTERED_DERIVATIVE_OF_ELECTRONICS]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_C_FILTERED_DIODE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_C_FILTERED_DERIVATIVE_OF_DIODE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_C_FILTERED_CASE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_C_FILTERED_DERIVATIVE_OF_CASE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_D_FILTERED_RESONATOR]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_D_FILTERED_DERIVATIVE_OF_RESONATOR]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_D_FILTERED_ELECTRONICS]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_D_FILTERED_DERIVATIVE_OF_ELECTRONICS]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_D_FILTERED_DIODE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_D_FILTERED_DERIVATIVE_OF_DIODE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_D_FILTERED_CASE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).
        temperatures_celsius[GYRO_D_FILTERED_DERIVATIVE_OF_CASE]),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).reserved),
    sizeof( (*IAS_L0R_GYRO_TEMPERATURE_PTR).warning_flag)
};

static const size_t OFFSET_GYRO_TEMPERATURE[TABLE_SIZE_TEMPERATURES_GYRO] =
{
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_A_FILTERED_RESONATOR]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_A_FILTERED_DERIVATIVE_OF_RESONATOR]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_A_FILTERED_ELECTRONICS]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_A_FILTERED_DERIVATIVE_OF_ELECTRONICS]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_A_FILTERED_DIODE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_A_FILTERED_DERIVATIVE_OF_DIODE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_A_FILTERED_CASE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_A_FILTERED_DERIVATIVE_OF_CASE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_B_FILTERED_RESONATOR]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_B_FILTERED_DERIVATIVE_OF_RESONATOR]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_B_FILTERED_ELECTRONICS]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_B_FILTERED_DERIVATIVE_OF_ELECTRONICS]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_B_FILTERED_DIODE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_B_FILTERED_DERIVATIVE_OF_DIODE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_B_FILTERED_CASE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_B_FILTERED_DERIVATIVE_OF_CASE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_C_FILTERED_RESONATOR]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_C_FILTERED_DERIVATIVE_OF_RESONATOR]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_C_FILTERED_ELECTRONICS]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_C_FILTERED_DERIVATIVE_OF_ELECTRONICS]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_C_FILTERED_DIODE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_C_FILTERED_DERIVATIVE_OF_DIODE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_C_FILTERED_CASE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_C_FILTERED_DERIVATIVE_OF_CASE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_D_FILTERED_RESONATOR]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_D_FILTERED_DERIVATIVE_OF_RESONATOR]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_D_FILTERED_ELECTRONICS]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_D_FILTERED_DERIVATIVE_OF_ELECTRONICS]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_D_FILTERED_DIODE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_D_FILTERED_DERIVATIVE_OF_DIODE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_D_FILTERED_CASE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,
        temperatures_celsius[GYRO_D_FILTERED_DERIVATIVE_OF_CASE]),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE, reserved),
    HOFFSET( IAS_L0R_GYRO_TEMPERATURE,warning_flag)
};

static const hid_t *field_type_temperatures_oli_tirs;
static hid_t temperatures_oli_tirs_types[TABLE_SIZE_TEMPERATURES_OLI_TIRS];
static const char *field_names_temperatures_oli_tirs
    [TABLE_SIZE_TEMPERATURES_OLI_TIRS] =
{
    "l0r_time_days_from_J2000",
    "l0r_time_seconds_of_day",
    "oli_primary_mirror_flexure",
    "oli_telescope_positive_z_negative_y_strut_tube",
    "oli_fpe_heat_pipe_evaporator",
    "oli_baseplate_positive_z",
    "oli_baseplate_negative_z",
    "oli_primary_mirror_bench_at_flex",
    "oli_secondary_mirror_center",
    "oli_secondary_mirror_edge",
    "oli_secondary_mirror_flexure",
    "oli_secondary_mirror_bench_at_flex",
    "oli_tertiary_mirror_center",
    "oli_tertiary_mirror_edge",
    "oli_tertiary_mirror_flexure",
    "oli_tertiary_mirror_bench_at_flex",
    "oli_quat_mirror_center",
    "oli_quat_mirror_edge",
    "oli_fpa_1_radiator",
    "oli_quat_mirror_flexure",
    "oli_fpa_2_heat_pipe_evaporator",
    "oli_fpa_3_heat_pipe_condenser",
    "oli_fpa_4_moly_bp_primary",
    "oli_fpa_5_moly_bp_redundant",
    "oli_fpa_6_sink",
    "oli_fpa_7_cold_cable_radiator",
    "oli_fpa_8_mli_negative_y_bench_tedlar",
    "oli_fpa_9_foot_at_spacecraft_interface",
    "oli_fpa_10_condenser",
    "tirs_tb1_ch49_bank4_01",
    "tirs_tb1_ch50_bank4_02",
    "oli_fpe_radiator",
    "tirs_tb1_ch51_bank4_03",
    "oli_fpe_heat_ptpt_condenser",
    "tirs_tb1_ch52_bank4_04",
    "oli_fpe_chassis_primary",
    "oli_baseplate_positive_y",
    "oli_fpe_chassis_redundant",
    "oli_ise_chassis_primary",
    "oli_ise_chassis_redundant",
    "oli_ise_radiator",
    "oli_quat_mirror_bench_at_flex",
    "oli_bench_positive_y_1",
    "oli_bench_positive_y_2",
    "oli_bench_positive_y_3",
    "oli_bench_negative_y_1",
    "oli_bench_negative_y_2",
    "oli_bench_negative_x",
    "oli_bench_positive_x_1",
    "oli_bench_positive_x_2",
    "oli_cal_assembly_diffuser_cover",
    "oli_negative_x_focus_mechanism",
    "oli_stimulation_lamp_1_diode_board",
    "oli_tb1_ch72_bank5_8",
    "oli_tb1_ch73_bank5_9",
    "oli_tb1_ch74_bank5_10",
    "oli_stimulation_lamp_2_diode_board",
    "oli_bench_negative_x_panel",
    "oli_diffuser_wheel_motor",
    "oli_shutter_wheel_motor",
    "tirs_tb1_ch87_bank6_7",
    "tirs_tb1_ch88_bank6_8",
    "tirs_tb1_ch89_bank6_9",
    "oli_baseplate_negative_y",
    "tirs_tb1_ch90_bank6_10",
    "oli_primary_mirror_center",
    "tirs_tb1_ch91_bank6_11",
    "oli_primary_mirror_edge",
    "tirs_tb1_ch92_bank6_12",
    "warning_flag"
};

static const IAS_L0R_OLI_TIRS_TEMPERATURE* IAS_L0R_OLI_TIRS_TEMPERATURE_PTR;
static const size_t IAS_L0R_OLI_TIRS_TEMPERATURE_SIZE =
    sizeof(IAS_L0R_OLI_TIRS_TEMPERATURE);
static const size_t IAS_L0R_OLI_TIRS_TEMPERATURE_SIZES
    [TABLE_SIZE_TEMPERATURES_OLI_TIRS] =
{
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
                 l0r_time.days_from_J2000),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
                 l0r_time.seconds_of_day),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_PRIMARY_MIRROR_FLEXURE]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_TELESCOPE_POSITIVE_Z_NEGATIVE_Y_STRUT_TUBE]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPE_HEAT_PIPE_EVAPORATIOR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BASEPLATE_POSITIVE_Z]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BASEPLATE_NEGATIVE_Z]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_PRIMARY_MIRROR_BENCH_AT_FLEX]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_SECONDARY_MIRROR_CENTER]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_SECONDARY_MIRROR_EDGE]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_SECONDARY_MIRROR_FLEXURE]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_SECONDARY_MIRROR_BENCH_AT_FLEX]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_TERTIARY_MIRROR_CENTER]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_TERTIARY_MIRROR_EDGE]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_TERTIARY_MIRROR_FLEXURE]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_TERTIARY_MIRROR_BENCH_AT_FLEX]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_QUAT_MIRROR_CENTER]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_QUAT_MIRROR_EDGE]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPA_1_RADIATOR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_QUAT_MIRROR_FLEXURE]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPA_2_HEAT_PIPE_EVAPORATOR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPA_3_HEAT_PIPE_CONDENSOR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPA_4_MOLY_BP_PRIMARY]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPA_5_MOLY_BP_REDUNDANT]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPA_6_SINK]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPA_7_COLD_CABLE_RADIATOR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPA_8_MLI_NEGATIVE_Y_BENCH_TEDLAR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPA_9_FOOT_AT_SPACECRAFT_INTERFACE]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPA_10_CONDENSOR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[TIRS_TB1_CH49_BANK4_01]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[TIRS_TB1_CH50_BANK4_02]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPE_RADIATOR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[TIRS_TB1_CH51_BANK4_03]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPE_HEAT_PTPT_CONDENSOR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[TIRS_TB1_CH52_BANK4_04]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPE_CHASSIS_PRIMARY]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BASEPLATE_POSITIVE_Y]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_FPE_CHASSIS_REDUNDANT]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_ISE_CHASSIS_PRIMARY]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_ISE_CHASSIS_REDUNDANT]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_ISE_RADIATOR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_QUAT_MIRROR_BENCH_AT_FLEX]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BENCH_POSITIVE_Y_1]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BENCH_POSITIVE_Y_2]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BENCH_POSITIVE_Y_3]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BENCH_NEGATIVE_Y_1]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BENCH_NEGATIVE_Y_2]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BENCH_NEGATIVE_X]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BENCH_POSITIVE_X_1]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BENCH_POSITIVE_X_2]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_CAL_ASSEMBLY_DIFFUSER_COVER]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_NEGATIVE_X_FOCUS_MECHANISM]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_STIMULATION_LAMP_1_DIODE_BOARD]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_TB1_CH72_BANK5_8]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_TB1_CH73_BANK5_9]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_TB1_CH74_BANK5_10]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_STIMULATION_LAMP_2_DIODE_BOARD]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BENCH_NEGATIVE_X_PANEL]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_DIFFUSER_WHEEL_MOTOR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_SHUTTER_WHEEL_MOTOR]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[TIRS_TB1_CH87_BANK6_7]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[TIRS_TB1_CH88_BANK6_8]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[TIRS_TB1_CH89_BANK6_9]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_BASEPLATE_NEGATIVE_Y]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[TIRS_TB1_CH90_BANK6_10]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_PRIMARY_MIRROR_CENTER]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[TIRS_TB1_CH91_BANK6_11]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[OLI_PRIMARY_MIRROR_EDGE]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).
        temperatures_celsius[TIRS_TB1_CH92_BANK6_12]),
    sizeof( (*IAS_L0R_OLI_TIRS_TEMPERATURE_PTR).warning_flag)
};

static const size_t OFFSET_OLI_TIRS_TEMPERATURE
    [TABLE_SIZE_TEMPERATURES_OLI_TIRS] =
{
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
                 l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
                 l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_PRIMARY_MIRROR_FLEXURE]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_TELESCOPE_POSITIVE_Z_NEGATIVE_Y_STRUT_TUBE]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPE_HEAT_PIPE_EVAPORATIOR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BASEPLATE_POSITIVE_Z]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BASEPLATE_NEGATIVE_Z]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_PRIMARY_MIRROR_BENCH_AT_FLEX]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_SECONDARY_MIRROR_CENTER]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_SECONDARY_MIRROR_EDGE]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_SECONDARY_MIRROR_FLEXURE]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_SECONDARY_MIRROR_BENCH_AT_FLEX]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_TERTIARY_MIRROR_CENTER]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_TERTIARY_MIRROR_EDGE]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_TERTIARY_MIRROR_FLEXURE]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_TERTIARY_MIRROR_BENCH_AT_FLEX]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_QUAT_MIRROR_CENTER]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_QUAT_MIRROR_EDGE]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPA_1_RADIATOR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_QUAT_MIRROR_FLEXURE]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPA_2_HEAT_PIPE_EVAPORATOR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPA_3_HEAT_PIPE_CONDENSOR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPA_4_MOLY_BP_PRIMARY]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPA_5_MOLY_BP_REDUNDANT]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPA_6_SINK]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPA_7_COLD_CABLE_RADIATOR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPA_8_MLI_NEGATIVE_Y_BENCH_TEDLAR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPA_9_FOOT_AT_SPACECRAFT_INTERFACE]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPA_10_CONDENSOR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[TIRS_TB1_CH49_BANK4_01]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[TIRS_TB1_CH50_BANK4_02]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPE_RADIATOR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[TIRS_TB1_CH51_BANK4_03]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPE_HEAT_PTPT_CONDENSOR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[TIRS_TB1_CH52_BANK4_04]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPE_CHASSIS_PRIMARY]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BASEPLATE_POSITIVE_Y]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_FPE_CHASSIS_REDUNDANT]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_ISE_CHASSIS_PRIMARY]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_ISE_CHASSIS_REDUNDANT]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_ISE_RADIATOR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_QUAT_MIRROR_BENCH_AT_FLEX]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BENCH_POSITIVE_Y_1]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BENCH_POSITIVE_Y_2]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BENCH_POSITIVE_Y_3]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BENCH_NEGATIVE_Y_1]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BENCH_NEGATIVE_Y_2]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BENCH_NEGATIVE_X]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BENCH_POSITIVE_X_1]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BENCH_POSITIVE_X_2]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_CAL_ASSEMBLY_DIFFUSER_COVER]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_NEGATIVE_X_FOCUS_MECHANISM]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_STIMULATION_LAMP_1_DIODE_BOARD]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_TB1_CH72_BANK5_8]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_TB1_CH73_BANK5_9]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_TB1_CH74_BANK5_10]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_STIMULATION_LAMP_2_DIODE_BOARD]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BENCH_NEGATIVE_X_PANEL]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_DIFFUSER_WHEEL_MOTOR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_SHUTTER_WHEEL_MOTOR]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[TIRS_TB1_CH87_BANK6_7]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[TIRS_TB1_CH88_BANK6_8]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[TIRS_TB1_CH89_BANK6_9]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_BASEPLATE_NEGATIVE_Y]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[TIRS_TB1_CH90_BANK6_10]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_PRIMARY_MIRROR_CENTER]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[TIRS_TB1_CH91_BANK6_11]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[OLI_PRIMARY_MIRROR_EDGE]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,
        temperatures_celsius[TIRS_TB1_CH92_BANK6_12]),
    HOFFSET( IAS_L0R_OLI_TIRS_TEMPERATURE,warning_flag)
};

/* subroutine used in writing ancillary data */
static int ias_l0r_append_ancillary_records
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int count, /* I: number of items */
    const void *buffer, /* I: data to write */
    const char *table_name, /* I: table to write to */
    const size_t size, /* I: number or records to write */
    const size_t *offset, /* I: a list of the offsets for the fields */
    const size_t *size_list /* I: a list of the sizes for the fields */
)
{
    herr_t status;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr =  l0r;

    /* write the records to the table */
    status = H5TBappend_records(hdfio_ptr->file_id_ancillary, table_name,
        count, size, offset, size_list, buffer);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error appending to table %s", table_name);
        return ERROR;
    }

    return SUCCESS;
}

int ias_l0r_establish_ancillary_file
(
    HDFIO *hdfio_ptr, /* I: Structure used in I/O */
    int create_if_absent /* I: Create file if it is not present */
)
{
    char ancillary_filename[IAS_L0R_FILE_NAME_LENGTH];
    char *ancillary_appendix = "_ANC.h5\0";

    int filename_length = 0;

    if (hdfio_ptr->access_mode_ancillary == -1)
    {
        IAS_LOG_ERROR("Ancillary file is not open");
        return ERROR;
    }

    if (hdfio_ptr->file_id_ancillary > 0)
    {
        return SUCCESS;
    }

    /*If the file is already in use for the header data, use that ID */
    if (hdfio_ptr->file_id_header > 0)
    {
        hdfio_ptr->file_id_ancillary = hdfio_ptr->file_id_header;
        return SUCCESS;
    }

    /*the length of the filename with 1 additional character for the
    *NULL terminator*/
    filename_length = strlen(hdfio_ptr->l0r_name_prefix) +
        strlen(ancillary_appendix) + 1;
    if ( filename_length > IAS_L0R_FILE_NAME_LENGTH)
    {
        IAS_LOG_ERROR(
            "The filename is too long:"
            " %s%s is %d characters vs the max of %d",
            hdfio_ptr->l0r_name_prefix, ancillary_appendix, filename_length,
            IAS_L0R_FILE_NAME_LENGTH);
        return ERROR;
    }

    snprintf(ancillary_filename, IAS_L0R_FILE_NAME_LENGTH, "%s%s",
        hdfio_ptr->l0r_name_prefix,
        ancillary_appendix);

    return ias_l0r_hdf_establish_file(hdfio_ptr->path, ancillary_filename,
        &hdfio_ptr->file_id_ancillary, hdfio_ptr->access_mode_ancillary,
        create_if_absent);
}

static int ias_l0r_anc_write
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const void *buffer, /* I: Data to write */
    const char *table_name, /* I: table name to write to */
    const size_t size, /* I: Number of records to write */
    const size_t *offset, /* I: Offsets for the field types */
    const size_t *size_list /* I: Field type sizes */
)
{
    herr_t status;
    hid_t dataset_id;
    hid_t dataspace_id;
    hsize_t dims;
    hsize_t maxdims;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr =  l0r;

    if (hdfio_ptr->access_mode_ancillary != IAS_WRITE)
    {
        IAS_LOG_ERROR("Ancillary data is not in write mode");
        return ERROR;
    }

    dataset_id = H5Dopen(hdfio_ptr->file_id_ancillary, table_name,
       H5P_DEFAULT);
    if (dataset_id < 0)
    {
        IAS_LOG_ERROR("Error getting dataset id for %s", table_name);
        return ERROR;
    }
    dataspace_id = H5Dget_space(dataset_id);
    if (dataspace_id < 0)
    {
        IAS_LOG_ERROR("Error getting dataspace id for %s", table_name);
        H5Dclose(dataset_id);
        return ERROR;
    }
    status = H5Sget_simple_extent_dims(dataspace_id, &dims, &maxdims );
    if (status < 0)
    {
        IAS_LOG_ERROR("Error getting extent dimensions");
        return ERROR;
    }

    if ( (index + count) > dims)
    {
        dims = index+ count;
        status = H5Dset_extent(dataset_id, &dims);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error setting extent");
            H5Dclose(dataset_id);
            H5Sclose(dataspace_id);
            return ERROR;
        }
    }
    status = H5Dclose(dataset_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error closing dataset");
    }
    status = H5Sclose(dataspace_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error closing group");
    }
    /* write the records to the table */
    status = H5TBwrite_records(hdfio_ptr->file_id_ancillary, table_name,
        index, count, size, offset, size_list, buffer);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error writing to %s %d records at index %d", table_name,
            count, index);
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_attitude_field_type

 PURPOSE: populates the attitude types that
          are unable to be assigned while compiling

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_attitude_field_type(void)
{
    int i = 0;
    attitude_types[i++] = H5T_STD_I32LE; /* int32_t days_from_J2000 */
    attitude_types[i++] = H5T_IEEE_F64LE; /* seconds of the day */
    attitude_types[i++] = H5T_IEEE_F64LE; /* time_tag_sec_orig */
    attitude_types[i++] = H5T_IEEE_F64LE; /*inertial to body quat*/
    attitude_types[i++] = H5T_IEEE_F64LE;
    attitude_types[i++] = H5T_IEEE_F64LE;
    attitude_types[i++] = H5T_IEEE_F64LE; /* quaternion scalar */
    attitude_types[i++] = H5T_STD_U8LE; /* warning_flag */
    if (i != TABLE_SIZE_ATTITUDE)
    {
        IAS_LOG_ERROR("field_type_attitude does not contain the correct "
            "number of types for the ATTITUDE table. %d vs %d", i,
            TABLE_SIZE_ATTITUDE);
        return NULL;
    }

    return attitude_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_attitude_filter_states_field_type

 PURPOSE: initializes the types used with the attitude filter
          table in LOR files

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_attitude_filter_states_field_type(void)
{
    int i = 0;
    attitude_filter_types[i++] = H5T_STD_I32LE;/* days from J2000 */
    attitude_filter_types[i++] = H5T_IEEE_F64LE;/* seconds of the day */
    attitude_filter_types[i++] = H5T_STD_I32LE; /* time_tag_sec_orig */
    attitude_filter_types[i++] = H5T_STD_I32LE; 
        /* time_tag_subseconds_orig */
    attitude_filter_types[i++] = H5T_IEEE_F64LE; /* Gyro combined bias x*/
    attitude_filter_types[i++] = H5T_IEEE_F64LE; /* Gyro combined bias y*/
    attitude_filter_types[i++] = H5T_IEEE_F64LE; /* Gyro combined bias z*/
    attitude_filter_types[i++] = H5T_IEEE_F64LE; /* Gyros scale factor*/
    attitude_filter_types[i++] = H5T_IEEE_F64LE;
    attitude_filter_types[i++] = H5T_IEEE_F64LE;
    attitude_filter_types[i++] =
                H5T_IEEE_F32LE; /* Gyro X axis misalignment along Y */
    attitude_filter_types[i++] =
                H5T_IEEE_F32LE; /* Gyro X axis misalignment along Z */
    attitude_filter_types[i++] =
                H5T_IEEE_F32LE; /* Gyro Y axis misalignment along X */
    attitude_filter_types[i++] =
                H5T_IEEE_F32LE; /* Gyro Y axis misalignment along Z */
    attitude_filter_types[i++] =
                H5T_IEEE_F32LE; /* Gyro Z axis misalignment along X */
    attitude_filter_types[i++] =
                H5T_IEEE_F32LE; /* Gyro Z axis misalignment along Y */
    attitude_filter_types[i++] =
                H5T_IEEE_F64LE;/* kalman filter attitude error */
    attitude_filter_types[i++] = H5T_IEEE_F64LE;
    attitude_filter_types[i++] = H5T_IEEE_F64LE;
    attitude_filter_types[i++] = H5T_IEEE_F64LE;/* covariance_diagonal */
    attitude_filter_types[i++] = H5T_IEEE_F64LE;
    attitude_filter_types[i++] = H5T_IEEE_F64LE;
    attitude_filter_types[i++] = H5T_STD_U8LE; /* warning_flag */
    if (i != TABLE_SIZE_ATTITUDE_FILTER)
    {
        IAS_LOG_ERROR("field_type_attitude_filter does not contain the correct "
            "number of types for the ATTITUDE_FILTER table. %d vs %d",
            i, TABLE_SIZE_ATTITUDE_FILTER);
        return NULL;
    }

    return attitude_filter_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_ephem_field_type

 PURPOSE: populates the field_type_ephemeris array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_ephem_field_type(void)
{
    int i = 0;
    ephemeris_types[i++] = H5T_STD_I32LE; /* days from j2000 */
    ephemeris_types[i++] = H5T_IEEE_F64LE; /* seconds of the day */
    ephemeris_types[i++] = H5T_IEEE_F64LE; /* time_tag_sec_orig */
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_IEEE_F64LE;
    ephemeris_types[i++] = H5T_STD_U8LE; /* warning_flag */
    if (i != TABLE_SIZE_EPHEMERIS)
    {
        IAS_LOG_ERROR("field_type_ephemeris does not contain the correct "
            "number of types for the EPHEMERIS table. %d vs %d",
            i, TABLE_SIZE_EPHEMERIS);
        return NULL;

    }

    return ephemeris_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_gps_field_type

 PURPOSE: populates the field_type_gps_position array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_gps_field_type(void)
{
    int i = 0;

    gps_position_types[i++] = H5T_STD_I32LE; /* int32_t days_from_J2000 */
    gps_position_types[i++] = H5T_IEEE_F64LE; /* seconds_of_day */
    gps_position_types[i++] = H5T_STD_U8LE; /* uint8_t month */
    gps_position_types[i++] = H5T_STD_U8LE; /* uint8_t day */
    gps_position_types[i++] = H5T_STD_U16LE; /* uint16_t year */
    gps_position_types[i++] = H5T_STD_U8LE; /* uint8_t hours */
    gps_position_types[i++] = H5T_STD_U8LE; /* uint8_t minutes */
    gps_position_types[i++] = H5T_STD_U8LE;; /* uint8_t seconds */
    gps_position_types[i++] = H5T_STD_U32LE; /* uint32_t nanoseconds*/
    gps_position_types[i++] = H5T_STD_U8LE; /* uint8_t function */
    gps_position_types[i++] = H5T_STD_U8LE; /* uint8_t subfunction */
    gps_position_types[i++] = H5T_IEEE_F64LE; /* point_arc_secs.lat */
    gps_position_types[i++] = H5T_IEEE_F64LE; /* point_arc_secs.lon */
    gps_position_types[i++] = H5T_IEEE_F64LE; /* height_uncorrected_meters*/
    gps_position_types[i++] = H5T_IEEE_F64LE;
    /* double height_corrected_meters*/
    gps_position_types[i++] = H5T_IEEE_F64LE;
    /*double velocity_meters_per_sec */
    gps_position_types[i++] = H5T_IEEE_F64LE; /* double heading_degrees */
    gps_position_types[i++] = H5T_IEEE_F32LE; /* float current dop */
    gps_position_types[i++] = H5T_STD_U8LE; /* uint8_t dop_type */
    gps_position_types[i++] = H5T_STD_U8LE; /* uint8_t num_visible_satellites */
    gps_position_types[i++] = H5T_STD_U8LE; /* uint8_t num_satellites_tracked */
    gps_position_types[i++] = H5T_STD_U8LE; /* tracked sat 1 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE; /* tracked sat 2 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE; /* tracked sat 3 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE; /* tracked sat 4 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE; /* tracked sat 5 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE; /* tracked sat 6 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE; /* tracked sat 7 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE; /* tracked sat 8 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE; /* tracked sat 9 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE;  /* tracked sat 10 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE;  /* tracked sat 11 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE; /* tracked sat 12 */
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE;
    gps_position_types[i++] = H5T_STD_U8LE; /* channel status flags */
    gps_position_types[i++] = H5T_STD_U8LE; /* receiver status flags */
    gps_position_types[i++] = H5T_IEEE_F64LE;  /* ecef_x_pos */
    gps_position_types[i++] = H5T_IEEE_F64LE;  /* ecef_y_pos */
    gps_position_types[i++] = H5T_IEEE_F64LE;  /* ecef_z_pos */
    gps_position_types[i++] = H5T_IEEE_F64LE;  /* ecef_x_vel */
    gps_position_types[i++] = H5T_IEEE_F64LE;  /* ecef_y_vel */
    gps_position_types[i++] = H5T_IEEE_F64LE;  /* ecef_z_vel */
    gps_position_types[i++] = H5T_STD_U8LE; /* warning_flag */

    if (i != TABLE_SIZE_GPS_POSITION)
    {
        IAS_LOG_ERROR("The number of size entries for the GPS_range table is"
            " not correct");
        return NULL;
    }

    return gps_position_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_gps_range_field_type

 PURPOSE: populates the field_type_gps_range array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_gps_range_field_type(void)
{
    int i = 0;
    int j = 0;
    gps_range_types[i++] = H5T_STD_I32LE; /* int32_t days_from_J2000 */
    gps_range_types[i++] = H5T_IEEE_F64LE; /* seconds_of_day */
    gps_range_types[i++] = H5T_STD_I32LE; /* seconds */
    gps_range_types[i++] = H5T_STD_I32LE; /* nanoseconds */
    gps_range_types[i++] = H5T_STD_U8LE; /* function */
    gps_range_types[i++] = H5T_STD_U8LE; /* sub_function */
   
    for(j=0; j < IAS_GPS_NUM_SATELLITES; j++)
    {
         gps_range_types[i++] = H5T_STD_U8LE; /* id */
         gps_range_types[i++] = H5T_STD_U8LE; /* tracking_mode */
         gps_range_types[i++] = H5T_STD_I32LE; /* gps_time_seconds */
         gps_range_types[i++] = H5T_STD_I32LE; /* gps_time_nanoseconds */
         gps_range_types[i++] = H5T_STD_I32LE; /* carrier cycles */
         gps_range_types[i++] = H5T_STD_U32LE;
         /* integrated_carrier_phase_cycles */
         gps_range_types[i++] = H5T_IEEE_F64LE;
         /* integrated_carrier_phase_deg */
         gps_range_types[i++] = H5T_IEEE_F64LE;
         /* code_discriminator_output */
    }
    gps_range_types[i++] = H5T_STD_U8LE; /* warning_flag */

    if (i != TABLE_SIZE_GPS_RANGE)
    {
        IAS_LOG_ERROR("The number of size entries for the GPS_range table is"
            " not correct");
        return NULL;
    }

    return gps_range_types;
};

/******************************************************************************
 NAME: ias_l0r_init_st_centroid_field_type

 PURPOSE: populates the field_type_star_tracker_centroid array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_st_centroid_field_type(void)
{
    int i = 0;
    int j = 0;
    /* Types found in the Star Tracker header */
    star_tracker_centroid_types[i++] =
                H5T_STD_U16LE; /* uint16_t index of the associated 
                                  quaternion record */
    /* Types for each star */
    for (j = 0; j<IAS_STAR_TRACKER_NUM_STARS; j++)
    {
        star_tracker_centroid_types[i++] = H5T_STD_U8LE; /*uint8_t valid;*/
        star_tracker_centroid_types[i++] = H5T_STD_U16LE; /*uint16_t id*/
        star_tracker_centroid_types[i++] = H5T_IEEE_F64LE; /*pos_arcsec_x*/
        star_tracker_centroid_types[i++] = H5T_IEEE_F64LE; /*pos_arcsec_y*/
        star_tracker_centroid_types[i++] = H5T_STD_U16LE; /*uint8_t bg_bias */
        star_tracker_centroid_types[i++] = H5T_IEEE_F32LE; /*float mag */
    }
    star_tracker_centroid_types[i++] = H5T_STD_U16LE;
    /*uint16_t effective_focal_length*/
    star_tracker_centroid_types[i++] = H5T_STD_U8LE; /*uint8_t warning_flag*/

    if (i != TABLE_SIZE_STAR_TRACKER_CENTROID)
    {
        IAS_LOG_ERROR("The number of defined "
            "types does not match the size of the"
            " table for the Star Tracker Centroid, %d vs %d", i,
            TABLE_SIZE_STAR_TRACKER_CENTROID);
        return NULL;
    }
    
    return star_tracker_centroid_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_st_quaternion_field_type

 PURPOSE: populates the field_type_star_tracker_quaternion
          array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_st_quaternion_field_type(void)
{
    int i = 0;

    /* Array of reserved fields */
    hid_t array_id_reserved;
    hsize_t dims_reserved[1] = {IAS_L0R_STAR_TRACKER_QUATERNION_RESERVED};

    array_id_reserved = H5Tarray_create( H5T_STD_U8LE, 1, dims_reserved);
    if (array_id_reserved < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }

    /* Types found in the Star Tracker header */
    /* quaternion index - index of quaternion record */
    star_tracker_quaternion_types[i++] = H5T_STD_U16LE;
    star_tracker_quaternion_types[i++] =
        H5T_STD_I32LE; /* int32_t days_from_J2000 */
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F64LE; /* double seconds from J2000*/
    star_tracker_quaternion_types[i++] =
        H5T_STD_I32LE; /*uint32_t udl_time_sec*/
    star_tracker_quaternion_types[i++] =
        H5T_STD_I32LE; /*uint32_t udl_time_subsec*/
    star_tracker_quaternion_types[i++] = H5T_STD_I32LE;
    /*uint32_t STA time tag */
    star_tracker_quaternion_types[i++] = H5T_STD_U8LE;
    /*uint8_t status_flag_1*/
    star_tracker_quaternion_types[i++] = H5T_STD_U8LE;
    /*uint8_t status_flag_2*/
    star_tracker_quaternion_types[i++] = H5T_STD_U8LE;
    /*uint8_t last_processed_command*/
    star_tracker_quaternion_types[i++] =
                H5T_STD_U8LE; /*virtual_trackers_states[0]*/
    star_tracker_quaternion_types[i++] =
                H5T_STD_U8LE; /*virtual_trackers_states[1]*/
    star_tracker_quaternion_types[i++] =
                H5T_STD_U8LE; /*virtual_trackers_states[2]*/
    star_tracker_quaternion_types[i++] =
                H5T_STD_U8LE; /*virtual_trackers_state[3]*/
    star_tracker_quaternion_types[i++] =
                H5T_STD_U8LE; /*virtual_trackers_state[4]*/
    star_tracker_quaternion_types[i++] =
                H5T_STD_U8LE; /*virtual_trackers_state[5]*/
    star_tracker_quaternion_types[i++] = H5T_STD_U8LE;/*uint8_t command_flags*/
    star_tracker_quaternion_types[i++] = H5T_STD_U8LE;
    /*uint8_t time_message_value*/
    star_tracker_quaternion_types[i++] = H5T_STD_U8LE; /*uint8_t camera*/
    star_tracker_quaternion_types[i++] = H5T_STD_U8LE; /*uint8_t version*/
    /*Quaternion body */
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F64LE;  /*quaternion seconds*/
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F64LE;  /*quaternion element 1*/
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F64LE;  /*quaternion element 2*/
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F64LE;  /*quaternion element 3*/
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F64LE;  /*quaternion element 4*/
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F64LE; /*loss_function_value;*/
    star_tracker_quaternion_types[i++] =
        H5T_STD_U16LE; /* uint16_t atm_frame_count */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t total_sa_writes */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t total_sa_reads */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t sa_15_writes */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t sa_15_reads */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t sa_26_writes */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t sa_29_reads */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t status_flags_3 */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t adm_separation_tolerance_arc_secs */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t adm_position_tolerance_arc_secs */
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F32LE; /* uint8_t adm_mag_tolerance */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t hot_pixel_count */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t hot_pixel_threshold */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t track_mode_pixel_threshold */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /* uint8_t acquisition_mode_pixel_threshold */
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F64LE; /* double tec_setpoint */
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F64LE; /* int32_t boresight.x */
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F64LE; /* int32_t boresight.y */
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F32LE; /*float ccd_temperature_celsius;*/
    star_tracker_quaternion_types[i++] =
        H5T_IEEE_F32LE; /*lens_cell_temperature_celsius;*/
    star_tracker_quaternion_types[i++] =
        array_id_reserved; /* reserved bytes */
    star_tracker_quaternion_types[i++] =
        H5T_STD_U8LE; /*uint8_t warning_flag*/
    if (i != TABLE_SIZE_STAR_TRACKER_QUATERNION)
    {
        IAS_LOG_ERROR("The number of defined types"
            " does not match the size of the "
            "table for the Star Tracker Quaternion, %d vs %d", i,
            TABLE_SIZE_STAR_TRACKER_QUATERNION);
        return NULL;
    }

    return star_tracker_quaternion_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_imu_field_type

 PURPOSE: populates the field_type_imu array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_imu_field_type(HDFIO* hdfio_ptr)
{
    unsigned int i = 0;
    int j = 0;
    herr_t hdf_status;
    hid_t gyro_sample_type_id = hdfio_ptr->type_id_gyro_sample;

    hdf_status = H5Tinsert(gyro_sample_type_id,
        "sync_event_time_tag",
        HOFFSET(IAS_L0R_IMU_SAMPLE, sync_event_time_tag),
         H5T_STD_I16LE);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Unable to add member to complex type");
        H5Tclose(gyro_sample_type_id);
        return NULL;
    }

    hdf_status = H5Tinsert(gyro_sample_type_id,
        "time_tag",
        HOFFSET(IAS_L0R_IMU_SAMPLE, time_tag),
        H5T_STD_U16LE);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Unable to add member to complex type");
        H5Tclose(gyro_sample_type_id);
        return NULL;
    }
    hdf_status = H5Tinsert(gyro_sample_type_id,
        "saturation_and_scaling",
        HOFFSET(IAS_L0R_IMU_SAMPLE, saturation_and_scaling),
        H5T_STD_U8LE);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Unable to add member to complex type");
        H5Tclose(gyro_sample_type_id);
        return NULL;
    }
    hdf_status = H5Tinsert(gyro_sample_type_id,
        "angular_rate_valid",
        HOFFSET(IAS_L0R_IMU_SAMPLE, angular_rate_valid),
        H5T_STD_U8LE);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Unable to add member to complex type");
        H5Tclose(gyro_sample_type_id);
        return NULL;
    }
    hdf_status = H5Tinsert(gyro_sample_type_id,
        "integrated_angle_count_1",
        HOFFSET(IAS_L0R_IMU_SAMPLE, integrated_angle_count[0]),
        H5T_STD_U16LE);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Unable to add member to complex type");
        H5Tclose(gyro_sample_type_id);
        return NULL;
    }
    hdf_status = H5Tinsert(gyro_sample_type_id,
        "integrated_angle_count_2",
        HOFFSET(IAS_L0R_IMU_SAMPLE, integrated_angle_count[1]),
        H5T_STD_U16LE);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Unable to add member to complex type");
        H5Tclose(gyro_sample_type_id);
        return NULL;
    }
    hdf_status = H5Tinsert(gyro_sample_type_id,
        "integrated_angle_count_3",
        HOFFSET(IAS_L0R_IMU_SAMPLE, integrated_angle_count[2]),
        H5T_STD_U16LE);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Unable to add member to complex type");
        H5Tclose(gyro_sample_type_id);
        return NULL;
    }
    hdf_status = H5Tinsert(gyro_sample_type_id,
        "integrated_angle_count_4",
        HOFFSET(IAS_L0R_IMU_SAMPLE, integrated_angle_count[3]),
        H5T_STD_U16LE);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Unable to add member to complex type");
        H5Tclose(gyro_sample_type_id);
        return NULL;
    }

    imu_types[i++] = H5T_STD_I32LE; /* uint32_t days from J2000 */
    imu_types[i++] = H5T_IEEE_F64LE; /* double seconds */
    imu_types[i++] = H5T_STD_I32LE; /* time_tag_sec_orig */
    imu_types[i++] = H5T_STD_I32LE; /* time_tag_subseconds_orig */
    for(j = 0; j < IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD; j++)
        imu_types[i++] = gyro_sample_type_id;
    imu_types[i++] = H5T_STD_U8LE; /* uint8_t warning_flag */

    if (i != TABLE_SIZE_IMU)
    {
        IAS_LOG_ERROR("%d elements set for IMU GYRO but the table is size %d",
            i, TABLE_SIZE_IMU);
        H5Tclose(gyro_sample_type_id);
        return NULL;
    }

    return imu_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_imu_field_type

 PURPOSE: populates the field_type_imu array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_imu_latency_field_type(void)
{
    unsigned int i = 0;

    imu_latency_types[i++] = H5T_STD_I32LE; /* int32_t days from J2000 */
    imu_latency_types[i++] = H5T_IEEE_F64LE; /* double seconds */
    imu_latency_types[i++] = H5T_IEEE_F64LE; /* double fine_ad_solution_time */
    imu_latency_types[i++] = H5T_IEEE_F32LE; /* float measured_imu_latency */
    imu_latency_types[i++] = H5T_STD_U8LE; /* uint8_t warning_flag */

    if (i != TABLE_SIZE_IMU_LATENCY)
    {
        IAS_LOG_ERROR("%d elements set for IMU LATENCY but the "
            "table is size %d",
            i, TABLE_SIZE_IMU_LATENCY);
        return NULL;
    }

    return imu_latency_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_oli_telemetry_group_3_field_type

 PURPOSE: populates the field_type_telemetry_group_3 array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_oli_telemetry_group_3_field_type(void)
{
    unsigned int i = 0;

    /* Array of reserved fields */
    hid_t array_id_reserved;
    hsize_t dims_reserved[1] = {IAS_L0R_OLI_TELEMETRY_GROUP_3_RESERVED};

    /* Array of spare fields */
    hid_t array_id_spare;
    hsize_t dims_spare[1] = {IAS_L0R_OLI_TELEMETRY_GROUP_3_SPARE};

    array_id_reserved = H5Tarray_create( H5T_STD_U8LE, 1, dims_reserved);
    if (array_id_reserved < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }
    array_id_spare = H5Tarray_create( H5T_STD_U8LE, 1, dims_spare);
    if (array_id_spare < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }

    telemetry_group_3_types[i++] = H5T_STD_I32LE; /* l0r_time_days */
    telemetry_group_3_types[i++] = H5T_IEEE_F64LE; /* l0r_time_sec */
    telemetry_group_3_types[i++] = H5T_STD_I16LE; /* days_original */
    telemetry_group_3_types[i++] = H5T_STD_I32LE; /* milliseconds_original */
    telemetry_group_3_types[i++] = H5T_STD_I16LE; /* microseconds_original */
    telemetry_group_3_types[i++] = H5T_STD_U16LE; /* sync_word */
    telemetry_group_3_types[i++] = H5T_STD_U16LE; /* id */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE;
    /* stim_lamp_output_current_amps */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* stim_lamp_bulb_a_volts */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* stim_lamp_bulb_b_volts */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* stim_lamp_thermistor1 */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* stim_lamp_thermistor2 */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; 
    /* stim_lamp_photodiode1_micro_amps */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; 
    /* stim_lamp_photodiode2_micro_amps */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* focus_motor_lvdt_1 */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* focus_motor_lvdt_2 */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* focus_motor_lvdt_3 */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE;
    /* pos_z_minus_y_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* bench_temp_1_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* bench_temp_2_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* bench_temp_3_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* bench_temp_4_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* bench_temp_5_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* bench_temp_7_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* bench_temp_8_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* fpm_7_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE;
    /* calibration_assembly_a_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE;
    /* pos_z_pos_y_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE;
    /* tert_mirror_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* fp_chassis_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* pos_y_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* fp_evap_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* fp_window_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE;
    /* minus_z_pos_y_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE;
    /* minus_z_minus_y_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* minus_y_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* fpm_14_temp_celsius */
    telemetry_group_3_types[i++] = H5T_IEEE_F32LE; /* lvps_temp_celsius */
    telemetry_group_3_types[i++] = array_id_reserved; /* reserved */
    telemetry_group_3_types[i++] = array_id_spare; /* spare */
    telemetry_group_3_types[i++] = H5T_STD_U8LE; /* warning_flag */

    if (i != TABLE_SIZE_TELEMETRY_GROUP_3)
    {
        IAS_LOG_ERROR("%d elements set for Payload OLI Instrument"
            " but the table is size %d",
            i, TABLE_SIZE_TELEMETRY_GROUP_3);
        return NULL;
    }

    return telemetry_group_3_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_oli_telemetry_group_4_field_type

 PURPOSE: populates the field_type_telemetry_oli_group_4 array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_oli_telemetry_group_4_field_type(void)
{
    unsigned int i = 0;

    /* Array of reserved fields */
    hid_t array_id_reserved;
    hsize_t dims_reserved[1] = {IAS_L0R_OLI_TELEMETRY_GROUP_4_RESERVED};
    /* Array of spare fields */
    hid_t array_id_spare;
    hsize_t dims_spare[1] = {IAS_L0R_OLI_TELEMETRY_GROUP_4_SPARE};

    array_id_reserved = H5Tarray_create( H5T_STD_U8LE, 1, dims_reserved);
    if (array_id_reserved < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }
    array_id_spare = H5Tarray_create( H5T_STD_U8LE, 1, dims_spare);
    if (array_id_spare < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }

    telemetry_group_4_types[i++] = H5T_STD_I32LE; /* int32_t days_from_J2000 */
    telemetry_group_4_types[i++] = H5T_IEEE_F64LE; /* seconds*/
    telemetry_group_4_types[i++] =
        H5T_STD_I16LE; /* days_orig */
    telemetry_group_4_types[i++] =
        H5T_STD_I32LE; /* millisecond_orig */
    telemetry_group_4_types[i++] =
        H5T_STD_I16LE; /* microsecond_orig */
    telemetry_group_4_types[i++] = H5T_STD_U16LE; /* sync_word */
    telemetry_group_4_types[i++] = H5T_STD_U16LE; /* id */
    telemetry_group_4_types[i++] = H5T_STD_U8LE; /* mech_command_reject_count */
    telemetry_group_4_types[i++] = H5T_STD_U8LE; /* mech_command_accept_count */
    telemetry_group_4_types[i++] = H5T_STD_U8LE; /* shutter_active */
    telemetry_group_4_types[i++] = H5T_STD_U8LE; /* last_command_opcode */
    telemetry_group_4_types[i++] = H5T_STD_U8LE; /* diffuser_active */
    telemetry_group_4_types[i++] = H5T_STD_U8LE; /* shutter_commanded_moves */
    telemetry_group_4_types[i++] = H5T_STD_U8LE; /* focus_motor_flags */
    telemetry_group_4_types[i++] = H5T_STD_U8LE;
    /* diffuser_commanded_moves */
    telemetry_group_4_types[i++] = H5T_IEEE_F64LE;
    /* focus_motor_pulse_time_step_sec */
    telemetry_group_4_types[i++] = H5T_IEEE_F64LE;
    /* focus_motor_pulse_length_sec */
    telemetry_group_4_types[i++] = H5T_STD_U16LE;
    /* focus_motor_pulses */
    telemetry_group_4_types[i++] = H5T_STD_U8LE;
    /* focus_mechanism_lvdt_relay_status */
    telemetry_group_4_types[i++] = H5T_STD_U8LE;
    /* focus_mechanism_motor_relay_status */
    telemetry_group_4_types[i++] = H5T_IEEE_F64LE;
    /* shutter_motor_pulse_length_sec */
    telemetry_group_4_types[i++] = H5T_STD_U8LE;
    /* shutter_status_flags */
    telemetry_group_4_types[i++] = H5T_STD_U8LE;
    /* diffuser_status_flags */
    telemetry_group_4_types[i++] = H5T_IEEE_F64LE;
    /* shutter_motor_pulse_time_sec */
    telemetry_group_4_types[i++] = H5T_IEEE_F64LE;
    /* diffuser_motor_pulse_time_sec */
    telemetry_group_4_types[i++] = H5T_IEEE_F64LE;
    /* diffuser_motor_pulse_length_sec */
    telemetry_group_4_types[i++] = H5T_STD_U16LE;
    /* shutter_move_count */
    telemetry_group_4_types[i++] = H5T_STD_U16LE;
    /* shutter_resolver_position */
    telemetry_group_4_types[i++] = H5T_STD_U16LE;
    /* diffuser_move_count */
    telemetry_group_4_types[i++] = H5T_STD_U16LE;
    /* diffuser_resolver_position */
    telemetry_group_4_types[i++] = H5T_STD_U16LE;
    /* diffuser_flags */
    telemetry_group_4_types[i++] = H5T_STD_U8LE;
    /* stl_command_rejected_count */
    telemetry_group_4_types[i++] = H5T_STD_U8LE;
    /* stl_command_accepted_count */
    telemetry_group_4_types[i++] = H5T_STD_U8LE; /* stl_power_flags */
    telemetry_group_4_types[i++] = H5T_STD_U8LE; /* stl_last_accepted_command */
    telemetry_group_4_types[i++] = H5T_STD_U8LE; /* stl_flags */
    telemetry_group_4_types[i++] = array_id_reserved; /* reserved */
    telemetry_group_4_types[i++] = array_id_spare; /* spare */
    telemetry_group_4_types[i++] =
        H5T_STD_U8LE; /* warning_flag */
    if (i != TABLE_SIZE_TELEMETRY_GROUP_4)
    {
        IAS_LOG_ERROR("%d elements set for Payload OLI Mechanism"
            " but the table is size %d",
            i, TABLE_SIZE_TELEMETRY_GROUP_4);
        return NULL;
    }

    return telemetry_group_4_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_oli_telemetry_group_5_field_type

 PURPOSE: populates the field_type_telemetry_oli_group_5 array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_oli_telemetry_group_5_field_type(void)
{
    unsigned int i = 0;

    /* Array of reserved fields */
    hid_t array_id_reserved;
    hsize_t dims_reserved[1] = {IAS_L0R_OLI_TELEMETRY_GROUP_5_RESERVED};
    /* Array of spare fields */
    hid_t array_id_spare;
    hsize_t dims_spare[1] = {IAS_L0R_OLI_TELEMETRY_GROUP_5_SPARE};

    array_id_reserved = H5Tarray_create( H5T_STD_U8LE, 1, dims_reserved);
    if (array_id_reserved < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }
    array_id_spare = H5Tarray_create( H5T_STD_U8LE, 1, dims_spare);
    if (array_id_spare < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }

    telemetry_group_5_types[i++] = H5T_STD_I32LE; /* int32_t days_from_J2000 */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE; /* seconds */
    telemetry_group_5_types[i++] = H5T_STD_I16LE; /* days_orig */
    telemetry_group_5_types[i++] = H5T_STD_I32LE; /* millisecond_orig */
    telemetry_group_5_types[i++] = H5T_STD_I16LE; /* microsecond_orig */
    telemetry_group_5_types[i++] = H5T_STD_U16LE; /* sync_word */
    telemetry_group_5_types[i++] = H5T_STD_U16LE; /* id */
    telemetry_group_5_types[i++] = H5T_STD_U8LE; /* fpe_command_reject_count */
    telemetry_group_5_types[i++] = H5T_STD_U8LE; /* fpe_command_accept_count */
    telemetry_group_5_types[i++] = H5T_STD_U8LE;
    /* safe_mode_consecutive_requests */
    /* Number of consecutive unacknowledged telemetry requests
        allowed before FSW transitions to safe mode.*/
    telemetry_group_5_types[i++] = H5T_STD_U8LE; /* last_command_opcode */
    telemetry_group_5_types[i++] = H5T_STD_U8LE;
    /* single_bit_edac_errors_detected */
    telemetry_group_5_types[i++] = H5T_STD_U8LE;
         /* consecutive_unacknowledged_requests */
    telemetry_group_5_types[i++] = H5T_STD_U8LE;
    /* fpe_message_errors_detected */
    telemetry_group_5_types[i++] = H5T_STD_U8LE;
    /* multi_bit_edac_errors_detected */
    telemetry_group_5_types[i++] = H5T_STD_U16LE;/* messages_forwarded_to_fpe */
    telemetry_group_5_types[i++] = H5T_STD_U8LE; /* command_sequence_count */
    telemetry_group_5_types[i++] = H5T_STD_U8LE;
    /* messages_reject_invalid_mode */
    telemetry_group_5_types[i++] = H5T_STD_U8LE; /* fpe_telemetry_valid */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* dlvps_relay_pos_28vdc_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE; /* dlvps_pos_5v_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE; /* dlvps_pos_15v_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE; /* dlvps_neg_15v_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE; /* dlvps_pos_3_3v_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* alvps_hv_bias_pos_85v_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE; /* alvps_pos_12v_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE; /* alvps_pos_7_5v_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE; /* alvps_neg_2_5v_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* alvps_pos_12v_current_amps */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* alvps_pos_7_5v_current_amps */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* alvps_pos_2_5v_current_amps */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* lvps_temperature_sensor_celsius */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* ctlr_temperature_sensor_celsius */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* ana_0_temperature_sensor_celsius */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* ana_1_temperature_sensor_celsius */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* ana_0_ch_0_vpa_bias_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* ana_0_ch_1_vpa_bias_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* ana_0_ch_2_vpa_bias_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* ana_0_ch_3_vpa_bias_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* ana_0_ch_4_vpa_bias_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* ana_0_ch_5_vpa_bias_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* ana_0_ch_6_vpa_bias_voltage */
    telemetry_group_5_types[i++] = H5T_IEEE_F64LE;
    /* ana_0_ch_7_vpa_bias_voltage */
    telemetry_group_5_types[i++] = array_id_reserved; /* reserved */
    telemetry_group_5_types[i++] = array_id_spare; /* spare */
    telemetry_group_5_types[i++] =
        H5T_STD_U8LE; /* warning_flag */
    if (i != TABLE_SIZE_TELEMETRY_GROUP_5)
    {
        IAS_LOG_ERROR("%d elements set for Payload OLI STL"
            " but the table is size %d",
            i, TABLE_SIZE_TELEMETRY_GROUP_5);
        return NULL;
    }

    return telemetry_group_5_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_tirs_telemetry_field_type

 PURPOSE: populates the field_type_tirs_telemetry array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_tirs_telemetry_field_type(void)
{
    unsigned int i = 0;

    /* Array fields */
    hid_t array_id_ssm;
    hsize_t dims_ssm[1] =
        {IAS_L0R_TIRS_TELEMETRY_SSM_ENCODER_POSITION_SAMPLE_COUNT};
    /* Arrays of reserved fields */
    hid_t array_id_reserved_2;
    hsize_t dims_reserved_2[1] = {IAS_L0R_TIRS_TELEMETRY_BLOCK_2_RESERVED};
    hid_t array_id_reserved_3;
    hsize_t dims_reserved_3[1] = {IAS_L0R_TIRS_TELEMETRY_BLOCK_3_RESERVED};
    hid_t array_id_reserved_4;
    hsize_t dims_reserved_4[1] = {IAS_L0R_TIRS_TELEMETRY_BLOCK_4_RESERVED};
    hid_t array_id_black_body_temps;
    hsize_t dims_black_body_temps[1] =
        {IAS_L0R_TIRS_TELEMETRY_NUM_BLACK_BODY_TEMPS};


    array_id_ssm = H5Tarray_create( H5T_STD_U32LE, 1, dims_ssm);
    if (array_id_ssm < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }
    array_id_reserved_2 = H5Tarray_create( H5T_STD_U8LE, 1, dims_reserved_2);
    if (array_id_reserved_2 < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }
    array_id_reserved_3 = H5Tarray_create( H5T_STD_U8LE, 1, dims_reserved_3);
    if (array_id_reserved_3 < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }
    array_id_reserved_4 = H5Tarray_create( H5T_STD_U8LE, 1, dims_reserved_4);
    if (array_id_reserved_4 < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }
    array_id_black_body_temps = H5Tarray_create( H5T_IEEE_F32LE, 1,
        dims_black_body_temps);
    if (array_id_black_body_temps < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }


    tirs_telemetry_types[i++] = H5T_STD_I32LE; /* l0r_time_days */
    tirs_telemetry_types[i++] = H5T_IEEE_F64LE; /* l0r_time_sec */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* unaccepted_command_count */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* accepted_command_count */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* pulse_per_second_count */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* tod_command_counter */
    tirs_telemetry_types[i++] = H5T_STD_I16LE; /* day */
    tirs_telemetry_types[i++] = H5T_STD_I32LE; /* millisecond */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* mc_encoder_flags */
    tirs_telemetry_types[i++] = H5T_STD_U16LE;
        /* science_data_frame_capture_count */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* science_acquisition_frame_rate */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* active_timing_table_pattern */
    tirs_telemetry_types[i++] = H5T_STD_U16LE; /* mode_register */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* timing_table_pattern_id_1 */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* timing_table_pattern_id_2 */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* timing_table_pattern_id_3 */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* ssm_position_sel */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* ssm_mech_mode */ 
    tirs_telemetry_types[i++] = array_id_ssm; 
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* bbcal_op7_a_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* bbcal_op7_b_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* bbcal_supp_1_celsius */
    tirs_telemetry_types[i++] = array_id_black_body_temps;
        /*blackbody_calibrator_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* cold_stage_heat_strap_cf_if_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* cryo_diode_t3_measured_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* cryo_diode_t4_measured_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* cryo_shroud_outer_at_tunnel_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* cryo_shroud_outer_flange_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fixed_baff_nadir_aft_hot_corner_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fixed_baff_nadir_aft_space_corner_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fixed_baff_nadir_fwd_hot_corner_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fixed_baff_nadir_fwd_space_corner_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* fp_a_asic_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* fp_b_asic_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* fpe1_fpe_a_asic_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* fpe2_fpe_b_asic_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_f2_fine_sensor_1_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_f4_fine_sensor_3_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_f6_fine_sensor_1_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_f7_fine_sensor_2_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* fp_op6_a_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* fp_op6_b_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* optical_deck_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* spare_4_thermistor_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* spare_5_thermistor_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* ssm_bearing_aft_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* ssm_bearing_fwd_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* ssm_bearing_housing_d4_aft_hot_side_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* ssm_bearing_housing_d5_fwd_hot_side_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* ssm_bearing_housing_d6_aft_space_side_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* ssm_bearing_housing_d7_fwd_space_side_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* ssm_bh_op5_a_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* ssm_bh_op5_b_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* ssm_encoder_remote_elec_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* ssm_enc_read_head_sensor_1_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* ssm_motor_housing_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* structure_foot_a_neg_z_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* structure_foot_c_pos_z_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* structure_nadir_aperture_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* tcb_board_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* telescope_aft_barrel_neg_z_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* telescope_aft_barrel_pos_z_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* telescope_aft_op3_a_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* telescope_aft_op3_b_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* telescope_fwd_barrel_neg_z_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* telescope_fwd_barrel_pos_z_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* telescope_fwd_op4_a_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* telescope_fwd_op4_b_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* telescope_stage_op2_a_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* telescope_stage_op2_b_celsius */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* fp_a_mon_pos_12v_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_a_vpd_current_1_amps_1 */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_a_vpd_current_1_amps_2 */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_a_vpd_current_1_amps_3 */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_detector_substrate_conn_for_sca_a_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_detector_substrate_conn_for_sca_b_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_detector_substrate_conn_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_digi_supply_mon_pos_5_5_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_supply_mon_pos_5_5_for_sca_a_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_supply_mon_pos_5_5_for_sca_b_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_supply_mon_pos_5_5_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_output_ref_level_mon_5_5_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_supply_10v_for_sca_a_current_mon_amps */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_supply_10v_for_sca_b_current_mon_amps */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_supply_10v_for_sca_c_current_mon_amps */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_output_driver_pos_5_5_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_output_ref_level_1_6_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_a_channel_ref_suppy_1_6_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* vpe_a_sca_a_video_ref */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* vpe_a_sca_b_video_ref */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* vpe_a_sca_c_video_ref */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* fp_b_mon_pos_12v_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_a_vpd_current_1_amps_1 */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_a_vpd_current_1_amps_2 */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_a_vpd_current_1_amps_3 */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_detector_substrate_conn_for_sca_a_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_detector_substrate_conn_for_sca_b_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_digi_supply_mon_pos_5_5_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_supply_mon_pos_5_5_for_sca_a_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_supply_mon_pos_5_5_for_sca_b_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_supply_mon_pos_5_5_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_output_ref_level_mon_5_5_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_supply_10v_for_sca_a_current_mon_amps */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_supply_10v_for_sca_b_current_mon_amps */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_supply_10v_for_sca_c_current_mon_amps */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* hsib_3_3_current_mon_amps */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_output_driver_pos_5_5_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_output_ref_level_1_6_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* fp_b_channel_ref_suppy_1_6_for_sca_c_roic_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* vpe_b_sca_a_video_ref */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* vpe_b_sca_b_video_ref */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE; /* vpe_b_sca_c_video_ref */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* cosine_motor_drive_for_mce_current_amps */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* sine_motor_drive_for_mce_current_amps */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* cosine_dac_telemetry_for_mce_volts */
    tirs_telemetry_types[i++] = H5T_IEEE_F32LE;
        /* sine_dac_telemetry_for_mce_volts */
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* elec_enabled_flags */
    tirs_telemetry_types[i++] = array_id_reserved_2;
    tirs_telemetry_types[i++] = array_id_reserved_3;
    tirs_telemetry_types[i++] = array_id_reserved_4;
    tirs_telemetry_types[i++] = H5T_STD_U8LE; /* warning_flag */

    if (i != TABLE_SIZE_TIRS_TELEMETRY)
    {
        IAS_LOG_ERROR("%d elements set for Payload TIRS"
            " but the table is size %d",
            i, TABLE_SIZE_TIRS_TELEMETRY);
        return NULL;
    }

    return tirs_telemetry_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_temperatures_gyro_field_type

 PURPOSE: populates the field_type_temperatures_gyro array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_temperatures_gyro_field_type(void)
{
    int i = 0;
    int j = 0;

    /* Array of reserved fields */
    hid_t array_id_reserved;
    hsize_t dims_reserved[1] = {IAS_L0R_GYRO_TEMPERATURE_RESERVED};

    array_id_reserved = H5Tarray_create( H5T_STD_U8LE, 1, dims_reserved);
    if (array_id_reserved < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }

    temperatures_gyro_types[i++] =
        H5T_STD_I32LE; /* int32_t days_from_J2000 */
    temperatures_gyro_types[i++] =
        H5T_IEEE_F64LE; /* seconds */
    for(j = 0; j < IAS_L0R_GYRO_TEMPERATURE_NUM_INDEXES; j++)
    {
        temperatures_gyro_types[i++] = H5T_IEEE_F32LE;
    }

    /* reserved bytes */
    temperatures_gyro_types[i++] = array_id_reserved;
    /* warning_flag */
    temperatures_gyro_types[i++] = H5T_STD_U8LE;

    if (i != TABLE_SIZE_TEMPERATURES_GYRO)
    {
        IAS_LOG_ERROR("%d elements set for ACS temperatures"
            " but the table is size %d",
            i, TABLE_SIZE_TEMPERATURES_GYRO);
        return NULL;
    }

    return temperatures_gyro_types;
}

/******************************************************************************
 NAME: ias_l0r_init_ancillary_temperatures_oli_tirs_field_type

 PURPOSE: populates the field_type_temperatures_oli array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_init_ancillary_temperatures_oli_tirs_field_type(void)
{
    int i = 0;
    int j = 0;

    temperatures_oli_tirs_types[i++] =
        H5T_STD_I32LE; /* int32_t days_from_J2000 */
    temperatures_oli_tirs_types[i++] =
        H5T_IEEE_F64LE; /* seconds */
    for(j = 0; j < IAS_L0R_OLI_TIRS_TEMPERATURE_NUM_INDEXES; j++)
    {
        temperatures_oli_tirs_types[i++] =
            H5T_IEEE_F32LE;
    }
     temperatures_oli_tirs_types[i++] = H5T_STD_U8LE; /* warning_flag */

    if (i != TABLE_SIZE_TEMPERATURES_OLI_TIRS)
    {
        IAS_LOG_ERROR("%d elements set for OLI temperatures"
            " but the table is size %d",
            i, TABLE_SIZE_TEMPERATURES_OLI_TIRS);
    }

    return temperatures_oli_tirs_types;
}

/******************************************************************************
 NAME: ias_l0r_anc_init

 PURPOSE: performs all initializations needed for the library to access
          ancillary data stored in HDF files.

 RETURNS: SUCCESS- Ancillary types populated
          ERROR- Unable to populate ancillary types
******************************************************************************/
int ias_l0r_anc_init(HDFIO* hdfio_ptr)
{
    if (field_type_attitude == NULL)
    {
        field_type_attitude = ias_l0r_init_ancillary_attitude_field_type();
        if (field_type_attitude == NULL)
            return ERROR;
    }
    if (field_type_attitude_filter == NULL)
    {
        field_type_attitude_filter =
            ias_l0r_init_ancillary_attitude_filter_states_field_type();
        if (field_type_attitude_filter == NULL)
            return ERROR;
    }
    if (field_type_ephemeris == NULL)
    {
        field_type_ephemeris = ias_l0r_init_ancillary_ephem_field_type();
        if (field_type_ephemeris == NULL)
            return ERROR;
    }
    if (field_type_gps_position == NULL)
    {
        field_type_gps_position = ias_l0r_init_ancillary_gps_field_type();
        if (field_type_gps_position == NULL)
            return ERROR;
    }
    if (field_type_gps_range == NULL)
    {
        field_type_gps_range = ias_l0r_init_ancillary_gps_range_field_type();
        if (field_type_gps_range == NULL)
            return ERROR;
    }
    if (field_type_imu == NULL)
    {
        field_type_imu = ias_l0r_init_ancillary_imu_field_type(hdfio_ptr);
        if (field_type_imu == NULL)
            return ERROR;
    }
    if (field_type_imu_latency == NULL)
    {
        field_type_imu_latency =
        ias_l0r_init_ancillary_imu_latency_field_type();
        if (field_type_imu_latency == NULL)
            return ERROR;
    }
    if (field_type_telemetry_group_3 == NULL)
    {
        field_type_telemetry_group_3 =
            ias_l0r_init_ancillary_oli_telemetry_group_3_field_type();
        if (field_type_telemetry_group_3 == NULL)
            return ERROR;
    }
    if (field_type_telemetry_oli_group_4 == NULL)
    {
        field_type_telemetry_oli_group_4 =
            ias_l0r_init_ancillary_oli_telemetry_group_4_field_type();
        if (field_type_telemetry_oli_group_4 == NULL)
            return ERROR;
    }
    if (field_type_telemetry_oli_group_5 == NULL)
    {
        field_type_telemetry_oli_group_5 =
            ias_l0r_init_ancillary_oli_telemetry_group_5_field_type();
        if (field_type_telemetry_oli_group_5 == NULL)
            return ERROR;
    }
    if (field_type_tirs_telemetry == NULL)
    {
        field_type_tirs_telemetry =
            ias_l0r_init_ancillary_tirs_telemetry_field_type();
        if (field_type_tirs_telemetry == NULL)
            return ERROR;
    }
    if (field_type_star_tracker_centroid == NULL)
    {
        field_type_star_tracker_centroid =
            ias_l0r_init_ancillary_st_centroid_field_type();
        if (field_type_star_tracker_centroid == NULL)
            return ERROR;
    }
    if (field_type_star_tracker_quaternion == NULL)
    {
        field_type_star_tracker_quaternion =
            ias_l0r_init_ancillary_st_quaternion_field_type();
        if (field_type_star_tracker_quaternion == NULL)
            return ERROR;
    }
    if (field_type_temperatures_gyro == NULL)
    {
        field_type_temperatures_gyro =
            ias_l0r_init_ancillary_temperatures_gyro_field_type();
        if (field_type_temperatures_gyro == NULL)
            return ERROR;
    }
    if (field_type_temperatures_oli_tirs == NULL)
    {
        field_type_temperatures_oli_tirs =
            ias_l0r_init_ancillary_temperatures_oli_tirs_field_type();
        if (field_type_temperatures_oli_tirs == NULL)
            return ERROR;
    }
    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_get_anc_table_records_count

 PURPOSE: Internal routine to read the size of the specified table in
          the ancillary file

 RETURNS: SUCCESS- Size was read
          ERROR- Unable to read size
******************************************************************************/
static int ias_l0r_get_anc_table_records_count
(
    HDFIO *hdfio_ptr, /* I: Pointer to structure used in I/O */
    const char *table_name, /* I: Name of table to get count for */
    int *size          /* O: Number of records in the table */
)
{
    int status;
   
    if (hdfio_ptr->access_mode_ancillary < 0)
    {
        IAS_LOG_ERROR("Ancillary data is not open");
        return ERROR;
    }
 
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, FALSE);
    if (status == ERROR)
    {
        /* if the ancillary file can not be opened, the size cannot be
           read.  Set size to 0 and return ERROR */
        *size = 0;
        return ERROR;
    }
    /* To support not creating empty files,
       the case for size 0 is the establish
       routine returning success but the file_id being uninitialized */
    else if (hdfio_ptr->file_id_ancillary < 1)
    {
        *size = 0;
        return SUCCESS;
    }

    /* otherwise set the size to the number of entries in the table */
    return ias_l0r_hdf_table_records_count
        (hdfio_ptr->file_id_ancillary, table_name, size);
}

/* ****************************************************************************/
/* public routine definitions */
/* ************************************************************************** */
/*******************************************************************************
 NAME: ias_l0r_open_ancillary
 
 PURPOSE: Opens access to the L0R ancillary data
 
 RETURNS: SUCCESS- Ancillary file opened
            ERROR- Unable to open file
*******************************************************************************/
int ias_l0r_open_ancillary
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const IAS_ACCESS_MODE file_access_mode /* I: Mode to open file in */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr = l0r;
  
    if (hdfio_ptr->access_mode_ancillary > 0)
    {
        IAS_LOG_ERROR("File access mode is already set");
        return ERROR;
    }
    hdfio_ptr->access_mode_ancillary = file_access_mode;

    return SUCCESS;
}

/*******************************************************************************
 NAME: ias_close_ancillary
 
 PURPOSE: Closes access to the L0R ancillary data
 
 RETURNS: SUCCESS- Ancillary file close
            ERROR- Unable to close file
*******************************************************************************/
int ias_l0r_close_ancillary
(
    L0RIO *l0r /* I: structure used with the L0R data */
)
{
    int error_count = 0;
    herr_t status;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr = l0r;
 
    if (hdfio_ptr->table_id_attitude > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_attitude);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing Attitude dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_attitude = -1;
    }
 
    if (hdfio_ptr->table_id_attitude_filter > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_attitude_filter);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing Attitude Filter State dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_attitude_filter = -1;
    }
 
    if (hdfio_ptr->table_id_ephemeris > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_ephemeris);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing Ephemeris dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_ephemeris = -1;
    }
 
    if (hdfio_ptr->table_id_gps_position > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_gps_position);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing GPS Position dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_gps_position = -1;
    }
 
    if (hdfio_ptr->table_id_gps_range > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_gps_range);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing GPS Range dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_gps_range = -1;
    }
 
    if (hdfio_ptr->table_id_imu > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_imu);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing IMU dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_imu = -1;
    }

    if (hdfio_ptr->table_id_telemetry_group_3 > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_telemetry_group_3);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing OLI Telemetry Group 3 dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_telemetry_group_3 = -1;
    }
 
    if (hdfio_ptr->table_id_telemetry_group_4 > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_telemetry_group_4);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing OLI Telemetry Group 4 dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_telemetry_group_4 = -1;
    }

    if (hdfio_ptr->table_id_telemetry_group_5 > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_telemetry_group_5);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing OLI Telemetry Group 5 dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_telemetry_group_5 = -1;
    }

    if (hdfio_ptr->table_id_star_tracker_centroid > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_star_tracker_centroid);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing Star Tracker Centroid dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_star_tracker_centroid = -1;
    }
 
    if (hdfio_ptr->table_id_star_tracker_quaternion > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_star_tracker_quaternion);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing Star Tracker Quaternion dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_star_tracker_quaternion = -1;
    }
 
    if (hdfio_ptr->table_id_temperatures_gyro > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_temperatures_gyro);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing ACS Temperatures dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_temperatures_gyro = -1;
    }

    if (hdfio_ptr->table_id_temperatures_oli_tirs > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_temperatures_oli_tirs);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing OLI/TIRS Temperatures dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_temperatures_oli_tirs = -1;
    }

    if (hdfio_ptr->table_id_tirs_telemetry > 0)
    {
        status = H5Dclose(hdfio_ptr->table_id_tirs_telemetry);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing TIRS Telemetry dataset"
                " for file: %s",hdfio_ptr->l0r_name_prefix);
            error_count++;
        }
        hdfio_ptr->table_id_tirs_telemetry = -1;
    }

    if (error_count > 0)
    {
        IAS_LOG_ERROR("%i errors encountered when closing ancillary groups",
            error_count);
        return ERROR;
    }

    hdfio_ptr->access_mode_ancillary = -1;

    /* if both the headers and ancillary data are not accessing the file
    * close the file */
    if (hdfio_ptr->access_mode_header < 0)
    {
        if (hdfio_ptr->file_id_ancillary > 0)
        {
            status = H5Fclose(hdfio_ptr->file_id_ancillary);
            if (status < 0)
            {
                IAS_LOG_ERROR("Error closing hdf resources"
                    " file_id_ancillary %d",
                    hdfio_ptr->file_id_ancillary);
                return ERROR;
            }
            hdfio_ptr->file_id_ancillary = -1;
        }
    }
    else /*Only flush the ancillary and leave header*/
    {
        if (hdfio_ptr->file_id_ancillary > 0)
        {
            status = H5Fflush(hdfio_ptr->file_id_ancillary,
                H5F_SCOPE_LOCAL);
            if (status < 0)
            {
                IAS_LOG_ERROR("Error flushing hdf resources"
                    "file_id_ancillary %d",
                    hdfio_ptr->file_id_ancillary);
                return ERROR;
            }
            hdfio_ptr->file_id_ancillary = -1;
        }
    }

    return SUCCESS;
}

/*******************************************************************************
 NAME: ias_l0r_copy_ancillary
 
 PURPOSE: Copy the ancillary data from one L0R dataset to another
 
 RETURNS: SUCCESS- Ancillary copied
          ERROR- Unable to copy ancillary
*******************************************************************************/
int ias_l0r_copy_ancillary
(
    L0RIO *l0r_source, /* I: structure used with the input L0R data */
    L0RIO *l0r_destination /* I: structure used with the output L0R data */
)
{
    int status;
    herr_t hdf_status;
    hid_t copy_props;
    hid_t link_props;

    if (l0r_source == NULL || l0r_destination == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    /* make sure the source file exists */
    status = ias_l0r_establish_ancillary_file(l0r_source, FALSE);
    if (status == ERROR || l0r_source->file_id_ancillary < 1)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Unable to establish file in ias_l0r_copy_ancillary");
        return ERROR;
    }

    /* make sure the destination file exists (create if it does not) */
    status = ias_l0r_establish_ancillary_file(l0r_destination, TRUE);
    if (status == ERROR || l0r_destination->file_id_ancillary < 1)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Unable to establish file in ias_l0r_copy_ancillary");
        return ERROR;
    }

    /* Create the empty (for now) property lists */
    copy_props = H5Pcreate ( H5P_OBJECT_COPY );
    link_props = H5Pcreate ( H5P_LINK_CREATE );

    /* Flush the source data in cache to the file to ensure it gets copied */
    hdf_status = H5Fflush( l0r_source->file_id_ancillary, H5F_SCOPE_LOCAL );

    /* copy the Spacecraft Group */
    hdf_status = H5Ocopy( l0r_source->file_id_ancillary, GROUP_NAME_SPACECRAFT,
         l0r_destination->file_id_ancillary, GROUP_NAME_SPACECRAFT,
         copy_props, link_props );
    if ( hdf_status < 0 )
    {
        IAS_LOG_ERROR("Unable to copy Spacecraft group in "
            "ias_l0r_copy_ancillary");
        H5Pclose( copy_props );
        H5Pclose( link_props );
        return ERROR;
    }

    /* copy the Telemetry Group */
    hdf_status = H5Ocopy( l0r_source->file_id_ancillary, GROUP_NAME_TELEMETRY,
         l0r_destination->file_id_ancillary, GROUP_NAME_TELEMETRY,
         copy_props, link_props );
    if ( hdf_status < 0 )
    {
        IAS_LOG_ERROR("Unable to copy Telemetry group in "
            "ias_l0r_copy_ancillary");
        H5Pclose( copy_props );
        H5Pclose( link_props );
        return ERROR;
    }

    hdf_status = H5Pclose( copy_props );
    if ( hdf_status < 0 )
    {
        IAS_LOG_ERROR("Unable to close property list in "
            "ias_l0r_copy_ancillary");
        /* try to close the other property list before exiting */
        H5Pclose( link_props );
        return ERROR;
    }
    hdf_status = H5Pclose( link_props );
    if ( hdf_status < 0 )
    {
        IAS_LOG_ERROR("Unable to close property list in "
            "ias_l0r_copy_ancillary");
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_append_ancillary_attitude

 PURPOSE: Appends the specified data to the applicable tables

 RETURNS: SUCCESS- The data was append
          ERROR- The data could not be appended
******************************************************************************/
int ias_l0r_append_ancillary_attitude
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count, /* I: number of items to append */
    const IAS_L0R_ATTITUDE *attitude /* I: data to append */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary < 1)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_append_ancillary_attitude");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT"/"GROUP_NAME_ACS,
        TABLE_NAME_ATTITUDE, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_attitude, TABLE_SIZE_ATTITUDE,
        IAS_L0R_ATTITUDE_SIZE, FIELD_NAMES_ATTITUDE,
        OFFSET_ATTITUDE, field_type_attitude);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_append_ancillary_attitude");
        return ERROR;
    }

    return ias_l0r_append_ancillary_records(l0r, count, attitude,
        "/"GROUP_NAME_SPACECRAFT"/"GROUP_NAME_ACS"/"TABLE_NAME_ATTITUDE,
        IAS_L0R_ATTITUDE_SIZE, OFFSET_ATTITUDE,
        IAS_L0R_ATTITUDE_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_attitude

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_attitude
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count,  /* the number of items pointed to by acs__attitude */
    const IAS_L0R_ATTITUDE *attitude /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_set_ancillary_attitude");
        return ERROR;
    }

    /* establish the structure in the file to contain the data */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT"/"GROUP_NAME_ACS,
        TABLE_NAME_ATTITUDE, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_attitude, TABLE_SIZE_ATTITUDE,
        IAS_L0R_ATTITUDE_SIZE, FIELD_NAMES_ATTITUDE,
        OFFSET_ATTITUDE, field_type_attitude);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_set_ancillary_attitude");
        return ERROR;
    }
  
    /* write the data into the file using the subroutine for ancillary data */
    return ias_l0r_anc_write(l0r, index, count, attitude,
       GROUP_NAME_SPACECRAFT"/"GROUP_NAME_ACS"/"TABLE_NAME_ATTITUDE,
       IAS_L0R_ATTITUDE_SIZE, OFFSET_ATTITUDE,
       IAS_L0R_ATTITUDE_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_attitude_filter_states

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_attitude_filter_states
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count,  /* the number of items pointed
                        to by acs__attitude_filter */
    const IAS_L0R_ATTITUDE_FILTER_STATES *attitude_filter /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_set_ancillary_attitude_filter");
        return ERROR;
    }

    /* establish the structure in the file to contain the data */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT"/"GROUP_NAME_ACS,
        TABLE_NAME_ATTITUDE_FILTER, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_attitude_filter, TABLE_SIZE_ATTITUDE_FILTER,
        IAS_L0R_ATTITUDE_FILTER_SIZE, FIELD_NAMES_ATTITUDE_FILTER,
        OFFSET_ATTITUDE_FILTER, field_type_attitude_filter);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_set_ancillary_attitude_filter");
        return ERROR;
    }

    /* write the data into the file using the subroutine for ancillary data */
    return ias_l0r_anc_write(l0r, index, count, attitude_filter,
       GROUP_NAME_SPACECRAFT"/"GROUP_NAME_ACS"/"TABLE_NAME_ATTITUDE_FILTER,
       IAS_L0R_ATTITUDE_FILTER_SIZE, OFFSET_ATTITUDE_FILTER,
       IAS_L0R_ATTITUDE_FILTER_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_append_ancillary_ephemeris

 PURPOSE: Appends the specified data to the applicable tables

 RETURNS: SUCCESS- The data was append
          ERROR- The data could not be appended
******************************************************************************/
int ias_l0r_append_ancillary_ephemeris
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int count, /* I: number of items */
    const IAS_L0R_EPHEMERIS *ephemeris /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary<= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_append_ancillary_ephemeris");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT,
        TABLE_NAME_EPHEMERIS, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_ephemeris, TABLE_SIZE_EPHEMERIS,
        IAS_L0R_EPHEMERIS_SIZE, FIELD_NAMES_EPHEMERIS,
        OFFSET_EPHEMERIS, field_type_ephemeris);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_append_ancillary_ephemeris");
        return ERROR;
    }

    return ias_l0r_append_ancillary_records(l0r, count, ephemeris,
       GROUP_NAME_SPACECRAFT"/"TABLE_NAME_EPHEMERIS,
       IAS_L0R_EPHEMERIS_SIZE, OFFSET_EPHEMERIS, IAS_L0R_EPHEMERIS_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_ephemeris

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_ephemeris
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const IAS_L0R_EPHEMERIS *ephemeris /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_set_ancillary_ephemeris");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT,
        TABLE_NAME_EPHEMERIS, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_ephemeris, TABLE_SIZE_EPHEMERIS,
        IAS_L0R_EPHEMERIS_SIZE, FIELD_NAMES_EPHEMERIS,
        OFFSET_EPHEMERIS, field_type_ephemeris);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_set_ancillary_ephemeris");
        return ERROR;
    }

    return ias_l0r_anc_write(l0r, index, count, ephemeris,
       GROUP_NAME_SPACECRAFT"/"TABLE_NAME_EPHEMERIS,
       IAS_L0R_EPHEMERIS_SIZE, OFFSET_EPHEMERIS, IAS_L0R_EPHEMERIS_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_append_ancillary_gps_position

 PURPOSE: Appends the specified data to the applicable tables

 RETURNS: SUCCESS- The data was append
          ERROR- The data could not be appended
******************************************************************************/
int ias_l0r_append_ancillary_gps_position
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int count, /* I: number of items */
    const IAS_L0R_GPS_POSITION *gps /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status ==ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_append_ancillary_gps");
        return ERROR;
    }
 
    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT,
        TABLE_NAME_GPS_POSITION, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_gps_position, TABLE_SIZE_GPS_POSITION,
        IAS_L0R_GPS_SIZE, FIELD_NAMES_GPS_POSITION,
        OFFSET_GPS_POSITION, field_type_gps_position);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_append_ancillary_gps");
        return ERROR;
    }

    return ias_l0r_append_ancillary_records(l0r, count, gps,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_GPS_POSITION,
        IAS_L0R_GPS_SIZE, OFFSET_GPS_POSITION, IAS_L0R_GPS_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_gps_position

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_gps_position
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const IAS_L0R_GPS_POSITION *gps /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_set_ancillary_gps_position");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT,
        TABLE_NAME_GPS_POSITION, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_gps_position, TABLE_SIZE_GPS_POSITION,
        IAS_L0R_GPS_SIZE, FIELD_NAMES_GPS_POSITION,
        OFFSET_GPS_POSITION, field_type_gps_position);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_set_ancillary_gps_position");
        return ERROR;
    }

    return ias_l0r_anc_write(l0r, index, count, gps,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_GPS_POSITION,
        IAS_L0R_GPS_SIZE, OFFSET_GPS_POSITION, IAS_L0R_GPS_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_gps_range

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_gps_range
(
    L0RIO *l0r, /* structure used with the L0R data */
    const int index, /* I: first record */ /* index to begin the write at */
    const int count, /* number of items to be written */
    const IAS_L0R_GPS_RANGE *gps /* data to write */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_set_ancillary_gps_range");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT,
        TABLE_NAME_GPS_RANGE, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_gps_range, TABLE_SIZE_GPS_RANGE,
        IAS_L0R_GPS_RANGE_SIZE, FIELD_NAMES_GPS_RANGE,
        OFFSET_GPS_RANGE, field_type_gps_range);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_set_ancillary_gps");
        return ERROR;
    }

    return ias_l0r_anc_write(l0r, index, count, gps,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_GPS_RANGE,
        IAS_L0R_GPS_RANGE_SIZE, OFFSET_GPS_RANGE,
        IAS_L0R_GPS_RANGE_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_append_ancillary_imu

 PURPOSE: Appends the specified data to the applicable tables

 RETURNS: SUCCESS- The data was append
          ERROR- The data could not be appended
******************************************************************************/
int ias_l0r_append_ancillary_imu
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int count, /* I: number of items */
    const IAS_L0R_IMU *imu /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_append_ancillary_imu");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU,
        TABLE_NAME_IMU, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_imu, TABLE_SIZE_IMU,
        IAS_L0R_IMU_SIZE, field_names_imu,
        OFFSET_IMU, field_type_imu);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Failed to establish table"
            " in ias_l0r_append_ancillary_imu");
        return ERROR;
    }

    return ias_l0r_append_ancillary_records(l0r, count, imu,
       GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU"/"TABLE_NAME_IMU,
       IAS_L0R_IMU_SIZE, OFFSET_IMU, IAS_L0R_IMU_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_imu

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_imu
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const IAS_L0R_IMU *imu /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;

    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_set_ancillary_imu");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU,
        TABLE_NAME_IMU, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_imu, TABLE_SIZE_IMU,
        IAS_L0R_IMU_SIZE, field_names_imu,
        OFFSET_IMU, field_type_imu);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_set_ancillary_imu");
        return ERROR;
    }

    return ias_l0r_anc_write(l0r, index, count, imu,
       GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU"/"TABLE_NAME_IMU,
       IAS_L0R_IMU_SIZE, OFFSET_IMU, IAS_L0R_IMU_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_append_ancillary_imu_latency

 PURPOSE: Appends the specified data to the applicable tables

 RETURNS: SUCCESS- The data was append
          ERROR- The data could not be appended
******************************************************************************/
int ias_l0r_append_ancillary_imu_latency
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int count, /* I: number of items */
    const IAS_L0R_IMU_LATENCY *imu_latency /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_append_ancillary_imu_latency");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU,
        TABLE_NAME_IMU_LATENCY, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_imu_latency, TABLE_SIZE_IMU_LATENCY,
        IAS_L0R_IMU_LATENCY_SIZE, field_names_imu_latency,
        OFFSET_IMU_LATENCY, field_type_imu_latency);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Failed to establish table"
            " in ias_l0r_append_ancillary_imu_latency");
        return ERROR;
    }

    return ias_l0r_append_ancillary_records(l0r, count, imu_latency,
       GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU"/"TABLE_NAME_IMU_LATENCY,
       IAS_L0R_IMU_LATENCY_SIZE, OFFSET_IMU_LATENCY, IAS_L0R_IMU_LATENCY_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_imu_latency

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_imu_latency
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const IAS_L0R_IMU_LATENCY *imu_latency /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;

    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_set_ancillary_imu_latency");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU,
        TABLE_NAME_IMU_LATENCY, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_imu_latency, TABLE_SIZE_IMU_LATENCY,
        IAS_L0R_IMU_LATENCY_SIZE, field_names_imu_latency,
        OFFSET_IMU_LATENCY, field_type_imu_latency);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_set_ancillary_imu_latency");
        return ERROR;
    }

    return ias_l0r_anc_write(l0r, index, count, imu_latency,
       GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU"/"TABLE_NAME_IMU_LATENCY,
       IAS_L0R_IMU_LATENCY_SIZE, OFFSET_IMU_LATENCY, IAS_L0R_IMU_LATENCY_SIZES);
}


/******************************************************************************
 NAME: ias_l0r_append_ancillary_oli_telemetry

 PURPOSE: Appends the specified data to the applicable tables

 RETURNS: SUCCESS- The data was append
          ERROR- The data could not be appended
******************************************************************************/
int ias_l0r_append_ancillary_oli_telemetry
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int count, /* I: number of items */
    const IAS_L0R_OLI_TELEMETRY *telemetry /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    herr_t status;

    /* establish access to the file */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary == 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish ancillary file access");
        return ERROR;
    }

    /* establish the Telemetry Group 3 structure in the file */
    status = ias_l0r_hdf_establish_table(
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI,
        TABLE_NAME_OLI_TELEMETRY_GROUP_3, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_telemetry_group_3,
        TABLE_SIZE_TELEMETRY_GROUP_3,
        IAS_L0R_OLI_TELEMETRY_GROUP_3_SIZE, FIELD_NAMES_TELEMETRY_GROUP_3,
        OFFSET_TELEMETRY_GROUP_3, field_type_telemetry_group_3);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table: %s",
            TABLE_NAME_OLI_TELEMETRY_GROUP_3);
        return ERROR;
    }

    /* establish the Telemetry Group 4 structure in the file */
    status = ias_l0r_hdf_establish_table(
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI,
        TABLE_NAME_OLI_TELEMETRY_GROUP_4, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_telemetry_group_4,
        TABLE_SIZE_TELEMETRY_GROUP_4,
        IAS_L0R_OLI_TELEMETRY_GROUP_4_SIZE, FIELD_NAMES_PAYLOAD_OLI_GROUP_4,
        OFFSET_PAYLOAD_OLI_GROUP_4, field_type_telemetry_oli_group_4);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table: %s",
                 TABLE_NAME_OLI_TELEMETRY_GROUP_4);
        return ERROR;
    }

    /* establish the Telemetry Group 5 structure in the file */
    status = ias_l0r_hdf_establish_table(
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI,
        TABLE_NAME_OLI_TELEMETRY_GROUP_5, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_telemetry_group_5, TABLE_SIZE_TELEMETRY_GROUP_5,
        IAS_L0R_OLI_TELEMETRY_GROUP_5_SIZE, FIELD_NAMES_PAYLOAD_OLI_GROUP_5,
        OFFSET_PAYLOAD_OLI_GROUP_5, field_type_telemetry_oli_group_5);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table: %s",
             TABLE_NAME_OLI_TELEMETRY_GROUP_5);
        return ERROR;
    }

    /* Append the Group 3 records */
    status = ias_l0r_append_ancillary_records(l0r, count,
        &telemetry->oli_telemetry_header_3,
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI"/"
        TABLE_NAME_OLI_TELEMETRY_GROUP_3,
        IAS_L0R_OLI_TELEMETRY_GROUP_3_SIZE, OFFSET_TELEMETRY_GROUP_3,
        IAS_L0R_OLI_TELEMETRY_GROUP_3_SIZES);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error appending table: %s",
        TABLE_NAME_OLI_TELEMETRY_GROUP_3);
        return ERROR;
    }

    /* Append the Group 4 records */
    status = ias_l0r_append_ancillary_records(l0r, count,
        &telemetry->oli_telemetry_header_4,
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI"/"
        TABLE_NAME_OLI_TELEMETRY_GROUP_4,
        IAS_L0R_OLI_TELEMETRY_GROUP_4_SIZE, OFFSET_PAYLOAD_OLI_GROUP_4,
        IAS_L0R_OLI_TELEMETRY_GROUP_4_SIZES);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error appending table: %s",
        TABLE_NAME_OLI_TELEMETRY_GROUP_4);
        return ERROR;
    }

    /* Append the Group 5 records */
    status = ias_l0r_append_ancillary_records(l0r, count,
        &telemetry->oli_telemetry_header_5,
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI"/"
        TABLE_NAME_OLI_TELEMETRY_GROUP_5,
        IAS_L0R_OLI_TELEMETRY_GROUP_5_SIZE, OFFSET_PAYLOAD_OLI_GROUP_5,
        IAS_L0R_OLI_TELEMETRY_GROUP_5_SIZES);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error appending table: %s",
            TABLE_NAME_OLI_TELEMETRY_GROUP_5);
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_append_ancillary_tirs_telemetry

 PURPOSE: Appends the specified data to the applicable tables

 RETURNS: SUCCESS- The data was append
          ERROR- The data could not be appended
******************************************************************************/
int ias_l0r_append_ancillary_tirs_telemetry
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int count, /* I: number of items */
    const IAS_L0R_TIRS_TELEMETRY *telemetry /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    herr_t status;

    /* establish access to the file */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary == 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish ancillary file");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_TIRS,
        TABLE_NAME_TIRS_TELEMETRY, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_tirs_telemetry, TABLE_SIZE_TIRS_TELEMETRY,
        IAS_L0R_TIRS_TELEMETRY_SIZE, FIELD_NAMES_TIRS_TELEMETRY,
        OFFSET_TIRS_TELEMETRY, field_type_tirs_telemetry);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table: %s",
         TABLE_NAME_TIRS_TELEMETRY);
        return ERROR;
    }

    return ias_l0r_append_ancillary_records(l0r, count, telemetry,
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_TIRS"/"TABLE_NAME_TIRS_TELEMETRY,
        IAS_L0R_TIRS_TELEMETRY_SIZE, OFFSET_TIRS_TELEMETRY,
        IAS_L0R_TIRS_TELEMETRY_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_oli_telemetry

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_oli_telemetry
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const IAS_L0R_OLI_TELEMETRY *telemetry /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;
    int i = 0;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary == 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish ancillary file");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI,
        TABLE_NAME_OLI_TELEMETRY_GROUP_3, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_telemetry_group_3,
        TABLE_SIZE_TELEMETRY_GROUP_3,
        IAS_L0R_OLI_TELEMETRY_GROUP_3_SIZE, FIELD_NAMES_TELEMETRY_GROUP_3,
        OFFSET_TELEMETRY_GROUP_3, field_type_telemetry_group_3);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table: %s",
            TABLE_NAME_OLI_TELEMETRY_GROUP_3);
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI,
        TABLE_NAME_OLI_TELEMETRY_GROUP_4, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_telemetry_group_4,
            TABLE_SIZE_TELEMETRY_GROUP_4,
        IAS_L0R_OLI_TELEMETRY_GROUP_4_SIZE, FIELD_NAMES_PAYLOAD_OLI_GROUP_4,
        OFFSET_PAYLOAD_OLI_GROUP_4, field_type_telemetry_oli_group_4);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table: %s",
                TABLE_NAME_OLI_TELEMETRY_GROUP_4);
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI,
        TABLE_NAME_OLI_TELEMETRY_GROUP_5, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_telemetry_group_5, TABLE_SIZE_TELEMETRY_GROUP_5,
        IAS_L0R_OLI_TELEMETRY_GROUP_5_SIZE, FIELD_NAMES_PAYLOAD_OLI_GROUP_5,
        OFFSET_PAYLOAD_OLI_GROUP_5, field_type_telemetry_oli_group_5);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table: %s",
                    TABLE_NAME_OLI_TELEMETRY_GROUP_5);
        return ERROR;
    }

    /*  We need to write the records one at a time due to there
        unique structure */
    for (i = 0; i < count; i++)
    {

        /* Write the Telemetry Group 3 data */
        status = ias_l0r_anc_write(l0r, index + i, 1,
                 &telemetry[i].oli_telemetry_header_3,
            GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI"/"
            TABLE_NAME_OLI_TELEMETRY_GROUP_3,
            IAS_L0R_OLI_TELEMETRY_GROUP_3_SIZE,
            OFFSET_TELEMETRY_GROUP_3, IAS_L0R_OLI_TELEMETRY_GROUP_3_SIZES);
        if (status == ERROR)
        {
            IAS_LOG_ERROR("Error trying to write table: %s",
                 TABLE_NAME_OLI_TELEMETRY_GROUP_3);
            return ERROR;
        }
    
        /* Write the Telemetry Group 4 data */
        status = ias_l0r_anc_write(l0r, index + i, 1,
                 &telemetry[i].oli_telemetry_header_4,
            GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI"/"
            TABLE_NAME_OLI_TELEMETRY_GROUP_4,
            IAS_L0R_OLI_TELEMETRY_GROUP_4_SIZE,
                OFFSET_PAYLOAD_OLI_GROUP_4,
                IAS_L0R_OLI_TELEMETRY_GROUP_4_SIZES);
        if (status == ERROR)
        {
            IAS_LOG_ERROR("Error trying to write table: %s",
                TABLE_NAME_OLI_TELEMETRY_GROUP_4);
            return ERROR;
        }
      
        /* Write the Telemetry Group 5 data */
        status = ias_l0r_anc_write(l0r, index + i, 1,
             &telemetry[i].oli_telemetry_header_5,
             GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI"/"
             TABLE_NAME_OLI_TELEMETRY_GROUP_5,
             IAS_L0R_OLI_TELEMETRY_GROUP_5_SIZE,
             OFFSET_PAYLOAD_OLI_GROUP_5, IAS_L0R_OLI_TELEMETRY_GROUP_5_SIZES);
        if (status == ERROR)
        {
            IAS_LOG_ERROR("Error trying to write table: %s",
                TABLE_NAME_OLI_TELEMETRY_GROUP_5);
            return ERROR;
        }
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_tirs_telemetry

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_tirs_telemetry
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const IAS_L0R_TIRS_TELEMETRY *telemetry /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary == 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish ancillary file");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_TIRS,
        TABLE_NAME_TIRS_TELEMETRY, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_tirs_telemetry, TABLE_SIZE_TIRS_TELEMETRY,
        IAS_L0R_TIRS_TELEMETRY_SIZE, FIELD_NAMES_TIRS_TELEMETRY,
        OFFSET_TIRS_TELEMETRY, field_type_tirs_telemetry);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table: %s",
            TABLE_NAME_TIRS_TELEMETRY);
        return ERROR;
    }

    return ias_l0r_anc_write(l0r, index, count, telemetry,
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_TIRS"/"TABLE_NAME_TIRS_TELEMETRY,
        IAS_L0R_TIRS_TELEMETRY_SIZE,
        OFFSET_TIRS_TELEMETRY, IAS_L0R_TIRS_TELEMETRY_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_append_ancillary_star_tracker_centroid

 PURPOSE: Appends the specified data to the applicable tables

 RETURNS: SUCCESS- The data was append
          ERROR- The data could not be appended
******************************************************************************/
int ias_l0r_append_ancillary_star_tracker_centroid
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int count, /* I: number of items */
    /* I: Data buffer */
    const IAS_L0R_STAR_TRACKER_CENTROID *star_tracker_centroid
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_append_ancillary_st_centroid");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table( GROUP_NAME_SPACECRAFT,
        TABLE_NAME_STAR_TRACKER_CENTROID, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_star_tracker_centroid,
        TABLE_SIZE_STAR_TRACKER_CENTROID,
        IAS_L0R_STAR_TRACKER_CENTROID_SIZE, field_names_star_tracker_centroid,
        OFFSET_STAR_TRACKER_CENTROID, field_type_star_tracker_centroid);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_append_ancillary_st_centroid");
        return ERROR;
    }

    return ias_l0r_append_ancillary_records(l0r, count, star_tracker_centroid,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_STAR_TRACKER_CENTROID,
        IAS_L0R_STAR_TRACKER_CENTROID_SIZE, OFFSET_STAR_TRACKER_CENTROID,
        IAS_L0R_STAR_TRACKER_CENTROID_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_star_tracker_centroid

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_star_tracker_centroid
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const IAS_L0R_STAR_TRACKER_CENTROID *st_centroid /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_set_ancillary_st_centroid");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table( GROUP_NAME_SPACECRAFT,
        TABLE_NAME_STAR_TRACKER_CENTROID, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_star_tracker_centroid,
        TABLE_SIZE_STAR_TRACKER_CENTROID,
        IAS_L0R_STAR_TRACKER_CENTROID_SIZE,
        field_names_star_tracker_centroid,
        OFFSET_STAR_TRACKER_CENTROID, field_type_star_tracker_centroid);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_set_ancillary_st_centroid");
        return ERROR;
    }

    return ias_l0r_anc_write(l0r, index, count, st_centroid,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_STAR_TRACKER_CENTROID,
        IAS_L0R_STAR_TRACKER_CENTROID_SIZE, OFFSET_STAR_TRACKER_CENTROID,
        IAS_L0R_STAR_TRACKER_CENTROID_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_append_ancillary_star_tracker_quaternion

 PURPOSE: Appends the specified data to the applicable tables

 RETURNS: SUCCESS- The data was append
          ERROR- The data could not be appended
******************************************************************************/
int ias_l0r_append_ancillary_star_tracker_quaternion
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int count, /* I: number of items */
    /* I: Data buffer */
    const IAS_L0R_STAR_TRACKER_QUATERNION *star_tracker_quaternion
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_append_ancillary_st_quaternion");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table( GROUP_NAME_SPACECRAFT,
        TABLE_NAME_STAR_TRACKER_QUATERNION, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_star_tracker_quaternion,
        TABLE_SIZE_STAR_TRACKER_QUATERNION,
        IAS_L0R_STAR_TRACKER_QUATERNION_SIZE,
        field_names_star_tracker_quaternion,
        OFFSET_STAR_TRACKER_QUATERNION, field_type_star_tracker_quaternion);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_append_ancillary_st_quaternion");
        return ERROR;
    }

    return ias_l0r_append_ancillary_records(l0r, count, star_tracker_quaternion,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_STAR_TRACKER_QUATERNION,
        IAS_L0R_STAR_TRACKER_QUATERNION_SIZE, OFFSET_STAR_TRACKER_QUATERNION,
        IAS_L0R_STAR_TRACKER_QUATERNION_SIZES);
}


/******************************************************************************
 NAME: ias_l0r_set_ancillary_star_tracker_quaternion

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_star_tracker_quaternion
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const IAS_L0R_STAR_TRACKER_QUATERNION *st_quaternion /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_set_ancillary_st_quaternion");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_SPACECRAFT,
        TABLE_NAME_STAR_TRACKER_QUATERNION, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_star_tracker_quaternion,
        TABLE_SIZE_STAR_TRACKER_QUATERNION,
        IAS_L0R_STAR_TRACKER_QUATERNION_SIZE,
        field_names_star_tracker_quaternion,
        OFFSET_STAR_TRACKER_QUATERNION, field_type_star_tracker_quaternion);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_set_ancillary_st_quaternion");
        return ERROR;
    }

    status = ias_l0r_anc_write(l0r, index, count, st_quaternion,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_STAR_TRACKER_QUATERNION,
        IAS_L0R_STAR_TRACKER_QUATERNION_SIZE, OFFSET_STAR_TRACKER_QUATERNION,
        IAS_L0R_STAR_TRACKER_QUATERNION_SIZES);

    return status;
}

/******************************************************************************
 NAME: ias_l0r_append_ancillary_temperatures_gyro

 PURPOSE: Appends the specified data to the applicable tables

 RETURNS: SUCCESS- The data was append
          ERROR- The data could not be appended
******************************************************************************/
int ias_l0r_append_ancillary_temperatures_gyro
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int count, /* I: number of items */
    const IAS_L0R_GYRO_TEMPERATURE *temperatures_gyro /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_append_ancillary_temperatures_gyro");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_TEMPERATURES,
        TABLE_NAME_TEMPERATURES_GYRO, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_temperatures_gyro,
        TABLE_SIZE_TEMPERATURES_GYRO,
        IAS_L0R_GYRO_TEMPERATURE_SIZE, field_names_temperatures_gyro,
        OFFSET_GYRO_TEMPERATURE, field_type_temperatures_gyro);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_append_ancillary_temperatures_gyro");
        return ERROR;
    }

    return ias_l0r_append_ancillary_records(l0r, count, temperatures_gyro,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_TEMPERATURES"/"
        TABLE_NAME_TEMPERATURES_GYRO,
        IAS_L0R_GYRO_TEMPERATURE_SIZE, OFFSET_GYRO_TEMPERATURE,
        IAS_L0R_GYRO_TEMPERATURE_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_temperatures_gyro

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_temperatures_gyro
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const IAS_L0R_GYRO_TEMPERATURE *temperatures_gyro /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_set_ancillary_temperatures_gyro");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table( GROUP_NAME_SPACECRAFT"/"
        GROUP_NAME_TEMPERATURES,
        TABLE_NAME_TEMPERATURES_GYRO, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_temperatures_gyro,
        TABLE_SIZE_TEMPERATURES_GYRO,
        IAS_L0R_GYRO_TEMPERATURE_SIZE,
        field_names_temperatures_gyro,
        OFFSET_GYRO_TEMPERATURE, field_type_temperatures_gyro);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_set_ancillary_temperatures_gyro");
        return ERROR;
    }

    return ias_l0r_anc_write(l0r, index, count, temperatures_gyro,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_TEMPERATURES"/"
        TABLE_NAME_TEMPERATURES_GYRO,
        IAS_L0R_GYRO_TEMPERATURE_SIZE, OFFSET_GYRO_TEMPERATURE,
        IAS_L0R_GYRO_TEMPERATURE_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_append_ancillary_temperatures_oli_tirs

 PURPOSE: Appends the specified data to the applicable tables

 RETURNS: SUCCESS- The data was append
          ERROR- The data could not be appended
******************************************************************************/
int ias_l0r_append_ancillary_temperatures_oli_tirs
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int count, /* I: number of items */
    const IAS_L0R_OLI_TIRS_TEMPERATURE *temperatures_oli /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_append_ancillary_temperatures_oli_tirs");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_TEMPERATURES,
        TABLE_NAME_TEMPERATURES_OLI_TIRS, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_temperatures_oli_tirs,
        TABLE_SIZE_TEMPERATURES_OLI_TIRS,
        IAS_L0R_OLI_TIRS_TEMPERATURE_SIZE, field_names_temperatures_oli_tirs,
        OFFSET_OLI_TIRS_TEMPERATURE, field_type_temperatures_oli_tirs);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_append_ancillary_temperatures_oli_tirs");
        return ERROR;
    }

    return ias_l0r_append_ancillary_records(l0r, count, temperatures_oli,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_TEMPERATURES"/"
        TABLE_NAME_TEMPERATURES_OLI_TIRS,
        IAS_L0R_OLI_TIRS_TEMPERATURE_SIZE, OFFSET_OLI_TIRS_TEMPERATURE,
        IAS_L0R_OLI_TIRS_TEMPERATURE_SIZES);
}

/******************************************************************************
 NAME: ias_l0r_set_ancillary_temperatures_oli_tirs

 PURPOSE: Writes the specified data to the applicable tables

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_ancillary_temperatures_oli_tirs
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const IAS_L0R_OLI_TIRS_TEMPERATURE *temperatures_oli_tirs
        /* I: Data buffer */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =   l0r;
    int status;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_ancillary_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_ancillary <= 0)
    {
        /* establish file failed */
        IAS_LOG_ERROR("Error trying to establish file in "
            "ias_l0r_set_ancillary_temperatures_oli_tirs");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table( GROUP_NAME_SPACECRAFT"/"
        GROUP_NAME_TEMPERATURES,
        TABLE_NAME_TEMPERATURES_OLI_TIRS, hdfio_ptr->file_id_ancillary,
        &hdfio_ptr->table_id_temperatures_oli_tirs,
        TABLE_SIZE_TEMPERATURES_OLI_TIRS,
        IAS_L0R_OLI_TIRS_TEMPERATURE_SIZE,
        field_names_temperatures_oli_tirs,
        OFFSET_OLI_TIRS_TEMPERATURE, field_type_temperatures_oli_tirs);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error trying to establish table in "
            "ias_l0r_set_ancillary_temperatures_oli_tirs");
        return ERROR;
    }

    return ias_l0r_anc_write(l0r, index, count, temperatures_oli_tirs,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_TEMPERATURES"/"
        TABLE_NAME_TEMPERATURES_OLI_TIRS,
        IAS_L0R_OLI_TIRS_TEMPERATURE_SIZE, OFFSET_OLI_TIRS_TEMPERATURE,
        IAS_L0R_OLI_TIRS_TEMPERATURE_SIZES);
}

/*******************************************************************************
*Compare routines
*    NOTE:  These routines are mostly for testing purposes
*******************************************************************************/
/******************************************************************************
 NAME: ias_l0r_compare_ancillary_attitude

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_attitude
(
    const IAS_L0R_ATTITUDE *attitude, /* I: 1st struct to compare */
    const IAS_L0R_ATTITUDE *attitude2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures
         are the same, 1 or -1 otherwise */
)
{
    if (attitude == NULL ||
        attitude2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    /* if the structures are the same */
    if (attitude == attitude2)
    {
        return SUCCESS;
    }

    /* If strings are added to the structure, elements need to be *
    * compared individually. For now just compare the block of memory */
    *different_elements = memcmp(attitude, attitude2, sizeof(IAS_L0R_ATTITUDE));
 
    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_compare_ancillary_attitude_filter_states

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_attitude_filter_states
(
    /* I: 1st struct to compare */
    const IAS_L0R_ATTITUDE_FILTER_STATES *attitude_filter,
    /* I: 2nd struct to compare */
    const IAS_L0R_ATTITUDE_FILTER_STATES *attitude_filter2,
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (attitude_filter == NULL ||
        attitude_filter2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    /* if the structures are the same */
    if (attitude_filter == attitude_filter2)
    {
        return SUCCESS;
    }

    /* If strings are added to the structure, elements need to be *
    * compared individually. For now just compare the block of memory */
    *different_elements = memcmp(attitude_filter,
        attitude_filter2, sizeof(IAS_L0R_ATTITUDE_FILTER_STATES));

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_compare_ancillary_ephemeris

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_ephemeris
(
    const IAS_L0R_EPHEMERIS *ephemeris, /* I: 1st struct to compare */
    const IAS_L0R_EPHEMERIS *ephemeris2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (ephemeris == NULL ||
        ephemeris2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    /* if the structures are the same */
    if (ephemeris == ephemeris2)
    {
        return SUCCESS;
    }

    /* If strings are added to the structure, elements need to be *
    * compared individually. For now just compare the block of memory */
    *different_elements = memcmp(ephemeris,
        ephemeris2, sizeof(IAS_L0R_EPHEMERIS));

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_compare_ancillary_gps_position

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_gps_position
(
    const IAS_L0R_GPS_POSITION *gps_position, /* I: 1st struct to compare */
    const IAS_L0R_GPS_POSITION *gps_position2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (gps_position == NULL ||
        gps_position2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    /* if the structures are the same */
    if (gps_position == gps_position2)
    {
        return SUCCESS;
    }

    *different_elements = memcmp(gps_position, gps_position2,
        sizeof(IAS_L0R_GPS_POSITION));

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_compare_ancillary_gps_range

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_gps_range
(
    const IAS_L0R_GPS_RANGE *gps_range, /* I: 1st struct to compare */
    const IAS_L0R_GPS_RANGE *gps_range2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (gps_range == NULL ||
        gps_range2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    /* if the structures are the same */
    if (gps_range == gps_range2)
    {
        return SUCCESS;
    }

    *different_elements = memcmp(gps_range,
        gps_range2, sizeof(IAS_L0R_GPS_RANGE));

    return SUCCESS;
}


/******************************************************************************
 NAME: ias_l0r_compare_ancillary_imu

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_imu
(
    const IAS_L0R_IMU *imu_data, /* I: 1st struct to compare */
    const IAS_L0R_IMU *imu_data2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (imu_data == NULL ||
        imu_data2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    /* if the structures are the same */
    if (imu_data == imu_data2)
    {
        return SUCCESS;
    }

    *different_elements = memcmp(imu_data, imu_data2, sizeof(IAS_L0R_IMU));

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_compare_ancillary_imu_latency

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_imu_latency
(
    const IAS_L0R_IMU_LATENCY *imu_latency_data, /* I: 1st struct to compare */
    const IAS_L0R_IMU_LATENCY *imu_latency_data2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (imu_latency_data == NULL ||
        imu_latency_data2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    /* if the structures are the same */
    if (imu_latency_data == imu_latency_data2)
    {
        return SUCCESS;
    }

    *different_elements = memcmp(imu_latency_data, imu_latency_data2,
        sizeof(IAS_L0R_IMU_LATENCY));

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_compare_ancillary_oli_telemetry

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_oli_telemetry
(
    const IAS_L0R_OLI_TELEMETRY *telemetry_data, /* I: 1st struct to compare */
    const IAS_L0R_OLI_TELEMETRY *telemetry_data2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (telemetry_data == NULL ||
        telemetry_data2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    /* if the structures are the same */
    if (telemetry_data == telemetry_data2)
    {
        return SUCCESS;
    }

    *different_elements = memcmp(telemetry_data,
         telemetry_data2, sizeof(IAS_L0R_OLI_TELEMETRY));

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_compare_ancillary_tirs_telemetry

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_tirs_telemetry
(
    const IAS_L0R_TIRS_TELEMETRY *telemetry_data, /* I: 1st struct to compare */
    const IAS_L0R_TIRS_TELEMETRY *telemetry_data2,
     /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (telemetry_data == NULL ||
        telemetry_data2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    /* if the structures are the same */
    if (telemetry_data == telemetry_data2)
    {
        return SUCCESS;
    }

    *different_elements = memcmp(telemetry_data,
         telemetry_data2, sizeof(IAS_L0R_TIRS_TELEMETRY));

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_compare_ancillary_star_tracker_centroid

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_star_tracker_centroid
(
    /* I: 1st struct to compare */
    const IAS_L0R_STAR_TRACKER_CENTROID *centroid,
    /* I: 2nd struct to compare */
    const IAS_L0R_STAR_TRACKER_CENTROID *centroid2,
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (centroid == NULL ||
        centroid2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    if (centroid == centroid2)
    {
        return SUCCESS;
    }

    *different_elements = memcmp(centroid, centroid2,
        sizeof(IAS_L0R_STAR_TRACKER_CENTROID));
 
    return SUCCESS;
}
 
/******************************************************************************
 NAME: ias_l0r_compare_ancillary_star_tracker_quaternion

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_star_tracker_quaternion
(
    /* I: 1st struct to compare */
    const IAS_L0R_STAR_TRACKER_QUATERNION *quaternion,
    /* I: 2nd struct to compare */
    const IAS_L0R_STAR_TRACKER_QUATERNION *quaternion2,
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (quaternion == NULL ||
        quaternion2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    if (quaternion == quaternion2)
    {
        return SUCCESS;
    }

    *different_elements = memcmp(quaternion, quaternion2,
        sizeof(IAS_L0R_STAR_TRACKER_QUATERNION));

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_compare_ancillary_gyro_temperature

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_gyro_temperature
(
    /* I: 1st struct to compare */
    const IAS_L0R_GYRO_TEMPERATURE *temperature_gyro,
    /* I: 2nd struct to compare */
    const IAS_L0R_GYRO_TEMPERATURE *temperature_gyro2,
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (temperature_gyro == NULL ||
        temperature_gyro2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    if (temperature_gyro == temperature_gyro2)
    {
        return SUCCESS;
    }

    *different_elements = memcmp(temperature_gyro, temperature_gyro2,
        sizeof(IAS_L0R_GYRO_TEMPERATURE));

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_compare_ancillary_oli_tirs_temperature

 PURPOSE: Determines if any elements in the structures differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_compare_ancillary_oli_tirs_temperature
(
    /* I: 1st struct to compare */
    const IAS_L0R_OLI_TIRS_TEMPERATURE *temperature_oli_tirs,
    /* I: 2nd struct to compare */
    const IAS_L0R_OLI_TIRS_TEMPERATURE *temperature_oli_tirs2,
    int *different_elements /* O:  Contains 0 if the structures are
        the same, 1 or -1 otherwise */
)
{
    if (temperature_oli_tirs == NULL ||
        temperature_oli_tirs2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;

    if (temperature_oli_tirs == temperature_oli_tirs2)
    {
        return SUCCESS;
    }

    *different_elements = memcmp(temperature_oli_tirs, temperature_oli_tirs2,
        sizeof(IAS_L0R_OLI_TIRS_TEMPERATURE));

    return SUCCESS;
}

/*******************************************************************************
* Ancillary data read routines
*******************************************************************************/
/* subroutine used in reading ancillary data */
static int ias_l0r_anc_read
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    const char *table_name, /* I: Table to read */
    const size_t size, /* I: Size in bytes of the structure to read into */
    const size_t *offset, /* I: List of offsets for each member */
    const size_t *size_list, /* I: List of sizes for each member */
    void *buffer /* O: Data buffer */
)
{
   herr_t hdf_status;
   int temp_size;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr =  l0r;

    if (hdfio_ptr->access_mode_ancillary < 0)
    {
        IAS_LOG_ERROR("Ancillary data is not open");
        return ERROR;
    }

    hdf_status = ias_l0r_establish_ancillary_file(hdfio_ptr, FALSE);
    if (hdf_status < 0 || hdfio_ptr->file_id_ancillary <= 0)
    {
        IAS_LOG_ERROR("Error establish file for read");
        return ERROR;
    }

    /* verify there are an adequate number of records for the read */
    if (ias_l0r_get_anc_table_records_count(hdfio_ptr, table_name,
         &temp_size) == ERROR)
    {
        IAS_LOG_ERROR("Unable to get size of %s", table_name);
        return ERROR;
    }
    if (temp_size < (index + count) )
    {
        IAS_LOG_ERROR("Insufficient records to read from %s"
            " Attempting to read %d "
            "records out of %d from %d", table_name, count, temp_size, index);
        return ERROR;
    }

    hdf_status = H5TBread_records(hdfio_ptr->file_id_ancillary,
        table_name, index, count, size,
        offset, size_list, buffer);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Error reading from %s", table_name);
        return ERROR;
    }

    return SUCCESS;
}

/*******************************************************************************
*ias_l0r_get_ancillary_X routines
*  retrieve data from the hdf file into the struct provided
*******************************************************************************/
/*******************************************************************************
 NAME: ias_l0r_get_ancillary_attitude
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_attitude
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    IAS_L0R_ATTITUDE *buffer /* O: Data buffer */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_ACS"/"TABLE_NAME_ATTITUDE,
        IAS_L0R_ATTITUDE_SIZE,OFFSET_ATTITUDE,
        IAS_L0R_ATTITUDE_SIZES, buffer);
}


/*******************************************************************************
 NAME: ias_l0r_get_ancillary_attitude_records_count
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_attitude_records_count(L0RIO *l0r, int *size)
{
    
   HDFIO *hdfio_ptr = l0r;
   return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_ACS"/"TABLE_NAME_ATTITUDE, size);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_attitude_filter_states
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_attitude_filter_states
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    IAS_L0R_ATTITUDE_FILTER_STATES *buffer /* O: Data buffer */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_ACS"/"TABLE_NAME_ATTITUDE_FILTER,
        IAS_L0R_ATTITUDE_FILTER_SIZE,OFFSET_ATTITUDE_FILTER,
        IAS_L0R_ATTITUDE_FILTER_SIZES, buffer);
}


/*******************************************************************************
 NAME: ias_l0r_get_ancillary_attitude_filter_states_records_count
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_attitude_filter_states_records_count
        (L0RIO *l0r, int *size)
{

   HDFIO *hdfio_ptr = l0r;
   return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_ACS"/"
        TABLE_NAME_ATTITUDE_FILTER, size);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_ephemeris
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_ephemeris
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    IAS_L0R_EPHEMERIS *buffer /* O: Data buffer */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_EPHEMERIS,
        IAS_L0R_EPHEMERIS_SIZE, OFFSET_EPHEMERIS,
        IAS_L0R_EPHEMERIS_SIZES, buffer);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_ephemeris_records_count
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_ephemeris_records_count
(
    L0RIO *l0r,
    int *size
)
{
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_EPHEMERIS, size);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_gps_position
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_gps_position
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    IAS_L0R_GPS_POSITION *buffer /* O: Data buffer */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_GPS_POSITION,
        IAS_L0R_GPS_SIZE, OFFSET_GPS_POSITION,
        IAS_L0R_GPS_SIZES, buffer);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_gps_range
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_gps_range
(
    L0RIO *l0r, /*  I: structure for the file used in I/O */
    const int index, /* I: first record */
    const int count, /* I: number of items */
    IAS_L0R_GPS_RANGE *buffer /* O: Data buffer */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_GPS_RANGE,
        IAS_L0R_GPS_RANGE_SIZE, OFFSET_GPS_RANGE,
        IAS_L0R_GPS_RANGE_SIZES, buffer);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_gps_position_records_count
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_gps_position_records_count
                            (L0RIO *l0r, int *size)
{
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_GPS_POSITION, size);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_gps_range_records_count

 PURPOSE: Gets the specified ancillary data

 RETURNS: SUCCESS- Data was read into the buffer
          ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_gps_range_records_count
                            (L0RIO *l0r, int *size)
{
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_GPS_RANGE, size);
}


/*******************************************************************************
 NAME: ias_l0r_get_ancillary_imu
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_imu
(
    L0RIO *l0r, /* I: structure used with the L0R data  */
    const int index, /* I: first record */ /* I: index to begin read at */
    const int count, /* I: number of items to read */
    IAS_L0R_IMU *imu /* O: buffer for the read data */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU"/"TABLE_NAME_IMU,
        IAS_L0R_IMU_SIZE, OFFSET_IMU,
        IAS_L0R_IMU_SIZES, imu);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_imu_records_count
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_imu_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
)
{
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU"/"TABLE_NAME_IMU, size);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_imu_latency
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_imu_latency
(
    L0RIO *l0r, /* I: structure used with the L0R data  */
    const int index, /* I: first record */ /* I: index to begin read at */
    const int count, /* I: number of items to read */
    IAS_L0R_IMU_LATENCY *imu_latency /* O: buffer for the read data */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU"/"TABLE_NAME_IMU_LATENCY,
        IAS_L0R_IMU_LATENCY_SIZE, OFFSET_IMU_LATENCY,
        IAS_L0R_IMU_LATENCY_SIZES, imu_latency);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_imu_latency_records_count
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_imu_latency_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in this dataset */
)
{
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_IMU"/"TABLE_NAME_IMU_LATENCY, size);
}


/*******************************************************************************
 NAME: ias_l0r_get_ancillary_oli_telemetry
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_oli_telemetry
(
    L0RIO *l0r, /* structure used with the L0R data */
    const int index, /* I: first record */ /* the index to begin the read at */
    const int count, /* I: number of items to read */
    IAS_L0R_OLI_TELEMETRY *telemetry /* O: buffer to hold data read */
)
{
    int i = 0;
    int status = 0;

    /*  We need to read the records one at a time due to there
        unique structure */
    for (i = 0; i < count; i++)
    {
        /* Read from the Telemetry Group 3 table into the buffer */
        status = ias_l0r_anc_read(l0r, index + i, 1,
            GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI"/"
            TABLE_NAME_OLI_TELEMETRY_GROUP_3,
            IAS_L0R_OLI_TELEMETRY_GROUP_3_SIZE,
            OFFSET_TELEMETRY_GROUP_3,
            IAS_L0R_OLI_TELEMETRY_GROUP_3_SIZES,
            &telemetry[i].oli_telemetry_header_3);
        if (status == ERROR)
        {
              IAS_LOG_ERROR("Unable to read table %s",
                             TABLE_NAME_OLI_TELEMETRY_GROUP_3);
              return ERROR;
        }
    
        /* Read from the Telemetry Group 4 table into the buffer */
        status = ias_l0r_anc_read(l0r, index + i, 1,
            GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI"/"
            TABLE_NAME_OLI_TELEMETRY_GROUP_4,
            IAS_L0R_OLI_TELEMETRY_GROUP_4_SIZE,
            OFFSET_PAYLOAD_OLI_GROUP_4,
            IAS_L0R_OLI_TELEMETRY_GROUP_4_SIZES,
            &telemetry[i].oli_telemetry_header_4);
        if (status == ERROR)
        {
              IAS_LOG_ERROR("Unable to read table %s",
                             TABLE_NAME_OLI_TELEMETRY_GROUP_4);
              return ERROR;
        }
    
        /* Read from the Telemetry Group 5 table into the buffer */
        status = ias_l0r_anc_read(l0r, index + i, 1,
            GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI"/"
            TABLE_NAME_OLI_TELEMETRY_GROUP_5,
            IAS_L0R_OLI_TELEMETRY_GROUP_5_SIZE,
            OFFSET_PAYLOAD_OLI_GROUP_5,
            IAS_L0R_OLI_TELEMETRY_GROUP_5_SIZES,
            &telemetry[i].oli_telemetry_header_5);
        if (status == ERROR)
        {
              IAS_LOG_ERROR("Unable to read table %s",
                             TABLE_NAME_OLI_TELEMETRY_GROUP_5);
              return ERROR;
        }
    }

    return SUCCESS;
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_oli_telemetry_records_count
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_oli_telemetry_records_count
(
    L0RIO *l0r, /* structure used with the L0R data */
    int *size          /* number of records in this dataset */
)
{
    HDFIO *hdfio_ptr = l0r;
    int status = 0;
    int group_3_size = 0;
    int group_4_size = 0;
    int group_5_size = 0;
      
     status = ias_l0r_get_anc_table_records_count(hdfio_ptr,
         GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI
         "/"TABLE_NAME_OLI_TELEMETRY_GROUP_3, &group_3_size);
     if (status == ERROR)
         return ERROR;

     status = ias_l0r_get_anc_table_records_count(hdfio_ptr,
         GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI
         "/"TABLE_NAME_OLI_TELEMETRY_GROUP_4, &group_4_size);
     if (status == ERROR)
         return ERROR;

     status = ias_l0r_get_anc_table_records_count(hdfio_ptr,
         GROUP_NAME_TELEMETRY"/"GROUP_NAME_OLI
         "/"TABLE_NAME_OLI_TELEMETRY_GROUP_5, &group_5_size);
     if (status == ERROR)
         return ERROR;

    if (group_3_size == group_4_size && group_4_size == group_5_size)
               *size = group_3_size;
    else
    {
        IAS_LOG_ERROR("OLI Payload tables are not the same size");
        return ERROR;
    }
    return SUCCESS;
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_tirs_telemetry
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_tirs_telemetry
(
    L0RIO *l0r, /* structure used with the L0R data */
    const int index, /* I: first record */ /* the index to begin the read at */
    const int count, /* number of items to read */
    IAS_L0R_TIRS_TELEMETRY *telemetry /* buffer to hold data read */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_TIRS"/"
        TABLE_NAME_TIRS_TELEMETRY,
        IAS_L0R_TIRS_TELEMETRY_SIZE, OFFSET_TIRS_TELEMETRY,
        IAS_L0R_TIRS_TELEMETRY_SIZES, telemetry);
}


/*******************************************************************************
 NAME: ias_l0r_get_ancillary_tirs_telemetry_records_count
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_tirs_telemetry_records_count
(
    L0RIO *l0r, /* structure used with the L0R data */
    int *size          /* structure size */
)
{
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_TELEMETRY"/"GROUP_NAME_TIRS"/"
        TABLE_NAME_TIRS_TELEMETRY, size);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_tirs_ssm
 
 PURPOSE: This is a convenience routine to extract the TIRS scene select mirror
    (SSM) data from the TIRS telemetry blocks since the users of the SSM data
    don't want all the other stuff.  This also allows for later putting the SSM
    data in a separate dataset without affecting users of the data.
 
 RETURNS: SUCCESS- Data was read into the buffer
          ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_tirs_ssm
(
    L0RIO *l0r, /* structure used with the L0R data */
    const int index, /* I: first record */ /* the index to begin the read at */
    const int count, /* number of items to read */
    IAS_L0R_TIRS_SSM_ENCODER *ssm_records /* buffer to hold data read */
)
{
    IAS_L0R_TIRS_TELEMETRY *telemetry;
    int i;

    /* allocate memory for the intermediate buffer */
    telemetry = malloc(count * sizeof(*telemetry));
    if (!telemetry)
    {
        IAS_LOG_ERROR("Error allocating memory for telemetry records");
        return ERROR;
    }

    /* read the telemetry records into the intermediate buffer */
    if (ias_l0r_get_ancillary_tirs_telemetry(l0r, index, count, telemetry)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Error reading telemetry records");
        free(telemetry);
        return ERROR;
    }

    /* copy the SSM information to the ssm records structure */
    for (i = 0; i < count; i++)
    {
        int j;

        ssm_records[i].l0r_time = telemetry[i].l0r_time;
        ssm_records[i].elec_enabled_flags
            = telemetry[i].circuit.elec_enabled_flags;
        ssm_records[i].ssm_mech_mode = telemetry[i].command.ssm_mech_mode;
        ssm_records[i].ssm_position_sel = telemetry[i].command.ssm_position_sel;

        for (j = 0;
                j < IAS_L0R_TIRS_TELEMETRY_SSM_ENCODER_POSITION_SAMPLE_COUNT;
                j++)
        {
            ssm_records[i].encoder_position[j]
                = telemetry[i].command.ssm_encoder_position_sample[j];
        }
    }

    free(telemetry);

    return SUCCESS;
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_tirs_ssm_records_count
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
          ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_tirs_ssm_records_count
(
    L0RIO *l0r, /* structure used with the L0R data */
    int *size   /* structure size */
)
{
    int status;

    /* This is the same as the TIRS telemetry record count */
    status = ias_l0r_get_ancillary_tirs_telemetry_records_count(l0r,
                size);
    return status;
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_star_tracker_centroid
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_star_tracker_centroid
(
    L0RIO *l0r,/* I: structure used with the L0R data */
    const int index, /* I: first record */ /* I: index to begin read at */
    const int count, /* I: number of items to read */
    IAS_L0R_STAR_TRACKER_CENTROID *centroid /* O: buffer for read data */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_STAR_TRACKER_CENTROID,
        IAS_L0R_STAR_TRACKER_CENTROID_SIZE, OFFSET_STAR_TRACKER_CENTROID,
        IAS_L0R_STAR_TRACKER_CENTROID_SIZES, centroid);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_star_tracker_quaternion
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_star_tracker_quaternion
(
    L0RIO *l0r,/* I: structure used with the L0R data */
    const int index, /* I: first record */ /* I: index to begin read at */
    const int count, /* I: number of items to read */
    IAS_L0R_STAR_TRACKER_QUATERNION *quaternion /* O: buffer for read data */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_STAR_TRACKER_QUATERNION,
        IAS_L0R_STAR_TRACKER_QUATERNION_SIZE, OFFSET_STAR_TRACKER_QUATERNION,
        IAS_L0R_STAR_TRACKER_QUATERNION_SIZES, quaternion);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_star_tracker_centroid_records_count
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_star_tracker_centroid_records_count
(
    L0RIO *l0r,
    int *size         
)
{
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_STAR_TRACKER_CENTROID, size);
}

/*******************************************************************************
 NAME: ias_l0r_get_ancillary_star_tracker_quaternion_records_count

 PURPOSE: Gets the specified ancillary data

 RETURNS: SUCCESS- Data was read into the buffer
          ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_star_tracker_quaternion_records_count
(
    L0RIO *l0r,
    int *size         
)
{
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_SPACECRAFT"/"TABLE_NAME_STAR_TRACKER_QUATERNION, size);
}


/******************************************************************************
 NAME: ias_l0r_get_ancillary_gyro_temperature
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_gyro_temperature
(
    L0RIO *l0r, /* I: pointer to structure used in I/O */
    const int index, /* I: first record */ /* I: index to begin read at */
    const int count,/* I: number of items to read */
    IAS_L0R_GYRO_TEMPERATURE *buffer/* O: buffer for read data */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_TEMPERATURES"/"
        TABLE_NAME_TEMPERATURES_GYRO,
        IAS_L0R_GYRO_TEMPERATURE_SIZE, OFFSET_GYRO_TEMPERATURE,
        IAS_L0R_GYRO_TEMPERATURE_SIZES, buffer);
}

/******************************************************************************
 NAME: ias_l0r_get_ancillary_gyro_temperature_records_count
 
 PURPOSE: get the number of lines in the table
 
 RETURNS: SUCCESS: Size determined
          ERROR: Size not able to be determined
******************************************************************************/
int ias_l0r_get_ancillary_gyro_temperature_records_count
(
    L0RIO *l0r, /* I: pointer to structure used in I/O */
    int *size         /* O: number of records in this dataset */
)
{
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_TEMPERATURES"/"
        TABLE_NAME_TEMPERATURES_GYRO, size);
}

/******************************************************************************
 NAME: ias_l0r_get_ancillary_oli_tirs_temperature
 
 PURPOSE: Gets the specified ancillary data
 
 RETURNS: SUCCESS- Data was read into the buffer
           ERROR- Could not read into the buffer
*******************************************************************************/
int ias_l0r_get_ancillary_oli_tirs_temperature
(
    L0RIO *l0r, /* I: pointer to structure used in I/O */
    const int index, /* I: index to begin read at */
    const int count,/* I: number of items to read */
    IAS_L0R_OLI_TIRS_TEMPERATURE *buffer/* O: buffer for read data */
)
{
    return ias_l0r_anc_read(l0r, index, count,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_TEMPERATURES"/"
        TABLE_NAME_TEMPERATURES_OLI_TIRS,
        IAS_L0R_OLI_TIRS_TEMPERATURE_SIZE, OFFSET_OLI_TIRS_TEMPERATURE,
        IAS_L0R_OLI_TIRS_TEMPERATURE_SIZES, buffer);
}

/******************************************************************************
 NAME: ias_l0r_get_ancillary_oli_tirs_temperature_records_count

 PURPOSE: get the number of records in the table
 
 RETURNS: SUCCESS: Size determined
          ERROR: Size not able to be determined
******************************************************************************/
int ias_l0r_get_ancillary_oli_tirs_temperature_records_count
(
    L0RIO *l0r, /* I: pointer to structure used in I/O */
    int *size         /* O: number of records in this dataset */
)
{
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_get_anc_table_records_count(hdfio_ptr,
        GROUP_NAME_SPACECRAFT"/"GROUP_NAME_TEMPERATURES"/"
        TABLE_NAME_TEMPERATURES_OLI_TIRS, size);
}


