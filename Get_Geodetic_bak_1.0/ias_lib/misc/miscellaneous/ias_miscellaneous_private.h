#ifndef IAS_MISCELLANEOUS_PRIVATE_H
#define IAS_MISCELLANEOUS_PRIVATE_H

#include "/usr/include/linux/limits.h"  /* XQian */

#include "ias_const.h"
#include "ias_l0r.h"
#include "ias_structures.h"
#include "ias_types.h"
#include "ias_miscellaneous.h"

/* Define date/time string lengths. Used when converting time to string. */
#define DATE_STRLEN 20
#define TIME_STRLEN 20

#define MISC_ALGORITHM_NAME_LENGTH 100
#define IAS_PROCESSING_CENTER_LENGTH 30
#define SCHEMA_LENGTH 30

/* IAS report header information. This structure is only visible internal to
   ias_misc_ functions. External RPS/GPS interfaces will use the forward
   reference to the IAS_REPORT_HEADER typedef to access the structure contents.
   For members that are considered "optional" information in a report, publicly
   accessible "getter" and/or "setter" functions will need to be implemented in
   the Miscellaneous library to retrieve values initialized by
   ias_misc_initialize_report_header(), or to change their values outside of
   any call to ias_misc_initialize_report_header().
   Also used for the GPS specific report header, since much of the same fields
   are used. */
struct IAS_REPORT_HEADER
{
    /* File names */
    char l0r_filename[PATH_MAX]; /* L0R file name, no path */
    char l1g_filename[PATH_MAX]; /* L1G file name, no path;used for GPS hdr */
    char l1r_filename[PATH_MAX]; /* L1R file name, no path */
    char cpf_name[PATH_MAX];     /* CPF file name, no path */

    /* Date scene/interval was acquired */
    char date_acquired[IAS_L0R_DATE_LENGTH + 1];
    char work_order_id[IAS_WORKORDER_ID_SIZE]; /* Work order ID string */

    /* IAS processing center */
    char processing_center[IAS_PROCESSING_CENTER_LENGTH + 1];

    /* Ingest software version this interval was processed with */
    char ingest_software_version[IAS_SOFTWARE_VERSION_SIZE + 1];

    /* IAS software version */
    char ias_software_version[IAS_SOFTWARE_VERSION_SIZE + 1];

    /* For individual Earth scenes, subsetter software version */
    char l0rp_subsetter_software_version[IAS_SOFTWARE_VERSION_SIZE + 1];

    /* Name of IAS characterization app */
    char algorithm_name[MISC_ALGORITHM_NAME_LENGTH + 1]; 

    /* Unique landsat ID for an individual scene */
    char landsat_scene_id[IAS_L0R_SCENE_ID_LENGTH + 1];

    /* Unique Landsat ID for an interval */
    char landsat_interval_id[IAS_L0R_INTERVAL_ID_LENGTH + 1];

    /* Type of collection (e.g. "EARTH", "SOLAR", etc) */
    char collection_type[IAS_COLLECT_TYPE_SIZE + 1];

    /* Spacecraft and sensor ID */
    char spacecraft_id[IAS_SPACECRAFT_NAME_LENGTH + 1];
    char sensor_id[IAS_SENSOR_NAME_LENGTH + 1];

    /* Start time for first ancillary data record acquisition */
    char ancillary_start_time[IAS_L0R_DATE_LENGTH + 1];

    /* Stop time for last ancillary data record acquisition */
    char ancillary_stop_time[IAS_L0R_DATE_LENGTH + 1];

    /* Database schema used during processing */
    char database_schema[SCHEMA_LENGTH + 1];

    int is_earth_scene;     /* Flag indicating whether report is for an
                               individual Earth scene or not */
    int wrs_type;           /* WRS system ID (currently WRS2) */
    int scene_number;       /* Scene number within the main interval
                               (Earth only) */
    int num_scenes;         /* Number of scenes in the main interval */
    int interval_version;   /* Interval version number (starting from 0) */
    int target_path;        /* Target path for an individual Earth scene */
    int target_row;         /* Target row for an individual Earth scene */
    int path;               /* Nominal scene center path */
    int row;                /* Nominal scene center row */
    int starting_path;      /* Starting path for an interval */
    int starting_row;       /* Starting row for an interval */
    int ending_row;         /* Ending row for an interval */
    int band_number;        /* (1-based) band number */
    int sca_number;         /* (1-based) SCA number */
    double roll_angle;      /* Off-nadir roll angle; used for GPS header */
};

#endif
