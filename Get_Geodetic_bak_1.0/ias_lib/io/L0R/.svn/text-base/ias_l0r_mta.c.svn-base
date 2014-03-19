/*-----------------------------------------------------------------------------

NAME: ias_l0r_mta.c

PURPOSE: Functions for reading and writing metadata contained in the L0R data

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/* project included headers */
#include "ias_l0r.h"
#include "ias_l0r_hdf.h"
#include "ias_logging.h"
#include "ias_types.h"

#define DATASET_NAME_MTA_FILE "File"
#define DATASET_NAME_MTA_INTERVAL "Interval"
#define DATASET_NAME_MTA_SCENE "Scenes"

#define TABLE_NAME_FILE_METADATA "File"
#define TABLE_NAME_INTERVAL_METADATA "Interval"
#define TABLE_NAME_SCENES_METADATA "Scenes"

#define TABLE_SIZE_FILE_METADATA 22
#define TABLE_SIZE_INTERVAL_METADATA 63
#define TABLE_SIZE_SCENES_METADATA 53

/******************************************************************************
 Global variables
******************************************************************************/
int landsat_scene_id_index = 0; /* This variable is set by
    ias_l0r_set_scene_field_type and used by ias_l0r_get_scene_metadata_by_id*/

/******************************************************************************
 private routine prototypes
*******************************************************************************/
static int ias_l0r_mta_establish_file
(
    HDFIO *hdfio_ptr, /* I: Pointer to structure used in I/O */
    int create_if_absent /* I: Create the file if it does not exist */
);

static int ias_l0r_mta_read
(
    HDFIO *hdfio_ptr, /* I: Pointer to structure used in I/O */
    const int index, /* I: First record to read */
    const int count, /* I: Number of records to read */
    const char *table_name, /* I: Table name to read from */
    const size_t size, /* I: Size of the structure which stores the data */
    const size_t *offset, /* I: List of the offsets for each member */
    const size_t *size_list, /* I: List of the sizes of each member */
    void *buffer /* I: Data Buffer */
);

static int ias_l0r_mta_table_records_count
(
    HDFIO *hdfio_ptr, /* I: Pointer to structure used in I/O */
    const char *table, /* I: Table name to read from */
    int *size          /* O: Number of records in the table */
);

int ias_l0r_mta_write
(
    L0RIO *l0r, /* structure for the file used in I/O */
    const int index, /* I: First record to write */
    const int count, /* I: Number of records to write */
    const char *table_name, /* I: Table name to write to */
    const size_t size, /* I: Size of the structure which stores the data */
    const size_t *offset, /* I: List of the offsets for each member */
    const size_t *size_list, /* I: List of the sizes of each member */
    const void *buffer /* I: Data Buffer */
);
 
/*****************************************************************************/
/* File level metadata constants*/
/****************************************************************************/
static const hid_t *field_type_metadata_file = NULL;
static hid_t file_metadata_types[TABLE_SIZE_FILE_METADATA];

static const char *field_names_metadata_file[TABLE_SIZE_FILE_METADATA] =
{
    "ANCILLARY_FILE_NAME", /* IAS_L0R_FILE ancillary_file; */
    "CHECKSUM_FILE_NAME",
    "FILE_NAME_BAND_1",
    "FILE_NAME_BAND_2",
    "FILE_NAME_BAND_3",
    "FILE_NAME_BAND_4",
    "FILE_NAME_BAND_5",
    "FILE_NAME_BAND_6",
    "FILE_NAME_BAND_7",
    "FILE_NAME_BAND_8",
    "FILE_NAME_BAND_9",
    "FILE_NAME_BAND_10",
    "FILE_NAME_BAND_11",
    "FILE_NAME_BAND_12",
    "FILE_NAME_BAND_13",
    "FILE_NAME_BAND_14",
    "FILE_NAME_BAND_15",
    "FILE_NAME_BAND_16",
    "FILE_NAME_BAND_17",
    "FILE_NAME_BAND_18",
    "INTERVAL_FILES",
    "METADATA_FILE_NAME" /* char metadata_file_name */

};
static const IAS_L0R_FILE_METADATA  *oli_l0r_metadata_file_ptr = NULL;
static const size_t IAS_L0R_FILE_METADATA_SIZE =
    sizeof( IAS_L0R_FILE_METADATA );
static const size_t IAS_L0R_FILE_METADATA_SIZES[TABLE_SIZE_FILE_METADATA] =
{
    sizeof( (*oli_l0r_metadata_file_ptr).ancillary_file.name),
    sizeof( (*oli_l0r_metadata_file_ptr).checksum_file.name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[0].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[1].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[2].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[3].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[4].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[5].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[6].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[7].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[8].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[9].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[10].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[11].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[12].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[13].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[14].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[15].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[16].name),
    sizeof( (*oli_l0r_metadata_file_ptr).band_files[17].name),
    sizeof( (*oli_l0r_metadata_file_ptr).interval_files),
    sizeof( (*oli_l0r_metadata_file_ptr).metadata_file.name)

};

static const size_t OFFSET_FILE_METADATA[TABLE_SIZE_FILE_METADATA] =
{
    HOFFSET( IAS_L0R_FILE_METADATA,ancillary_file.name),
    HOFFSET( IAS_L0R_FILE_METADATA,checksum_file.name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[0].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[1].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[2].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[3].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[4].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[5].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[6].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[7].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[8].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[9].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[10].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[11].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[12].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[13].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[14].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[15].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[16].name),
    HOFFSET( IAS_L0R_FILE_METADATA,band_files[17].name),
    HOFFSET( IAS_L0R_FILE_METADATA,interval_files),
    HOFFSET( IAS_L0R_FILE_METADATA,metadata_file.name)
};

/*****************************************************************************/
/* Interval level metadata constants*/
/*****************************************************************************/
static const hid_t *field_type_metadata_interval = NULL;
static hid_t interval_metadata_types[TABLE_SIZE_INTERVAL_METADATA];

static const char *field_names_metadata_interval[TABLE_SIZE_INTERVAL_METADATA] =
{
    "ANCILLARY_START_TIME",
    "ANCILLARY_STOP_TIME",
    "ATTITUDE_POINTS",
    "ATTITUDE_POINTS_MISSING",
    "ATTITUDE_POINTS_REJECTED",
    "COLLECTION_TYPE",
    "CORNER_UL_LAT_OLI",
    "CORNER_UL_LON_OLI",
    "CORNER_UR_LAT_OLI",
    "CORNER_UR_LON_OLI",
    "CORNER_LL_LAT_OLI",
    "CORNER_LL_LON_OLI",
    "CORNER_LR_LAT_OLI",
    "CORNER_LR_LON_OLI",
    "CORNER_UL_LAT_TIRS",
    "CORNER_UL_LON_TIRS",
    "CORNER_UR_LAT_TIRS",
    "CORNER_UR_LON_TIRS",
    "CORNER_LL_LAT_TIRS",
    "CORNER_LL_LON_TIRS",
    "CORNER_LR_LAT_TIRS",
    "CORNER_LR_LON_TIRS",
    "CPF_NAME",
    "CRC_ERRORS_OLI",
    "CRC_ERRORS_TIRS",
    "DATA_TYPE",
    "DATE_ACQUIRED",
    "EPHEMERIS_POINTS",
    "EPHEMERIS_POINTS_MISSING",
    "EPHEMERIS_POINTS_REJECTED",
    "FRAMES_FILLED_OLI",
    "FRAMES_FILLED_TIRS",
    "HOSTNAME",
    "IMAGE_QUALITY_OLI",
    "IMAGE_QUALITY_TIRS",
    "INTERVAL_FRAMES_OLI",
    "INTERVAL_FRAMES_TIRS",
    "INTERVAL_NUMBER",
    "INTERVAL_VERSION",
    "IS_VERSION",
    "LANDSAT_CAL_INTERVAL_ID",
    "LANDSAT_INTERVAL_ID",
    "NADIR_OFFNADIR",
    "QUALITY_ALGORITHM",
    "ROLL_ANGLE",
    "SATELLITE",
    "SENSOR_ID",
    "SPACECRAFT_ID",
    "START_TIME_OLI",
    "START_TIME_TIRS",
    "STATION_ID",
    "STOP_TIME_OLI",
    "STOP_TIME_TIRS",
    "TIME_CODE_ERRORS_OLI",
    "TIME_CODE_ERRORS_TIRS",
    "DETECTOR_MAP_ID_TIRS",
    "WRS_ENDING_ROW",
    "WRS_SCENES",
    "WRS_SCENES_FULL",
    "WRS_SCENES_PARTIAL",
    "WRS_STARTING_PATH",
    "WRS_STARTING_ROW",
    "WRS_TYPE"
};

static const IAS_L0R_INTERVAL_METADATA *ias_l0r_mta_interval_ptr;
static const size_t IAS_L0R_INTERVAL_METADATA_SIZE =
    sizeof( IAS_L0R_INTERVAL_METADATA);
static const size_t IAS_L0R_INTERVAL_METADATA_SIZES
    [TABLE_SIZE_INTERVAL_METADATA] =
{
    sizeof( (*ias_l0r_mta_interval_ptr).ancillary_start_time),
    sizeof( (*ias_l0r_mta_interval_ptr).ancillary_stop_time),
    sizeof( (*ias_l0r_mta_interval_ptr).attitude_points),
    sizeof( (*ias_l0r_mta_interval_ptr).attitude_points_missing),
    sizeof( (*ias_l0r_mta_interval_ptr).attitude_points_rejected),
    sizeof( (*ias_l0r_mta_interval_ptr).collection_type),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_oli.upleft.lat),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_oli.upleft.lng),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_oli.upright.lat),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_oli.upright.lng),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_oli.loleft.lat),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_oli.loleft.lng),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_oli.loright.lat),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_oli.loright.lng),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_tirs.upleft.lat),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_tirs.upleft.lng),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_tirs.upright.lat),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_tirs.upright.lng),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_tirs.loleft.lat),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_tirs.loleft.lng),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_tirs.loright.lat),
    sizeof( (*ias_l0r_mta_interval_ptr).corners_tirs.loright.lng),
    sizeof( (*ias_l0r_mta_interval_ptr).cpf_name),
    sizeof( (*ias_l0r_mta_interval_ptr).crc_errors_oli),
    sizeof( (*ias_l0r_mta_interval_ptr).crc_errors_tirs),
    sizeof( (*ias_l0r_mta_interval_ptr).data_type),
    sizeof( (*ias_l0r_mta_interval_ptr).date_acquired),
    sizeof( (*ias_l0r_mta_interval_ptr).ephemeris_points),
    sizeof( (*ias_l0r_mta_interval_ptr).ephemeris_points_missing),
    sizeof( (*ias_l0r_mta_interval_ptr).ephemeris_points_rejected),
    sizeof( (*ias_l0r_mta_interval_ptr).frames_filled_oli),
    sizeof( (*ias_l0r_mta_interval_ptr).frames_filled_tirs),
    sizeof( (*ias_l0r_mta_interval_ptr).hostname),
    sizeof( (*ias_l0r_mta_interval_ptr).image_quality_oli),
    sizeof( (*ias_l0r_mta_interval_ptr).image_quality_tirs),
    sizeof( (*ias_l0r_mta_interval_ptr).interval_frames_oli),
    sizeof( (*ias_l0r_mta_interval_ptr).interval_frames_tirs),
    sizeof( (*ias_l0r_mta_interval_ptr).interval_number),
    sizeof( (*ias_l0r_mta_interval_ptr).interval_version),
    sizeof( (*ias_l0r_mta_interval_ptr).is_version),
    sizeof( (*ias_l0r_mta_interval_ptr).landsat_cal_interval_id),
    sizeof( (*ias_l0r_mta_interval_ptr).landsat_interval_id),
    sizeof( (*ias_l0r_mta_interval_ptr).nadir_offnadir),
    sizeof( (*ias_l0r_mta_interval_ptr).quality_algorithm),
    sizeof( (*ias_l0r_mta_interval_ptr).roll_angle ),
    sizeof( (*ias_l0r_mta_interval_ptr).satellite ),
    sizeof( (*ias_l0r_mta_interval_ptr).sensor_id ),
    sizeof( (*ias_l0r_mta_interval_ptr).spacecraft_id ),
    sizeof( (*ias_l0r_mta_interval_ptr).start_time_oli),
    sizeof( (*ias_l0r_mta_interval_ptr).start_time_tirs),
    sizeof( (*ias_l0r_mta_interval_ptr).station_id ),
    sizeof( (*ias_l0r_mta_interval_ptr).stop_time_oli),
    sizeof( (*ias_l0r_mta_interval_ptr).stop_time_tirs),
    sizeof( (*ias_l0r_mta_interval_ptr).time_code_errors_oli),
    sizeof( (*ias_l0r_mta_interval_ptr).time_code_errors_tirs),
    sizeof( (*ias_l0r_mta_interval_ptr).detector_map_id_tirs),
    sizeof( (*ias_l0r_mta_interval_ptr).wrs_ending_row),
    sizeof( (*ias_l0r_mta_interval_ptr).wrs_scenes),
    sizeof( (*ias_l0r_mta_interval_ptr).wrs_scenes_full),
    sizeof( (*ias_l0r_mta_interval_ptr).wrs_scenes_partial),
    sizeof( (*ias_l0r_mta_interval_ptr).wrs_starting_path),
    sizeof( (*ias_l0r_mta_interval_ptr).wrs_starting_row),
    sizeof( (*ias_l0r_mta_interval_ptr).wrs_type)
};

static const size_t OFFSET_INTERVAL_METADATA[TABLE_SIZE_INTERVAL_METADATA] =
{
   HOFFSET( IAS_L0R_INTERVAL_METADATA,ancillary_start_time),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,ancillary_stop_time),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,attitude_points),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,attitude_points_missing),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,attitude_points_rejected),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,collection_type),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_oli.upleft.lat),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_oli.upleft.lng),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_oli.upright.lat),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_oli.upright.lng),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_oli.loleft.lat),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_oli.loleft.lng),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_oli.loright.lat),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_oli.loright.lng),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_tirs.upleft.lat),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_tirs.upleft.lng),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_tirs.upright.lat),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_tirs.upright.lng),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_tirs.loleft.lat),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_tirs.loleft.lng),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_tirs.loright.lat),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,corners_tirs.loright.lng),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,cpf_name),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,crc_errors_oli),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,crc_errors_tirs),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,data_type),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,date_acquired),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,ephemeris_points),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,ephemeris_points_missing),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,ephemeris_points_rejected),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,frames_filled_oli),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,frames_filled_tirs),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,hostname),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,image_quality_oli),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,image_quality_tirs),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,interval_frames_oli),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,interval_frames_tirs),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,interval_number),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,interval_version),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,is_version),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,landsat_cal_interval_id),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,landsat_interval_id),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,nadir_offnadir),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,quality_algorithm),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,roll_angle),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,satellite),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,sensor_id),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,spacecraft_id),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,start_time_oli),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,start_time_tirs),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,station_id),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,stop_time_oli),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,stop_time_tirs),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,time_code_errors_oli),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,time_code_errors_tirs),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,detector_map_id_tirs),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,wrs_ending_row),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,wrs_scenes),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,wrs_scenes_full),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,wrs_scenes_partial),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,wrs_starting_path),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,wrs_starting_row),
   HOFFSET( IAS_L0R_INTERVAL_METADATA,wrs_type)
};

/*****************************************************************************/
static const hid_t  *field_type_metadata_scene = NULL;
static hid_t scene_metadata_types[TABLE_SIZE_SCENES_METADATA];

static const char *field_names_metadata_scene[TABLE_SIZE_SCENES_METADATA] =
{
    "ATTITUDE_POINTS",
    "ATTITUDE_POINTS_MISSING",
    "ATTITUDE_POINTS_REJECTED",
    "CORNER_UL_LAT_OLI",
    "CORNER_UL_LON_OLI",
    "CORNER_UR_LAT_OLI",
    "CORNER_UR_LON_OLI",
    "CORNER_LL_LAT_OLI",
    "CORNER_LL_LON_OLI",
    "CORNER_LR_LAT_OLI",
    "CORNER_LR_LON_OLI",
    "CORNER_UL_LAT_TIRS",
    "CORNER_UL_LON_TIRS",
    "CORNER_UR_LAT_TIRS",
    "CORNER_UR_LON_TIRS",
    "CORNER_LL_LAT_TIRS",
    "CORNER_LL_LON_TIRS",
    "CORNER_LR_LAT_TIRS",
    "CORNER_LR_LON_TIRS",
    "CRC_ERRORS",
    "DATE_ACQUIRED", /* timecode for the center frame */
    "DAY_NIGHT", /* day/night condition of the scene */
    "EPHEMERIS_POINTS", /* good ephemeris data points received */
    "EPHEMERIS_POINTS_MISSING", /* ephemeris data points found missing */
    "EPHEMERIS_POINTS_REJECTED",/* ephemeris data points found to fail */
    "FULL_PARTIAL_SCENE", /* full or partial scene */
    "HOSTNAME",
    "IMAGE_QUALITY_OLI",
    "IMAGE_QUALITY_TIRS",
    "LANDSAT_SCENE_ID", /* unique ID used for landsat data */
    "MISSING_FRAMES",
    "NADIR_OFFNADIR",
    "PRESENT_SENSOR_OLI",
    "PRESENT_SENSOR_TIRS",
    "ROLL_ANGLE", /* amount or spacecraft roll at the scene center */
    "SCENE_CENTER_LAT", /* scene center point */
    "SCENE_CENTER_LON", /* scene center point */
    "SCENE_CENTER_SHIFT",
    "SCENE_START_FRAME_OLI", /* first OLI frame number int the scene */
    "SCENE_STOP_FRAME_OLI", /* last OLI frame number in the scene */
    "SCENE_START_FRAME_TIRS", /* first TIRS frame number in the scene */
    "SCENE_STOP_FRAME_TIRS",/* last TIRS frame number in the scene */
    "START_TIME", /* spacecraft time of the first scene frame */
    "STOP_TIME", /* spacecraft time of the last scene frame */
    "SUBSETTER_VERSION_L0RP",
    "SUN_AZIMUTH", /* sun azimuth angle at the scene center */
    "SUN_ELEVATION", /* sun elevation angle at the scene center */
    "TARGET_WRS_PATH", /* nearest WRS path to the line of sight scene center */
    "TARGET_WRS_ROW", /* nearest WRS row to the line of sight scene center */
    "TIME_CODE_ERRORS", /* the number of time code errors in the scene */
    "WRS_PATH",
    "WRS_ROW",
    "WRS_SCENE_NUMBER"
};

static const IAS_L0R_SCENE_METADATA  *ias_l0r_metadata_scene_ptr = NULL;
static const size_t IAS_L0R_SCENE_METADATA_SIZE =
    sizeof( IAS_L0R_SCENE_METADATA );
static const size_t IAS_L0R_SCENE_METADATA_SIZES[TABLE_SIZE_SCENES_METADATA] =
{
    sizeof( (*ias_l0r_metadata_scene_ptr).attitude_points),
    sizeof( (*ias_l0r_metadata_scene_ptr).attitude_points_missing),
    sizeof( (*ias_l0r_metadata_scene_ptr).attitude_points_rejected),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_oli.upleft.lat),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_oli.upleft.lng),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_oli.upright.lat),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_oli.upright.lng),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_oli.loleft.lat),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_oli.loleft.lng),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_oli.loright.lat),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_oli.loright.lng),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_tirs.upleft.lat),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_tirs.upleft.lng),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_tirs.upright.lat),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_tirs.upright.lng),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_tirs.loleft.lat),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_tirs.loleft.lng),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_tirs.loright.lat),
    sizeof( (*ias_l0r_metadata_scene_ptr).corners_tirs.loright.lng),
    sizeof( (*ias_l0r_metadata_scene_ptr).crc_errors),
    sizeof( (*ias_l0r_metadata_scene_ptr).date_acquired),
    sizeof( (*ias_l0r_metadata_scene_ptr).day_night),
    sizeof( (*ias_l0r_metadata_scene_ptr).ephemeris_points),
    sizeof( (*ias_l0r_metadata_scene_ptr).ephemeris_points_missing),
    sizeof( (*ias_l0r_metadata_scene_ptr).ephemeris_points_rejected),
    sizeof( (*ias_l0r_metadata_scene_ptr).full_partial_scene),
    sizeof( (*ias_l0r_metadata_scene_ptr).hostname),
    sizeof( (*ias_l0r_metadata_scene_ptr).image_quality_oli),
    sizeof( (*ias_l0r_metadata_scene_ptr).image_quality_tirs),
    sizeof( (*ias_l0r_metadata_scene_ptr).landsat_scene_id),
    sizeof( (*ias_l0r_metadata_scene_ptr).missing_frames),
    sizeof( (*ias_l0r_metadata_scene_ptr).nadir_offnadir),
    sizeof( (*ias_l0r_metadata_scene_ptr).sensor_present_oli),
    sizeof( (*ias_l0r_metadata_scene_ptr).sensor_present_tirs),
    sizeof( (*ias_l0r_metadata_scene_ptr).roll_angle),
    sizeof( (*ias_l0r_metadata_scene_ptr).scene_center.lat),
    sizeof( (*ias_l0r_metadata_scene_ptr).scene_center.lng),
    sizeof( (*ias_l0r_metadata_scene_ptr).scene_center_shift),
    sizeof( (*ias_l0r_metadata_scene_ptr).scene_start_frame_oli),
    sizeof( (*ias_l0r_metadata_scene_ptr).scene_stop_frame_oli),
    sizeof( (*ias_l0r_metadata_scene_ptr).scene_start_frame_tirs),
    sizeof( (*ias_l0r_metadata_scene_ptr).scene_stop_frame_tirs),
    sizeof( (*ias_l0r_metadata_scene_ptr).start_time),
    sizeof( (*ias_l0r_metadata_scene_ptr).stop_time),
    sizeof( (*ias_l0r_metadata_scene_ptr).subsetter_version_l0rp),
    sizeof( (*ias_l0r_metadata_scene_ptr).sun_azimuth),
    sizeof( (*ias_l0r_metadata_scene_ptr).sun_elevation),
    sizeof( (*ias_l0r_metadata_scene_ptr).target_wrs_path),
    sizeof( (*ias_l0r_metadata_scene_ptr).target_wrs_row),
    sizeof( (*ias_l0r_metadata_scene_ptr).time_code_errors),
    sizeof( (*ias_l0r_metadata_scene_ptr).wrs_path),
    sizeof( (*ias_l0r_metadata_scene_ptr).wrs_row),
    sizeof( (*ias_l0r_metadata_scene_ptr).wrs_scene_number)
};

static const size_t OFFSET_SCENE_METADATA[TABLE_SIZE_SCENES_METADATA] =
{
    HOFFSET( IAS_L0R_SCENE_METADATA,attitude_points),
    HOFFSET( IAS_L0R_SCENE_METADATA,attitude_points_missing),
    HOFFSET( IAS_L0R_SCENE_METADATA,attitude_points_rejected),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_oli.upleft.lat),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_oli.upleft.lng),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_oli.upright.lat),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_oli.upright.lng),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_oli.loleft.lat),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_oli.loleft.lng),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_oli.loright.lat),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_oli.loright.lng),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_tirs.upleft.lat),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_tirs.upleft.lng),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_tirs.upright.lat),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_tirs.upright.lng),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_tirs.loleft.lat),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_tirs.loleft.lng),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_tirs.loright.lat),
    HOFFSET( IAS_L0R_SCENE_METADATA,corners_tirs.loright.lng),
    HOFFSET( IAS_L0R_SCENE_METADATA,crc_errors),
    HOFFSET( IAS_L0R_SCENE_METADATA,date_acquired),
    HOFFSET( IAS_L0R_SCENE_METADATA,day_night),
    HOFFSET( IAS_L0R_SCENE_METADATA,ephemeris_points),
    HOFFSET( IAS_L0R_SCENE_METADATA,ephemeris_points_missing),
    HOFFSET( IAS_L0R_SCENE_METADATA,ephemeris_points_rejected),
    HOFFSET( IAS_L0R_SCENE_METADATA,full_partial_scene),
    HOFFSET( IAS_L0R_SCENE_METADATA,hostname),
    HOFFSET( IAS_L0R_SCENE_METADATA,image_quality_oli),
    HOFFSET( IAS_L0R_SCENE_METADATA,image_quality_tirs),
    HOFFSET( IAS_L0R_SCENE_METADATA,landsat_scene_id),
    HOFFSET( IAS_L0R_SCENE_METADATA,missing_frames),
    HOFFSET( IAS_L0R_SCENE_METADATA,nadir_offnadir),
    HOFFSET( IAS_L0R_SCENE_METADATA,sensor_present_oli),
    HOFFSET( IAS_L0R_SCENE_METADATA,sensor_present_tirs),
    HOFFSET( IAS_L0R_SCENE_METADATA,roll_angle),
    HOFFSET( IAS_L0R_SCENE_METADATA,scene_center.lat),
    HOFFSET( IAS_L0R_SCENE_METADATA,scene_center.lng),
    HOFFSET( IAS_L0R_SCENE_METADATA,scene_center_shift),
    HOFFSET( IAS_L0R_SCENE_METADATA,scene_start_frame_oli),
    HOFFSET( IAS_L0R_SCENE_METADATA,scene_stop_frame_oli),
    HOFFSET( IAS_L0R_SCENE_METADATA,scene_start_frame_tirs),
    HOFFSET( IAS_L0R_SCENE_METADATA,scene_stop_frame_tirs),
    HOFFSET( IAS_L0R_SCENE_METADATA,start_time),
    HOFFSET( IAS_L0R_SCENE_METADATA,stop_time),
    HOFFSET( IAS_L0R_SCENE_METADATA,subsetter_version_l0rp),
    HOFFSET( IAS_L0R_SCENE_METADATA,sun_azimuth),
    HOFFSET( IAS_L0R_SCENE_METADATA,sun_elevation),
    HOFFSET( IAS_L0R_SCENE_METADATA,target_wrs_path),
    HOFFSET( IAS_L0R_SCENE_METADATA,target_wrs_row),
    HOFFSET( IAS_L0R_SCENE_METADATA,time_code_errors),
    HOFFSET( IAS_L0R_SCENE_METADATA,wrs_path),
    HOFFSET( IAS_L0R_SCENE_METADATA,wrs_row),
    HOFFSET( IAS_L0R_SCENE_METADATA,wrs_scene_number)
};

const hid_t* ias_l0r_set_file_field_type(HDFIO* hdfio_ptr);
const hid_t* ias_l0r_set_interval_field_type(HDFIO* hdfio_ptr);
const hid_t* ias_l0r_set_scene_field_type(HDFIO* hdfio_ptr);

/*******************************************************************************
 NAME: ias_l0r_mta_init

 PURPOSE: Initializes the types used with the tables.

 RETURNS: SUCCESS- Metadata types populated
          ERROR- Unable to populate metadata types
*******************************************************************************/
int ias_l0r_mta_init(HDFIO* hdfio_ptr)
{
    /* set the constant pointers to the array of types */
    if (field_type_metadata_file == NULL)
    {
        field_type_metadata_file = ias_l0r_set_file_field_type(hdfio_ptr);
        if (field_type_metadata_file == NULL)
            return ERROR;
    }
    
    if (field_type_metadata_interval == NULL)
    {
        field_type_metadata_interval =
            ias_l0r_set_interval_field_type(hdfio_ptr);
        if (field_type_metadata_interval == NULL)
            return ERROR;
    }

    if (field_type_metadata_scene == NULL)
    {
        field_type_metadata_scene = ias_l0r_set_scene_field_type(hdfio_ptr);
        if (field_type_metadata_scene == NULL)
            return ERROR;
    }
    return SUCCESS;
};

/*******************************************************************************
 private metadata routines
*******************************************************************************/
/******************************************************************************
 NAME: ias_l0r_establish_mta_file

 PURPOSE: Initializes access to or creates the specified metadata file

 RETURNS: SUCCESS- Access had been established
          ERROR- Could not establish access
******************************************************************************/
static int ias_l0r_mta_establish_file
(
    HDFIO *hdfio_ptr, /* I: Pointer to structure used in I/O */
    int create_if_absent /* I: Create the file if it does not exist */
)
{
    char metadata_filename[IAS_L0R_FILE_NAME_LENGTH];
    char *metadata_appendix = "_MTA.h5\0";

    if (hdfio_ptr->access_mode_metadata == -1)
    {
        IAS_LOG_ERROR("Metadata file is not open");
        return ERROR;
    }
   
    if ((strlen(hdfio_ptr->l0r_name_prefix) + strlen(metadata_appendix) + 1)  >
        IAS_L0R_FILE_NAME_LENGTH)
    {
        IAS_LOG_ERROR("Metadata file name of %s%s is too long. Max is %i",
            hdfio_ptr->l0r_name_prefix,metadata_appendix,
            IAS_L0R_FILE_NAME_LENGTH);
        return ERROR;
    }
 
    snprintf(metadata_filename, IAS_L0R_FILE_NAME_LENGTH, "%s%s",
        hdfio_ptr->l0r_name_prefix,
        metadata_appendix);

    return ias_l0r_hdf_establish_file(hdfio_ptr->path, metadata_filename,
        &hdfio_ptr->file_id_metadata, hdfio_ptr->access_mode_metadata,
        create_if_absent);
}

/* subroutine used in reading metadata */
/******************************************************************************
 NAME: ias_l0r_mta_read

 PURPOSE: Internal routines to read the mta from the table name specified

 RETURNS: SUCCESS- Data was read in the buffer
          FAILURE- Unable to read into the buffer
******************************************************************************/
static int ias_l0r_mta_read
(
    HDFIO *hdfio_ptr, /* I: Pointer to structure used in I/O */
    const int index, /* I: First record to read */
    const int count, /* I: Number of records to read */
    const char *table_name, /* I: Table name to read from */
    const size_t size, /* I: Size of the structure which stores the data */
    const size_t *offset, /* I: List of the offsets for each member */
    const size_t *size_list, /* I: List of the sizes of each member */
    void *buffer /* I: Data Buffer */
)
{
    int temp_size = 0;
    herr_t status;

    /*clear the buffer to initialize structure members that are not in the
    *table, useful for reading older files that may not contain new fields */
    memset(buffer, 0, size * count );

    status = ias_l0r_mta_establish_file(hdfio_ptr, FALSE);
    if (status == ERROR || hdfio_ptr->file_id_metadata <= 0)
    {
        IAS_LOG_ERROR("Error establishing file for read");
        return ERROR;
    }
 
    /* verify there are an adequate number of records for the read */
    if (ias_l0r_mta_table_records_count(hdfio_ptr, table_name,
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

    status = H5TBread_records(hdfio_ptr->file_id_metadata,
        table_name, index, count, size,
        offset, size_list, buffer);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error reading from %s", table_name);
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_mta_table_records_count

 PURPOSE: Internal routine to read the size of mta table with the name
          specified

 RETURNS: SUCCESS- Size was read
          FAILURE- Unable to read size
******************************************************************************/
static int ias_l0r_mta_table_records_count
(
    HDFIO *hdfio_ptr, /* I: Pointer to structure used in I/O */
    const char *table, /* I: Table name to read from */
    int *size          /* O: Number of records in the table */
)
{
    int status;
    status = ias_l0r_mta_establish_file(hdfio_ptr, FALSE);
    if (status == ERROR)
    {
        *size = 0;
        return ERROR;
    }
    /* if there was no file opened but the routine was successful the file */
    /* does not exist, the case for size 0 */
    else if (hdfio_ptr->file_id_metadata < 1)
    {
        *size = 0;
        return SUCCESS;
    }

    /* if the file exists and there wasn't an error check the size
       of the table */
    status = ias_l0r_hdf_table_records_count(hdfio_ptr->file_id_metadata,
         table, size);
    if (status == ERROR)
    {
        *size = 0;
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_open_metadata

 PURPOSE: Opens access to the metadata (MTA) file

 RETURNS: SUCCESS- Metadata file opened
          ERROR- Metadata failed to open
******************************************************************************/
int ias_l0r_open_metadata
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const IAS_ACCESS_MODE file_access_mode /* I: Mode to open file in */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }
    
    HDFIO *hdfio_ptr = l0r;
    hdfio_ptr->access_mode_metadata = file_access_mode;
    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_close_metadata

 PURPOSE: Closes access to the metadata (MTA) file

 RETURNS: SUCCESS- Metadata file close
          ERROR- Metadata file failed to close
******************************************************************************/
int ias_l0r_close_metadata
(
    L0RIO *l0r /* I: structure for the file used in I/O */
)
{
    herr_t status;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr = l0r;

    if (hdfio_ptr->table_id_mta_file != -1)
    {
        status = H5Dclose(hdfio_ptr->table_id_mta_file);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing hdf MTA file resources");
            return ERROR;
        }
        hdfio_ptr->table_id_mta_file = -1;
    }

    if (hdfio_ptr->table_id_mta_interval != -1)
    {
        status = H5Dclose(hdfio_ptr->table_id_mta_interval);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing hdf MTA interval resources");
            return ERROR;
        }
        hdfio_ptr->table_id_mta_interval = -1;
    }

    if (hdfio_ptr->table_id_mta_scene != -1)
    {
        status = H5Dclose(hdfio_ptr->table_id_mta_scene);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing hdf MTA scene resources");
            return ERROR;
        }
        hdfio_ptr->table_id_mta_scene = -1;
    }

    if (hdfio_ptr->file_id_metadata > -1)
    {
        status = H5Fclose(hdfio_ptr->file_id_metadata);
        if (status < 0)
        {
            IAS_LOG_ERROR("Error closing hdf resources, file_id_metadata");
            return ERROR;
        }
        hdfio_ptr->file_id_metadata = -1;
    }

    hdfio_ptr->access_mode_metadata = -1;
    return SUCCESS;
}
/*******************************************************************************
*Public metadata comparison routines
*    NOTE: These routines are mostly for testing purposes
*******************************************************************************/
/******************************************************************************
 NAME: ias_l0r_mta_compare_file

 PURPOSE: Counts the number of elements in the structure that differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_mta_compare_file
(
    const IAS_L0R_FILE_METADATA *file_metadata, /* I: 1st struct to compare */
    const IAS_L0R_FILE_METADATA *file_metadata2, /* I: 2nd struct to compare */
    int *different_elements /* O: Number of elements that differ */
)
{
    int i = 0;
    
    if (file_metadata == NULL ||
        file_metadata2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }
  
    *different_elements = 0;
    if (file_metadata == file_metadata2)
    {
        return SUCCESS;
    }

    if (strncmp(file_metadata->ancillary_file.name,
        file_metadata2->ancillary_file.name, IAS_L0R_FILE_NAME_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp(file_metadata->checksum_file.name,
        file_metadata2->checksum_file.name, IAS_L0R_FILE_NAME_LENGTH) != 0)
    {
        *different_elements += 1;
    }
   
    /* Compare the information for each of the band files */
    for (i = 0; i < IAS_L0R_MAX_BAND_FILES; i++)
    {
        if (strncmp(file_metadata->band_files[i].name,
            file_metadata2->band_files[i].name, IAS_L0R_FILE_NAME_LENGTH) != 0)
        {
            *different_elements += 1;
        }
   }

    if (strncmp(file_metadata->metadata_file.name,
        file_metadata2->metadata_file.name, IAS_L0R_FILE_NAME_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (file_metadata->interval_files != file_metadata2->interval_files)
    {
        *different_elements += 1;
    }
   
    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_mta_compare_interval

 PURPOSE: Counts the number of elements in the structure that differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_mta_compare_interval
(
    /* I: 1st struct to compare */
    const IAS_L0R_INTERVAL_METADATA *interval_metadata,
    /* I: 2nd struct to compare */
    const IAS_L0R_INTERVAL_METADATA *interval_metadata2,
    int *different_elements /* O: Number of elements that differ */
)
{
    if (interval_metadata == NULL ||
        interval_metadata2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;
    if (interval_metadata == interval_metadata2)
    {
        return SUCCESS;
    }

    if (strncmp(interval_metadata->ancillary_start_time,
        interval_metadata2->ancillary_start_time, IAS_L0R_DATE_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp( interval_metadata->ancillary_stop_time,
        interval_metadata2->ancillary_stop_time, IAS_L0R_DATE_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp( interval_metadata->collection_type,
        interval_metadata2->collection_type,
             IAS_L0R_COLLECTION_TYPE_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    /* compare the memory containing 2 sets of corner points */
    if (memcmp(&interval_metadata->corners_oli,
        &interval_metadata2->corners_oli,
        (2 * sizeof(IAS_L0R_CORNERS)) ) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp( interval_metadata->cpf_name, interval_metadata2->cpf_name,
        IAS_L0R_CPF_NAME_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    /* Compare both OLI & TIRS fields at the same time by
        multiplying the size by 2*/
    if (memcmp(&interval_metadata->crc_errors_oli,
        &interval_metadata2->crc_errors_oli, sizeof(uint32_t)*2) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp( interval_metadata->data_type, interval_metadata2->data_type,
        IAS_L0R_DATA_TYPE_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp( interval_metadata->date_acquired,
        interval_metadata2->date_acquired, IAS_L0R_DATE_LENGTH) != 0)
    {
        *different_elements += 1;
    }
 
    if (memcmp(&interval_metadata->ephemeris_points,
        &interval_metadata2->ephemeris_points, sizeof(uint16_t)) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->hostname, interval_metadata2->hostname,
        IAS_L0R_HOSTNAME_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (memcmp(&interval_metadata->image_quality_oli,
        &interval_metadata2->image_quality_oli, sizeof(uint8_t)) != 0)
    {
        *different_elements += 1;
    }

    if (memcmp(&interval_metadata->image_quality_tirs,
        &interval_metadata2->image_quality_tirs, sizeof(uint8_t)) != 0)
    {
        *different_elements += 1;
    }

    if (memcmp(&interval_metadata->interval_number,
        &interval_metadata2->interval_number, sizeof(uint8_t)) != 0)
    {
        *different_elements += 1;
    }

    if (interval_metadata->interval_version !=
        interval_metadata2->interval_version)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->is_version, interval_metadata2->is_version,
        IAS_SOFTWARE_VERSION_SIZE) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->landsat_cal_interval_id,
        interval_metadata2->landsat_cal_interval_id,
             IAS_L0R_INTERVAL_ID_LENGTH)!=0)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->landsat_interval_id,
        interval_metadata2->landsat_interval_id,
            IAS_L0R_INTERVAL_ID_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    /*number of OLI & TIRS frames that were filled
      Compare both OLI & TIRS fields at the same time by
      multiplying the size by 2*/

    if (memcmp(&interval_metadata->frames_filled_oli,
        &interval_metadata2->frames_filled_oli, sizeof(uint32_t)*2) != 0 )
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->nadir_offnadir,
        interval_metadata2->nadir_offnadir, IAS_L0R_NADIR_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->quality_algorithm,
        interval_metadata2->quality_algorithm,
        IAS_L0R_ALGORITHM_NAME_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (interval_metadata->roll_angle != interval_metadata2->roll_angle)
    {
        *different_elements += 1;
    }

    if (interval_metadata->satellite != interval_metadata2->satellite)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->sensor_id, interval_metadata2->sensor_id,
        IAS_L0R_SENSOR_ID_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->spacecraft_id,
        interval_metadata2->spacecraft_id, IAS_L0R_SPACECRAFT_ID_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->start_time_oli,
        interval_metadata2->start_time_oli,
        IAS_L0R_DATE_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->start_time_tirs,
        interval_metadata2->start_time_tirs,
        IAS_L0R_DATE_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->station_id, interval_metadata2->station_id,
         IAS_L0R_STATION_ID_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->stop_time_oli,
        interval_metadata2->stop_time_oli,
        IAS_L0R_DATE_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp(interval_metadata->stop_time_tirs,
        interval_metadata2->stop_time_tirs,
        IAS_L0R_DATE_LENGTH) != 0)
    {
        *different_elements += 1;
    }

    /* Compare both OLI & TIRS fields
       Compare both OLI & TIRS fields at the same time by
        multiplying the size by 2*/

    if (memcmp(&interval_metadata->time_code_errors_oli,
        &interval_metadata2->time_code_errors_oli, sizeof(uint32_t)*2) != 0)
    {
        *different_elements += 1;
    }

    if (interval_metadata->detector_map_id_tirs !=
        interval_metadata2->detector_map_id_tirs)
    {
        *different_elements += 1;
    }

    /* WRS information for the interval */
    if (memcmp(&interval_metadata->wrs_ending_row,
        &interval_metadata2->wrs_ending_row, sizeof(uint8_t)*7) != 0)
    {
        *different_elements += 1;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_mta_compare_scene

 PURPOSE: Counts the number of elements in the structure that differ.

 RETURNS: SUCCESS- The data was successfully compared
        ERROR- The data could not be compared
******************************************************************************/
int ias_l0r_mta_compare_scene
(
    const IAS_L0R_SCENE_METADATA *scene_metadata, /* I: 1st struct to compare */
    const IAS_L0R_SCENE_METADATA *scene_metadata2, /* I: 2nd struct to compare*/
    int *different_elements /* O: Number of elements that differ */
)
{
    if (scene_metadata == NULL ||
        scene_metadata2 == NULL)
    {
        IAS_LOG_ERROR("NULL pointer passed");
        return ERROR;
    }

    *different_elements = 0;
    if (scene_metadata == scene_metadata2)
    {
        return SUCCESS;
    }

    if (scene_metadata->attitude_points != scene_metadata2->attitude_points)
    {
        different_elements += 1;
    }
    if (scene_metadata->attitude_points_missing !=
        scene_metadata2->attitude_points_missing)
    {
        different_elements += 1;
    }
    if (scene_metadata->attitude_points_rejected !=
        scene_metadata2->attitude_points_rejected)
    {
        different_elements += 1;
    }

    /* compare the memory containing 2 sets of corner points */   
    if (memcmp(&scene_metadata->corners_oli, &scene_metadata2->corners_oli,
        sizeof(IAS_L0R_CORNERS) * 2) != 0)
    {
        different_elements += 1;
    }

    if (memcmp(&scene_metadata->crc_errors,
        &scene_metadata2->crc_errors, sizeof(uint32_t)) != 0)
    {
        *different_elements += 1;
    }

    if (strncmp(scene_metadata->date_acquired, scene_metadata2->date_acquired,
        IAS_L0R_TIME_LENGTH) != 0)
    {
        different_elements += 1;
    }
   
    if (strncmp(scene_metadata->day_night, scene_metadata2->day_night,
        IAS_L0R_DAY_NIGHT_LENGTH) != 0)
    {
        different_elements += 1;
    }

    if (scene_metadata->ephemeris_points != scene_metadata2->ephemeris_points)
    {
        different_elements += 1;
    }

    if (scene_metadata->ephemeris_points_missing !=
        scene_metadata2->ephemeris_points_missing)
    {
        different_elements += 1;
    }

    if (scene_metadata->ephemeris_points_rejected !=
        scene_metadata2->ephemeris_points_rejected)
    {
        different_elements += 1;
    }

    if (strncmp(scene_metadata->full_partial_scene,
        scene_metadata2->full_partial_scene,
        IAS_L0R_SCENE_FULL_PARTIAL_LENGTH) != 0)
    {
        different_elements += 1;
    }

    if (strncmp(scene_metadata->hostname, scene_metadata2->hostname,
        IAS_L0R_HOSTNAME_LENGTH) != 0)
    {
        different_elements += 1;
    }

    if (scene_metadata->image_quality_oli != scene_metadata2->image_quality_oli)
    {
        different_elements += 1;
    }

    if (scene_metadata->image_quality_tirs !=
         scene_metadata2->image_quality_tirs)
    {
        different_elements += 1;
    }

    if (strncmp(scene_metadata->landsat_scene_id,
        scene_metadata2->landsat_scene_id, IAS_L0R_SCENE_ID_LENGTH) != 0)
    {
        different_elements += 1;
    }

    if (scene_metadata->missing_frames !=
        scene_metadata2->missing_frames)
    {
        different_elements += 1;
    }

    if (strncmp(scene_metadata->nadir_offnadir, scene_metadata2->nadir_offnadir,
        IAS_L0R_NADIR_LENGTH) != 0)
    {
        different_elements += 1;
    }

    if (scene_metadata->roll_angle != scene_metadata2->roll_angle)
    {
        different_elements += 1;
    }

    if (memcmp(&scene_metadata->scene_center, &scene_metadata2->scene_center,
        sizeof(IAS_DBL_LAT_LONG)) != 0)
    {
        different_elements += 1;
    }

    if (scene_metadata->scene_center_shift !=
        scene_metadata2->scene_center_shift)
    {
        different_elements += 1;
    }

    if (scene_metadata->scene_start_frame_oli !=
             scene_metadata2->scene_start_frame_oli)
    {
        different_elements += 1;
    }

    if (scene_metadata->scene_stop_frame_oli !=
             scene_metadata2->scene_stop_frame_oli)
    {
        different_elements += 1;
    }
    
    if (scene_metadata->scene_start_frame_tirs !=
             scene_metadata2->scene_start_frame_tirs)
    {
        different_elements += 1;
    }

    if (scene_metadata->scene_stop_frame_tirs !=
             scene_metadata2->scene_stop_frame_tirs)
    {
        different_elements += 1;
    }

    if (scene_metadata->sensor_present_oli !=
             scene_metadata2->sensor_present_oli)
    {
        different_elements += 1;
    }

    if (scene_metadata->sensor_present_tirs !=
             scene_metadata2->sensor_present_tirs)
    {
        different_elements += 1;
    }
    
    if (strncmp(scene_metadata->start_time, scene_metadata2->start_time,
        IAS_L0R_TIME_LENGTH) != 0)
    {
        different_elements += 1;
    }
    if (strncmp(scene_metadata->stop_time, scene_metadata2->stop_time,
        IAS_L0R_TIME_LENGTH) != 0)
    {
        different_elements += 1;
    }

    if (strncmp(scene_metadata->subsetter_version_l0rp,
        scene_metadata2->subsetter_version_l0rp,
        IAS_SOFTWARE_VERSION_SIZE) != 0)
    {
        different_elements += 1;
    }

    if (scene_metadata->sun_azimuth != scene_metadata2->sun_azimuth)
    {
        different_elements += 1;
    }
    
    if (scene_metadata->sun_elevation != scene_metadata2->sun_elevation)
    {
        different_elements += 1;
    }

    if (scene_metadata->target_wrs_path != scene_metadata2->target_wrs_path)
    {
        different_elements += 1;
    }

    if (scene_metadata->target_wrs_row != scene_metadata2->target_wrs_row)
    {
        different_elements += 1;
    }

    if (scene_metadata->time_code_errors != scene_metadata2->time_code_errors)
    {
        different_elements += 1;
    }
 
    if (scene_metadata->wrs_scene_number != scene_metadata2->wrs_scene_number)
    {
        different_elements += 1;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_format_time

 PURPOSE: Formats a tm (time) structure into a string in the format used
          by the library

 RETURNS: SUCCESS- String formatted
          ERROR- Unable to format string
******************************************************************************/
int ias_l0r_format_time
(
    const struct tm *timeptr, /* I: The time structure to be formatted */
    char *formatted_time, /* O: The formatted time */
    int size     /* I: Size in bytes of the buffer allocated */
)
{
    if(size < IAS_L0R_FILEDATE_LENGTH)
    {
        IAS_LOG_ERROR("The size of %u for the buffer you passed for "
            "the time string is to small.  It needs to be at least %d",
            size, IAS_L0R_FILEDATE_LENGTH);
        return ERROR;
    }

    int size_written = 0;

    size_written = strftime(formatted_time, size,
                      "%04Y:%03j:%02H:%02M:%02S.0000000", timeptr);

    if (size_written > 0 && size_written <= size)
        return SUCCESS;
    else
        return ERROR;
}

/******************************************************************************
 NAME: ias_l0r_get_file_metadata

 PURPOSE: Reads the file level metadata

 RETURNS: SUCCESS- Metadata retrieved
          ERROR- Metadata could not be retrieved
******************************************************************************/
int ias_l0r_get_file_metadata
(
     L0RIO *l0r, /* I: structure used with the l0r data to be read from */
     IAS_L0R_FILE_METADATA *mta_info_file /* O: File Metadata */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr =  l0r;

    return ias_l0r_mta_read(hdfio_ptr, 0, 1,
        TABLE_NAME_FILE_METADATA,
        IAS_L0R_FILE_METADATA_SIZE, OFFSET_FILE_METADATA,
        IAS_L0R_FILE_METADATA_SIZES, mta_info_file);
}

/******************************************************************************
 NAME: ias_l0r_get_file_metadata_records_count

 PURPOSE: Reads the file level metadata table size

 RETURNS: SUCCESS- Size retrieved
          ERROR- Size could not be determined
******************************************************************************/
int ias_l0r_get_file_metadata_records_count
(
    L0RIO *l0r, /* I: structure used with the l0r data */
    int *size          /* O: number of records in the file dataset */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr =  l0r;

    return ias_l0r_mta_table_records_count(hdfio_ptr,
        TABLE_NAME_FILE_METADATA, size);
}

/******************************************************************************
 NAME: ias_l0r_get_filenames

 PURPOSE: Generates comma delimited list of the
          filenames which make up the dataset,
          as read from the metadata.
          This routine allocates a char *,
          which is returned by this routine. This memory must be freed
          by the CALLING FUNCTION.
 
 RETURNS: SUCCESS- Filenames retrieved
          ERROR- File names could not be determined
******************************************************************************/
int ias_l0r_get_filenames
(
     L0RIO *l0r_ptr, /* I: structure used with the l0r data */
     char** filenames /* O: filenames populated by the read  */
)
{
    char *filename_list = NULL;
    int file_count = 0;
    int filename_list_size;
    int i;

    IAS_L0R_FILE_METADATA mta_info_file;

    if (l0r_ptr == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }

    if (*filenames != NULL)
    {
        IAS_LOG_ERROR("Pointer should be uninitialized to NULL");
        return ERROR;
    }

    HDFIO *hdfio_ptr =  l0r_ptr;

    ias_l0r_get_filename_count(l0r_ptr, &file_count);

    if (file_count < 1)
    {
        /* there are no filenames that can be read */
        return ERROR;
    }

    /* IAS_L0R_FILE_NAME_LENGTH is used as the constant to
       declare the filename */
    /* arrays in the IAS_L0R_FILE_METADATA structure */
    filename_list_size = file_count * IAS_L0R_FILE_NAME_LENGTH;
    /* add one to the size of the list to include the delimiter */
    *filenames = (char*) malloc((filename_list_size+1) * sizeof(char));
    filename_list = *filenames;

    if (filename_list == NULL)
    {
        IAS_LOG_ERROR("Unable to allocate memory for filename list");
        return ERROR;
    }

    ias_l0r_mta_read(hdfio_ptr, 0, 1,
        TABLE_NAME_FILE_METADATA,
        IAS_L0R_FILE_METADATA_SIZE, OFFSET_FILE_METADATA,
        IAS_L0R_FILE_METADATA_SIZES, &mta_info_file);

    /* initialize the list empty. strncat doesn't work correctly without it */
    strncpy(filename_list, "", filename_list_size);

    /* check the filename for each file to determine existence */
    for (i = 0; i< IAS_L0R_MAX_BAND_FILES; i++)
    {
        if ( strlen(mta_info_file.band_files[i].name) > 0)
        {
            strncat(filename_list, mta_info_file.band_files[i].name,
               filename_list_size);
            strncat(filename_list, ",", filename_list_size);
        }
    }
    if ( strlen(mta_info_file.ancillary_file.name) > 0)
    {
        strncat(filename_list, mta_info_file.ancillary_file.name,
            filename_list_size);
        strncat(filename_list, ",", filename_list_size);
    }
    if ( strlen(mta_info_file.checksum_file.name) > 0)
    {
        strncat(filename_list, mta_info_file.checksum_file.name,
            filename_list_size);
        strncat(filename_list, ",", filename_list_size);
    }
    if ( strlen(mta_info_file.metadata_file.name) > 0)
    {
        strncat(filename_list, mta_info_file.metadata_file.name,
            filename_list_size);
        strncat(filename_list, ",", filename_list_size);
    }

    /* remove the last character if it is a delimiter */
    filename_list_size = strlen(filename_list);
    if (filename_list[filename_list_size-1] == ',')
    {
        filename_list[filename_list_size-1] = '\0';
    }

    return SUCCESS;
}

/*******************************************************************************
 NAME: ias_l0r_get_filenames_count

 PURPOSE: Get the number of files
          which make up the dataset,
          as read from the metadata

 RETURNS: SUCCESS- Count retrieved
          ERROR- Count unable to be determined
*******************************************************************************/
int ias_l0r_get_filename_count
(
     L0RIO *l0r, /* I: structure used with the l0r data */
     int *count /* O: Number of files */
)
{
    int i = 0; /* loop increment */
    IAS_L0R_FILE_METADATA mta_info_file;
    
    if (ias_l0r_get_file_metadata(l0r, &mta_info_file) == ERROR)
    {
        *count = -1;
        return ERROR;
    }

    *count = 0;
 
    /* check the filename for each file to determine existence */
    for (i = 0; i < IAS_L0R_MAX_BAND_FILES; i++)
    {
        if ( strlen(mta_info_file.band_files[i].name) > 0)
        {
            *count += 1;
        }
    }
    if ( strlen(mta_info_file.ancillary_file.name) > 0)
    {
        *count += 1;
    }
    if ( strlen(mta_info_file.checksum_file.name) > 0)
    {
        *count += 1;
    }
    if ( strlen(mta_info_file.metadata_file.name) > 0)
    {
        *count += 1;
    }

    return SUCCESS;
}

/*******************************************************************************
 NAME: ias_l0r_get_metadata_filename

 PURPOSE:  Generates a filename for the metadata
           file using the prefix for the passed dataset

 RETURNS: SUCCESS- Filename retrieved
          ERROR- Unable to determine filename
*******************************************************************************/
int ias_l0r_get_metadata_filename
(
     L0RIO *l0r, /* I: structure used with the l0r data */
     char filename[IAS_L0R_FILE_NAME_LENGTH+1]  /* O: filename populated
                                                      by the read */
)
{
    const char metadata_file_appendix[] = "_MTA.h5\0";

    if (l0r == NULL)
    {
        return ERROR;
    }
    HDFIO *hdf_ptr =  l0r;
    
    strncpy(filename, hdf_ptr->l0r_name_prefix,
        IAS_L0R_FILE_NAME_LENGTH);
    strncat(filename, metadata_file_appendix,
        IAS_L0R_FILE_NAME_LENGTH);
    
    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_get_interval_metadata

 PURPOSE: Read the interval level metadata from the file provided

 RETURNS: SUCCESS- Metadata retrieved
          ERROR- Metadata could not be retrieved
******************************************************************************/
int ias_l0r_get_interval_metadata
(
    L0RIO *l0r, /* I: structure used with the l0r data */
    IAS_L0R_INTERVAL_METADATA *mta_info_interval /* O: Metadata read */
)
{
    int status = 0;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr =  l0r;

    status = ias_l0r_mta_read(hdfio_ptr, 0, 1,
        TABLE_NAME_INTERVAL_METADATA,
        IAS_L0R_INTERVAL_METADATA_SIZE, OFFSET_INTERVAL_METADATA,
        IAS_L0R_INTERVAL_METADATA_SIZES, mta_info_interval);

    return status;
}

/******************************************************************************
 NAME: ias_l0r_get_interval_metadata_records_count

 PURPOSE: Read the size of the interval level metadata table

 RETURNS: SUCCESS- Size retrieved
          ERROR- Unable to determine size
******************************************************************************/
int ias_l0r_get_interval_metadata_records_count
(
    L0RIO *l0r,  /* I: structure used with the l0r data */
    int *size          /* O: number of records in the interval dataset */
)
{
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_mta_table_records_count(hdfio_ptr,
        TABLE_NAME_INTERVAL_METADATA, size);
}

/******************************************************************************
 NAME: ias_l0r_get_scene_metadata

 PURPOSE: Read the scene level metadata from the file provided

 RETURNS: SUCCESS- Metadata retrieved
          ERROR- Metadata could not be retrieved
******************************************************************************/
int ias_l0r_get_scene_metadata
(
    L0RIO *l0r, /* I: structure used with the l0r data */
    const int index, /* I: scene to begin the read at */
    const int count, /* I: the number of scenes to read */
    IAS_L0R_SCENE_METADATA *mta_info_scene /* O: Metadata read */
)
{
    int i = 0;
    int status = SUCCESS;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr =  l0r;

    status =  ias_l0r_mta_read(hdfio_ptr, index, count,
        TABLE_NAME_SCENES_METADATA,
        IAS_L0R_SCENE_METADATA_SIZE, OFFSET_SCENE_METADATA,
        IAS_L0R_SCENE_METADATA_SIZES, mta_info_scene);

    for (i = 0; i < count; i++)
    {
        if (mta_info_scene[i].sensor_present_oli == 'Y')
        {
            mta_info_scene[i].sensor_present_oli = 1;
        }
        else
        {
            mta_info_scene[i].sensor_present_oli = 0;
        }

        if (mta_info_scene[i].sensor_present_tirs == 'Y')
        {
            mta_info_scene[i].sensor_present_tirs = 1; 
        }
        else
        {
            mta_info_scene[i].sensor_present_tirs = 0;
        } 
    } 

    return status;
}

/******************************************************************************
 NAME: ias_l0r_mta_get_scene_metadata_by_id

 PURPOSE:  Retrieves the scene metadata
           for the scene that matches the id provided
           
 RETURNS: SUCCESS- Metadata retrieved
          ERROR- Metadata could not be retrieved
******************************************************************************/
int ias_l0r_get_scene_metadata_by_id
(
    L0RIO *l0r, /* I: structure used with the l0r data */
    const char *scene_id, /* I: string for the Landsat scene id */
    IAS_L0R_SCENE_METADATA *mta_info_scene /* O: Metadata read */
)
{
    int i = 0;
    herr_t status;
    int count = 0;

    if (l0r == NULL || scene_id == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    memset(mta_info_scene, 0, sizeof(IAS_L0R_SCENE_METADATA));

    if (ias_l0r_get_scene_metadata_records_count(l0r, &count) == ERROR)
    {
        IAS_LOG_ERROR("No scenes available to read");
        return ERROR;
    }

    /* This will return the first ID found.  IDs are stated to be unique */
    /*  so the possibility of another match is not checked */
    for(i = 0; i < count; i++)
    {
        status = ias_l0r_get_scene_metadata(l0r, i, 1, mta_info_scene);
   
        if (status < 0)
        {
            IAS_LOG_ERROR("Problem reading scene number %d of %u from %s for "
                "%s",i, count, TABLE_NAME_SCENES_METADATA, scene_id);
            return ERROR;
        }
        if (strncmp(scene_id, mta_info_scene->landsat_scene_id,
            IAS_L0R_SCENE_ID_LENGTH) == 0)
        {
            return SUCCESS;
        }
    }
   
    memset(mta_info_scene, 0, sizeof(IAS_L0R_SCENE_METADATA));
 
    IAS_LOG_ERROR(
            "The scene id %s was not found", scene_id);
    return ERROR;
}

/******************************************************************************
 NAME: ias_l0r_get_scene_metadata_records_count
 
 PURPOSE:  Read the size of the scene level metadata table
 
 RETURNS: SUCCESS- Size retrieved
          ERROR- Unable to determine size
******************************************************************************/
int ias_l0r_get_scene_metadata_records_count
(
    L0RIO *l0r, /* I: structure used with the l0r data */
    int *size          /* O: number of records in the scene dataset */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr = l0r;
    return ias_l0r_mta_table_records_count(hdfio_ptr,
        TABLE_NAME_SCENES_METADATA, size);
}

/******************************************************************************
 NAME: ias_l0r_mta_write
 
 PURPOSE:  Internal routine to write to the specified table in the metadata
           file.
 
 RETURNS: SUCCESS- Table written to
          ERROR- Unable to write to table
******************************************************************************/
int ias_l0r_mta_write
(
    L0RIO *l0r, /* structure for the file used in I/O */
    const int index, /* I: First record to write */
    const int count, /* I: Number of records to write */
    const char *table_name, /* I: Table name to write to */
    const size_t size, /* I: Size of the structure which stores the data */
    const size_t *offset, /* I: List of the offsets for each member */
    const size_t *size_list, /* I: List of the sizes of each member */
    const void *buffer /* I: Data Buffer */
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

    if (hdfio_ptr->access_mode_metadata != IAS_WRITE)
    {
        IAS_LOG_ERROR("Ancillary data is not in write mode");
        return ERROR;
    }

    dataset_id = H5Dopen(hdfio_ptr->file_id_metadata, table_name,
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
        IAS_LOG_ERROR("Error getting dimension for dataspace %i",
            dataspace_id);
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
        IAS_LOG_ERROR("Error closing dataset for %s", table_name);
    }
    status = H5Sclose(dataspace_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error closing dataspace %i", dataspace_id);
    }
    /* write the records to the table */
    status = H5TBwrite_records(hdfio_ptr->file_id_metadata, table_name,
        index, count, size, offset, size_list, buffer);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error writing to %s", table_name);
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_delete
 
 PURPOSE:  Internal routine to delete the specified table in the metadata
           file.

 NOTE:    This routine unlinks the dataset to make it inaccessible.  The space
          used by the dataset isn't reclaimed.  In order to remove the unused
          space, something like h5repack can be used to copy the remaining 
          valid dataspaces.
 
 RETURNS: SUCCESS- Table deleted 
          ERROR- Unable to delete the table
******************************************************************************/
static int ias_l0r_delete
(
    L0RIO *l0r, /* structure for the file used in I/O */
    const char *table_name /* I: Table name to delete */
)
{
    herr_t status;
    htri_t exists;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr =  l0r;

    if (hdfio_ptr->access_mode_metadata != IAS_WRITE)
    {
        IAS_LOG_ERROR("Ancillary data is not in write mode to delete %s",
            table_name );
        return ERROR;
    }

    exists = H5Lexists( hdfio_ptr->file_id_metadata, table_name,
        H5P_DEFAULT );
    if( exists != FALSE )
    {
       status = H5Ldelete( hdfio_ptr->file_id_metadata, table_name,
           H5P_DEFAULT );
       if (status < 0)
       {
           IAS_LOG_ERROR("Error deleting dataspace link %s", table_name );
           return ERROR;
       }
    }

    return SUCCESS;
}


/******************************************************************************
 NAME: ias_l0r_set_file_metadata

 PURPOSE: write the file level metadata to the file provided

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_file_metadata
(
     L0RIO *l0r, /* I: structure used with the l0r data */
     const IAS_L0R_FILE_METADATA *mta_file  /* I: Data to write */
)
{
    int status;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
 
    HDFIO *hdfio_ptr =   l0r;

    /* Make sure file is in write mode */
    if (hdfio_ptr->access_mode_metadata != IAS_WRITE)
    {
        IAS_LOG_ERROR("Access to metadata is not in write mode");
         return ERROR;
    }

    status = ias_l0r_mta_establish_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_metadata <= 0)
    {
        IAS_LOG_ERROR("Error establishing file access");
        return ERROR;
    }

    status = ias_l0r_hdf_establish_table("", TABLE_NAME_FILE_METADATA,
            hdfio_ptr->file_id_metadata, &hdfio_ptr->table_id_mta_file,
            TABLE_SIZE_FILE_METADATA, IAS_L0R_FILE_METADATA_SIZE,
            field_names_metadata_file, OFFSET_FILE_METADATA,
            field_type_metadata_file);
     
     if (status == ERROR || hdfio_ptr->table_id_mta_file < 0)
     {
        IAS_LOG_ERROR("Error establishing file level metadata table");
        return ERROR;
     }
    
     return ias_l0r_mta_write(l0r, 0, 1,
        TABLE_NAME_FILE_METADATA, IAS_L0R_FILE_METADATA_SIZE,
        OFFSET_FILE_METADATA, IAS_L0R_FILE_METADATA_SIZES, mta_file);
}

/******************************************************************************
 NAME: ias_l0r_set_interval_metadata

 PURPOSE: write the interval level metadata to the file provided

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_interval_metadata
(
    L0RIO *l0r, /* I: structure used with the l0r data */
    const IAS_L0R_INTERVAL_METADATA *mta_info_interval   /* I: Data to write */
)
{
    int status;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr =   l0r;

    /* Make sure file is in write mode */
    if (hdfio_ptr->access_mode_metadata != IAS_WRITE)
    {
        IAS_LOG_ERROR("Access to metadata is not in write mode");
         return ERROR;
    }

    status = ias_l0r_mta_establish_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_metadata <= 0)
    {
        IAS_LOG_ERROR("Error establishing file access");
        return ERROR;
    }

    status = ias_l0r_hdf_establish_table("", TABLE_NAME_INTERVAL_METADATA,
            hdfio_ptr->file_id_metadata, &hdfio_ptr->table_id_mta_interval,
            TABLE_SIZE_INTERVAL_METADATA, IAS_L0R_INTERVAL_METADATA_SIZE,
            field_names_metadata_interval, OFFSET_INTERVAL_METADATA,
            field_type_metadata_interval);

     if (status == ERROR || hdfio_ptr->table_id_mta_interval < 0)
     {
        IAS_LOG_ERROR("Error establishing interval metadata table");
        return ERROR;
     }

    return ias_l0r_mta_write(l0r, 0, 1,
        TABLE_NAME_INTERVAL_METADATA, IAS_L0R_INTERVAL_METADATA_SIZE,
        OFFSET_INTERVAL_METADATA, IAS_L0R_INTERVAL_METADATA_SIZES,
        mta_info_interval);
}

/******************************************************************************
 NAME: ias_l0r_delete_scene_metadata

 PURPOSE: delete the scene level metadata in the file provided

 RETURNS: SUCCESS- The data was successfully deleted 
          ERROR- The data could not be deleted
******************************************************************************/
int ias_l0r_delete_scene_metadata
(
    L0RIO *l0r /* I: structure used with the l0r data   */
)
{
    int status;
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    status = ias_l0r_delete(l0r, TABLE_NAME_SCENES_METADATA );

    return status;
}

/******************************************************************************
 NAME: ias_l0r_set_scene_metadata

 PURPOSE: write the scene level metadata to the file provided

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_scene_metadata
(
    L0RIO *l0r, /* I: structure used with the l0r data   */
    const int index, /* I: first scene to write */
    const int count, /* I: number of scenes to write */
    const IAS_L0R_SCENE_METADATA *mta_info_scene  /* I: Data to write */
)
{
    int i = 0;
    int status = SUCCESS;

    IAS_L0R_SCENE_METADATA *output_metadata = NULL; 

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr = l0r;

    /* Make sure file is in write mode */
    if (hdfio_ptr->access_mode_metadata != IAS_WRITE)
    {
        IAS_LOG_ERROR("Access to metadata is not in write mode");
         return ERROR;
    }

    status = ias_l0r_mta_establish_file(hdfio_ptr, TRUE);
    if (status == ERROR || hdfio_ptr->file_id_metadata < 0)
    {
        IAS_LOG_ERROR("Error establishing file access");
        return ERROR;
    }

    status = ias_l0r_hdf_establish_table("", TABLE_NAME_SCENES_METADATA,
            hdfio_ptr->file_id_metadata, &hdfio_ptr->table_id_mta_scene,
            TABLE_SIZE_SCENES_METADATA, IAS_L0R_SCENE_METADATA_SIZE,
            field_names_metadata_scene, OFFSET_SCENE_METADATA,
            field_type_metadata_scene);

    if (status == ERROR || hdfio_ptr->table_id_mta_scene < 0)
    {
        IAS_LOG_ERROR("Error establishing scene metadata table");
        return ERROR;
    }

    /*Create a copy of the metadata to use with the Y/N flags for sensor
        present instead of the 1/0 values used in the structure*/
    output_metadata = calloc(count, sizeof(IAS_L0R_SCENE_METADATA));

    if (output_metadata == NULL)
    {
        IAS_LOG_ERROR("Unable to allocate memory");
        return ERROR;
    }

    memcpy(output_metadata, mta_info_scene, 
        count * sizeof(IAS_L0R_SCENE_METADATA));

    for (i = 0; i < count; i++)
    {
        if (mta_info_scene[i].sensor_present_oli == 1)
        {
            output_metadata[i].sensor_present_oli = 'Y';
        }
        else
        {
            output_metadata[i].sensor_present_oli = 'N';
        }

        if (mta_info_scene[i].sensor_present_tirs == 1)
        { 
            output_metadata[i].sensor_present_tirs = 'Y';
        } 
        else 
        { 
            output_metadata[i].sensor_present_tirs = 'N';
        }
    }

    status = ias_l0r_mta_write(l0r, index, count,
        TABLE_NAME_SCENES_METADATA, IAS_L0R_SCENE_METADATA_SIZE,
        OFFSET_SCENE_METADATA, IAS_L0R_SCENE_METADATA_SIZES, output_metadata);

    free(output_metadata);

    return status;
}

/******************************************************************************
 NAME: ias_l0r_set_file_field_type

 PURPOSE; populates the field_type_metadata_file array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_set_file_field_type(HDFIO* hdfio_ptr)
{
    int i = 0;
    int j = 0;
    int string_bank = 0; /* Keep track of the strings that we have used
        out of the available strings.  This should never exceed 
        IAS_L0R_MAX_NUMBER_OF_STRINGS */

    /* Add one to lengths for a null terminator */
    size_t size_file_name = IAS_L0R_FILE_NAME_LENGTH+1;

    file_metadata_types[i] =
        hdfio_ptr->type_id_file_mta_strings[string_bank++]; 
        /* ancillary_file.name */
    if(H5Tset_size(file_metadata_types[i++], size_file_name) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }

    file_metadata_types[i] =
        hdfio_ptr->type_id_file_mta_strings[string_bank++]; 
        /* checksum_file.name */
    if(H5Tset_size(file_metadata_types[i++], size_file_name) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }

     /* Band files */
    for (j = 0 ; j < IAS_L0R_MAX_BAND_FILES; j++)
    {
        file_metadata_types[i] =
        hdfio_ptr->type_id_file_mta_strings[string_bank++];
        if(H5Tset_size(file_metadata_types[i++], size_file_name) < 0)
        {
            IAS_LOG_ERROR("Call to H5Tset_size failed");
            return NULL;
        }
    }
    file_metadata_types[i++] = H5T_STD_U8LE; /*uint8_t interval_files; */
    file_metadata_types[i] =
        hdfio_ptr->type_id_file_mta_strings[string_bank++]; 
        /* metadata_file.name */
    if(H5Tset_size(file_metadata_types[i++], size_file_name) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
 
    if (i != TABLE_SIZE_FILE_METADATA)
    {
        IAS_LOG_ERROR(
            "Error assigning types for file metadata. %d columns assigned"
            " and %d columns exist in the table",
            i, TABLE_SIZE_FILE_METADATA);
        return NULL;
    }

    if (string_bank > IAS_L0R_MAX_NUMBER_OF_STRINGS)
    {
        IAS_LOG_ERROR("Assigned %i string types, which is greater"
            " than the max of %i", string_bank,
            IAS_L0R_MAX_NUMBER_OF_STRINGS);
        return NULL;
    }

    return file_metadata_types;
}

/******************************************************************************
 NAME: ias_l0r_set_interval_field_type

 PURPOSE: set interval field_type needed to make INTERVAL
          table, hdf5 does not allow the field_type to be const

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_set_interval_field_type(HDFIO* hdfio_ptr)
{
    int i = 0;
    int j = 0;
    int string_bank = 0; /* Keep track of the strings that we have used
        out of the available strings.  This should never exceed 
        IAS_L0R_MAX_NUMBER_OF_STRINGS */

    size_t size_algorithm = IAS_L0R_ALGORITHM_NAME_LENGTH+1;
    size_t size_collection = IAS_L0R_COLLECTION_TYPE_LENGTH+1;
    size_t size_cpf = IAS_L0R_CPF_NAME_LENGTH+1;
    size_t size_data_type = IAS_L0R_DATA_TYPE_LENGTH+1;
    size_t size_date = IAS_L0R_DATE_LENGTH+1;
    size_t size_hostname = IAS_L0R_HOSTNAME_LENGTH+1;
    size_t size_interval = IAS_L0R_INTERVAL_ID_LENGTH+1;
    size_t size_is_version = IAS_SOFTWARE_VERSION_SIZE+1;
    size_t size_nadir = IAS_L0R_NADIR_LENGTH+1;
    size_t size_spacecraft = IAS_L0R_SPACECRAFT_ID_LENGTH+1;
    size_t size_sensor_id = IAS_L0R_SENSOR_ID_LENGTH+1;
    size_t size_station_id = IAS_L0R_STATION_ID_LENGTH+1;

    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++];
    if(H5Tset_size(interval_metadata_types[i++], size_date) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    /*char ancillary_start_date[IAS_L0R_DATE_LENGTH+1]*/
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++];
    if(H5Tset_size(interval_metadata_types[i++], size_date) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    /*char ancillary_stop_date[IAS_L0R_DATE_LENGTH+1]*/
    interval_metadata_types[i++] = H5T_STD_U32LE; /* attitude_points */
    interval_metadata_types[i++] = H5T_STD_U32LE; /* attitude_points_missing */
    interval_metadata_types[i++] = H5T_STD_U32LE; /* attitude_points_rejected */
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++]; 
        /*IAS_COLLECTION_TYPE_LEN*/
    if(H5Tset_size(interval_metadata_types[i++], size_collection) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }

    /* Add 16 doubles for each of the corner points
        2 (OLI/TIRS) * 2(lat/lng) * 4 corners */
    for (j = 0; j < 16; j++)
    {
        interval_metadata_types[i++] = H5T_IEEE_F64LE;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++];
        /* IAS_L0R_CPF_NAME_LENGTH */
    if(H5Tset_size(interval_metadata_types[i++], size_cpf) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i++] = H5T_STD_U32LE; /* crc_errors_oli */
    interval_metadata_types[i++] = H5T_STD_U32LE; /* crc_errors_tirs */
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++];
    if(H5Tset_size(interval_metadata_types[i++], size_data_type) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++]; 
        /*char date_acquired*/
    if(H5Tset_size(interval_metadata_types[i++], size_date) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i++] = H5T_STD_U32LE; /* ephemeris points */
    interval_metadata_types[i++] = H5T_STD_U32LE; /* ephemeris points_missing */
    interval_metadata_types[i++] = H5T_STD_U32LE;
    /* ephemeris points_rejected */
    interval_metadata_types[i++] = H5T_STD_U32LE; /* frames_filled_oli */
    interval_metadata_types[i++] = H5T_STD_U32LE; /* frames_filled_tirs */
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++]; 
        /* hostname */
    if(H5Tset_size(interval_metadata_types[i++], size_hostname) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i++] = H5T_STD_U8LE; /* image_quality_oli */
    interval_metadata_types[i++] = H5T_STD_U8LE; /* image_quality_tirs */
    interval_metadata_types[i++] = H5T_STD_U32LE; /* interval_frames_oli */
    interval_metadata_types[i++] = H5T_STD_U32LE; /* interval_frames_tirs */
    interval_metadata_types[i++] = H5T_STD_U8LE; /* interval_number */
    interval_metadata_types[i++] = H5T_STD_U8LE; /* interval_version */
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++]; 
        /*char is_version */
    if(H5Tset_size(interval_metadata_types[i++], size_is_version) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++];
        /*landsat_cal_interval_id*/
    if(H5Tset_size(interval_metadata_types[i++], size_interval) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++]; 
        /*landsat_interval_id*/
    if(H5Tset_size(interval_metadata_types[i++], size_interval) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++];
    if(H5Tset_size(interval_metadata_types[i++], size_nadir) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++];
    if(H5Tset_size(interval_metadata_types[i++], size_algorithm) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i++] = H5T_IEEE_F32LE; /* float roll_angle */
    interval_metadata_types[i++] = H5T_STD_U8LE;/* uint8_t satellite */
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++]; 
        /* char sensor_id */
    if(H5Tset_size(interval_metadata_types[i++], size_sensor_id) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++];
    if(H5Tset_size(interval_metadata_types[i++], size_spacecraft) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++]; 
        /* char start_time_oli */
    if(H5Tset_size(interval_metadata_types[i++], size_date) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++]; 
        /* char start_time_tirs */
    if(H5Tset_size(interval_metadata_types[i++], size_date) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++]; 
        /* char station_id */
    if(H5Tset_size(interval_metadata_types[i++], size_station_id) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++]; 
        /*stop_time_oli */
    if(H5Tset_size(interval_metadata_types[i++], size_date) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i] =
        hdfio_ptr->type_id_interval_mta_strings[string_bank++]; 
        /*stop_time_tirs */
    if(H5Tset_size(interval_metadata_types[i++], size_date) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    interval_metadata_types[i++] = H5T_STD_U32LE; /* time_code_errors_oli */
    interval_metadata_types[i++] = H5T_STD_U32LE; /* time_code_errors_tirs */
    interval_metadata_types[i++] = H5T_STD_U32LE; /* detector_map_id_tirs */
    interval_metadata_types[i++] = H5T_STD_U8LE; /* WRS ending row */
    interval_metadata_types[i++] = H5T_STD_U8LE; /* uint8_t wrs_scenes;  */
    interval_metadata_types[i++] = H5T_STD_U8LE; /* uint8_t wrs_scenes_full;*/
    interval_metadata_types[i++] = H5T_STD_U8LE; /* uint8_t wrs_scenes_partial*/
    interval_metadata_types[i++] = H5T_STD_U8LE; /* uint8_t wrs_starting_path;*/
    interval_metadata_types[i++] = H5T_STD_U8LE; /* uint8_t wrs_starting_row; */
    interval_metadata_types[i++] = H5T_STD_U8LE; /* uint8_t wrs_type;  */
    if (i != TABLE_SIZE_INTERVAL_METADATA)
    {
        IAS_LOG_ERROR(
            "Error assigning types for file metadata. %d columns assigned"
            " and %d columns exist in the table",
            i, TABLE_SIZE_INTERVAL_METADATA);
        return NULL;
    }

    if (string_bank > IAS_L0R_MAX_NUMBER_OF_STRINGS)
    {
        IAS_LOG_ERROR("Assigned %i string types, which is greater"
            " than the max of %i", string_bank,
            IAS_L0R_MAX_NUMBER_OF_STRINGS);
        return NULL;
    }

    return interval_metadata_types;
}

/******************************************************************************
 NAME: ias_l0r_set_scene_field_type

 PURPOSE: populates the field_type_metadata_scene array with HDF types.

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_set_scene_field_type(HDFIO* hdfio_ptr)
{
    int i = 0;
    int j = 0;
    int string_bank = 0; /* Keep track of the strings that we have used
        out of the available strings.  This should never exceed 
        IAS_L0R_MAX_NUMBER_OF_STRINGS */

    size_t size_day_night = IAS_L0R_DAY_NIGHT_LENGTH+1;
    size_t size_full_partial = IAS_L0R_SCENE_FULL_PARTIAL_LENGTH+1;
    size_t size_hostname = IAS_L0R_HOSTNAME_LENGTH+1;
    size_t size_nadir = IAS_L0R_NADIR_LENGTH+1;
    size_t size_scene = IAS_L0R_SCENE_ID_LENGTH+1;
    size_t size_time = IAS_L0R_TIME_LENGTH+1;
    size_t size_subsetter_version = IAS_SOFTWARE_VERSION_SIZE+1;
    size_t size_sensor_present = 1; /*Single char*/

    scene_metadata_types[i++] = H5T_STD_U16LE;/*uint8_t attitude_points; */
    scene_metadata_types[i++] = H5T_STD_U16LE;/*uint8_t attitude_pts_missing*/
    scene_metadata_types[i++] = H5T_STD_U16LE;/*uint8_t attitude_points_rej */
    /* Add 16 doubles for each of the corner points
        2 (OLI/TIRS) * 2(lat/lng) * 4 corners */
    for (j = 0; j < 16; j++)
    {
        scene_metadata_types[i++] = H5T_IEEE_F64LE;
    }
    /* end of corner points */
    scene_metadata_types[i++] = H5T_STD_U32LE; /* crc_errors */
    scene_metadata_types[i] =
        hdfio_ptr->type_id_scene_mta_strings[string_bank++];
    /* char date_acquired[TIME_LEN]*/
    if(H5Tset_size(scene_metadata_types[i++], size_time) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    scene_metadata_types[i] =
        hdfio_ptr->type_id_scene_mta_strings[string_bank++];
        /* IAS_L0R_DAY_NIGHT_LENGTH */
    if(H5Tset_size(scene_metadata_types[i++], size_day_night) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    scene_metadata_types[i++] = H5T_STD_U16LE; /* ephemeris_points */
    scene_metadata_types[i++] = H5T_STD_U16LE; /* ephemeris_pts_missing */
    scene_metadata_types[i++] = H5T_STD_U16LE; /* ephemeris_pts_rejected*/
    scene_metadata_types[i] =
        hdfio_ptr->type_id_scene_mta_strings[string_bank++];
        /*IAS_L0R_SCENE_FULL_PARTIAL_LENGTH */
    if(H5Tset_size(scene_metadata_types[i++], size_full_partial) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    scene_metadata_types[i] =
        hdfio_ptr->type_id_scene_mta_strings[string_bank++]; 
        /* IAS_L0R_HOSTNAME_LENGTH */
    if(H5Tset_size(scene_metadata_types[i++], size_hostname) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    scene_metadata_types[i++] = H5T_STD_I8LE; /*int8_t image_quality_oli;*/
    scene_metadata_types[i++] = H5T_STD_I8LE; /*int8_t image_quality_tirs;*/
    /*  Store the index of the landsat_scene_id into the global variable */
    landsat_scene_id_index = i;
    scene_metadata_types[i] =
        hdfio_ptr->type_id_scene_mta_strings[string_bank++]; 
        /* IAS_L0R_SCENE_ID_LENGTH */
    if(H5Tset_size(scene_metadata_types[i++], size_scene) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    scene_metadata_types[i++] = H5T_STD_U16LE; /*uint16_t missing_frames */
    scene_metadata_types[i] =
        hdfio_ptr->type_id_scene_mta_strings[string_bank++]; 
        /* IAS_L0R_NADIR_LENGTH */
    if(H5Tset_size(scene_metadata_types[i++], size_nadir) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }

    /* sensor_present_oli */
    scene_metadata_types[i] =
        hdfio_ptr->type_id_scene_mta_strings[string_bank++];
    if(H5Tset_size(scene_metadata_types[i++], size_sensor_present) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }  

    /* sensor_present_tirs */
    scene_metadata_types[i] =
        hdfio_ptr->type_id_scene_mta_strings[string_bank++];
    if(H5Tset_size(scene_metadata_types[i++], size_sensor_present) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }

    scene_metadata_types[i++] = H5T_IEEE_F32LE; /*float roll_angle */
    scene_metadata_types[i++] = H5T_IEEE_F64LE; /* scene center lat */
    scene_metadata_types[i++] = H5T_IEEE_F64LE; /* scene center lon */
    scene_metadata_types[i++] = H5T_STD_I32LE; /*scene_center_shift */
    scene_metadata_types[i++] = H5T_STD_U32LE; /* scene_start_frame_oli */
    scene_metadata_types[i++] = H5T_STD_U32LE; /* scene_stop_frame_oli */
    scene_metadata_types[i++] = H5T_STD_U32LE; /* scene_start_frame_tirs */
    scene_metadata_types[i++] = H5T_STD_U32LE; /* scene_stop_frame_tirs */

    scene_metadata_types[i] =
        hdfio_ptr->type_id_scene_mta_strings[string_bank++];    
        /* TIME_LEN */
    if(H5Tset_size(scene_metadata_types[i++], size_time) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    scene_metadata_types[i] =
        hdfio_ptr->type_id_scene_mta_strings[string_bank++];    
        /* TIME_LEN */
    if(H5Tset_size(scene_metadata_types[i++], size_time) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    scene_metadata_types[i] =
        hdfio_ptr->type_id_scene_mta_strings[string_bank++];
        /*subsetter version*/
    if(H5Tset_size(scene_metadata_types[i++], size_subsetter_version) < 0)
    {
        IAS_LOG_ERROR("Call to H5Tset_size failed");
        return NULL;
    }
    scene_metadata_types[i++] = H5T_IEEE_F64LE; /*float sun_azimuth; */
    scene_metadata_types[i++] = H5T_IEEE_F64LE; /*float sun_elevation;*/
    scene_metadata_types[i++] = H5T_STD_U8LE; /*uint16_t target_path */
    scene_metadata_types[i++] = H5T_STD_U16LE; /*uint16_t target_row */
    scene_metadata_types[i++] = H5T_STD_U16LE; /*uint16_t time_code_errors */
    scene_metadata_types[i++] = H5T_STD_U16LE; /*uint8_t wrs_path */
    scene_metadata_types[i++] = H5T_STD_U8LE; /*uint16_t wrs_row */
    scene_metadata_types[i++] = H5T_STD_U8LE; /*uint8_t wrs_scene_number */
    if (i != TABLE_SIZE_SCENES_METADATA)
    {
        IAS_LOG_ERROR(
            "Error assigning types for scene metadata. %d columns assigned"
            " and %d columns exist in the table",
            i, TABLE_SIZE_SCENES_METADATA);
        return NULL;
    }

    if (string_bank > IAS_L0R_MAX_NUMBER_OF_STRINGS)
    {
        IAS_LOG_ERROR("Assigned %i string types, which is greater"
            " than the max of %i", string_bank,
            IAS_L0R_MAX_NUMBER_OF_STRINGS);
        return NULL;
    }

    return scene_metadata_types;
}
