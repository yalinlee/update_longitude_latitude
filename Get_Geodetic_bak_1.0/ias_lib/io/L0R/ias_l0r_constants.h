/*-----------------------------------------------------------------------------

NAME: ias_l0r_constants.h

PURPOSE: Defines constants used throughout the L0R Library

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/

#ifndef IAS_L0R_CONSTANTS
#define IAS_L0R_CONSTANTS

#define IAS_L0R_ALGORITHM_NAME_LENGTH 50
#define IAS_L0R_CHECKSUM_LENGTH 32
#define IAS_L0R_COLLECTION_TYPE_LENGTH 50
#define IAS_L0R_CPF_NAME_LENGTH 25
#define IAS_L0R_DATA_TYPE_LENGTH 20
#define IAS_L0R_DATE_LENGTH 26
#define IAS_L0R_DAY_NIGHT_LENGTH 5
#define IAS_L0R_FILEDATE_LENGTH 32
#define IAS_L0R_FILE_NAME_LENGTH 256 /* length of the L0R filename */
#define IAS_L0R_HOSTNAME_LENGTH 20
#define IAS_L0R_INTERVAL_ID_LENGTH 24
#define IAS_L0R_MAX_BAND_FILES 18
#define IAS_L0R_NADIR_LENGTH 9
#define IAS_L0R_SCENE_FULL_PARTIAL_LENGTH 7
#define IAS_L0R_SCENE_ID_LENGTH 21
#define IAS_L0R_SENSOR_ID_LENGTH 8
#define IAS_L0R_SPACECRAFT_ID_LENGTH 10
#define IAS_L0R_STATION_ID_LENGTH 3
#define IAS_L0R_TIME_LENGTH 26
#define IAS_L0R_TIRS_BANDS 3
#define IAS_L0R_TIRS_SCAS 3
#define IAS_L0R_IMAGE_DIMENSIONS  3 /* the number of dimensions of the image */
#define IAS_L0R_IMAGE_DIMENSION_DETECTOR  2 /* dimension used for detectors */
#define IAS_L0R_IMAGE_DIMENSION_LINE  1 /* the dimension representing lines */
#define IAS_L0R_IMAGE_DIMENSION_SCA  0 /* the dimension used to represent SCAs*/

/* Max number of strings that can be present in a single dataset.
    Currently the most needed is 21 in the file metadata,
    add in a few extra  */
#define IAS_L0R_MAX_NUMBER_OF_STRINGS 25

/******************************************************************************
* Constants returned by routines which try to locate something.  These are
*   used instead of returning SUCCESS or FAILURE because OBJECT_NOT_FOUND is
*   a different condition than a FAILURE.  A FAILURE indicates it can not be
*   determined if the object exists or not
******************************************************************************/
#define IAS_L0R_OBJECT_FOUND 3
#define IAS_L0R_OBJECT_NOT_FOUND 4

#endif
