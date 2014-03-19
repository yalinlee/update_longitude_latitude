#ifndef IAS_MISCELLANEOUS_H
#define IAS_MISCELLANEOUS_H

#include <stdio.h>          /* FILE definition */
#include "ias_db.h"
#include "ias_l0r.h"
#include "ias_structures.h"
#include "ias_types.h"
#include "ias_const.h"
#include "ias_geo.h"

/* Define the attribute to use in files to store the file type. Used by the
   ias_misc_check_file_type routine. */
#define IAS_FILE_TYPE_ATTRIBUTE "File Type"

/* Acquisition date/time information. Include OLI and TIRS here -- it's
   possible both will image during any given data collect */
typedef struct l0r_acquisition_datetime_info
{
    /* Date/time OLI and/or TIRS images were acquired */

    /* OLI imaging start time */
    char start_time_oli[IAS_L0R_DATE_LENGTH + 1];

    /* OLI imaging stop time */
    char stop_time_oli[IAS_L0R_DATE_LENGTH + 1];

    /* TIRS imaging start time */
    char start_time_tirs[IAS_L0R_DATE_LENGTH + 1];

    /* TIRS imaging stop time */
    char stop_time_tirs[IAS_L0R_DATE_LENGTH + 1];

    double total_time_oli;      /* Elapsed time (sec) of OLI collect */
    double total_time_tirs;     /* Elapsed time (sec) of TIRS collect */
} L0R_ACQUISITION_DATETIME_INFO;


/* General RPS/GPS specific header data structures */
/* The RPS report header content was developed mostly by the developer
   with little in put from CalVal. It is believed CalVal will want to tweak
   the content, so some of the values that were suspected may eventually be
   wanted by CalVal were included in the structure at this time, since the
   data was being collected from the various sources and it was very little
   additional work to populate a few more fields now to prepare for potential
   future adjustments to the header content. Added a comment about this in
   the header file.
   A few bug trackers that relate to this: 2766, 2768, 2776 */
typedef struct rps_report_header
{
    /* Image start/stop times and elapsed acquisition times */
    L0R_ACQUISITION_DATETIME_INFO acq_datetime;

    /* Sensor-specific information.  Add desired TIRS temperatures
       when these become known */
    int ms_integration_time;                   /* OLI MS band integration
                                                  time (ms)*/
    int pan_integration_time;                  /* OLI PAN band integration
                                                  time (ms) */
    int current_detector_select_table;         /* Current OLI pixel map */
    int image_data_truncation_setting;         /* Flag indicating which
                                                  12 bits */
    int num_oli_frames;
    int num_tirs_frames;
    int average_oli_frame_rate;                /* Frame rate calculated as
                                                  the ratio of the number
                                                  to the difference in
                                                  image and stop times (OLI) */
    int average_tirs_frame_rate;               /* Frame rate calculated as
                                                  the ratio of the number
                                                  to the difference in
                                                  image and stop times
                                                  (TIRS)*/
    float oli_fpm7_start_temp;                 /* SCA 7 temperature at
                                                  start of ancillary data
                                                  collect */
    float oli_fpm7_stop_temp;                  /* SCA 7 temperature at end
                                                  of ancillary data
                                                  collect */
    float oli_fpm14_start_temp;                /* SCA 14 temperature at
                                                  start of ancillary data
                                                  collect */
    float oli_fpm14_stop_temp;                 /* SCA 14 temperature at
                                                  end of ancillary data
                                                  collect */
    float oli_fpa_window_start_temp;           /* FPA window temperature
                                                  at start of ancillary
                                                  data collect */
    float oli_fpa_window_stop_temp;            /* FPA window temperature
                                                  at end of ancilary data
                                                  collect */
    float oli_fpe_chassis_start_temp;          /* FPE chassis temperature
                                                  at start of ancillary
                                                  data collect */
    float oli_fpe_chassis_stop_temp;           /* FPE chassis temperature
                                                  at end of ancillary data
                                                  collect */
    uint16_t diffuser_resolver_position;       /* OLI solar diffuser panel
                                                  position */

    /* TIRS blackbody calibrator temperatures */
    float tirs_blackbody_1_start_temp;
    float tirs_blackbody_1_stop_temp;
    float tirs_blackbody_2_start_temp;
    float tirs_blackbody_2_stop_temp;
    float tirs_blackbody_3_start_temp;
    float tirs_blackbody_3_stop_temp;
    float tirs_blackbody_4_start_temp;
    float tirs_blackbody_4_stop_temp;

    /* TIRS cryocooler temperatures */
    float tirs_cryocooler_1_start_temp;
    float tirs_cryocooler_1_stop_temp;
    float tirs_cryocooler_2_start_temp;
    float tirs_cryocooler_2_stop_temp;
}   RPS_REPORT_HEADER;

/* Create a forward reference to the IAS_REPORT_HEADER data structure.
   This will be defined in a local header file */
typedef struct IAS_REPORT_HEADER IAS_REPORT_HEADER;

/* Prototype functions to initialize and write out the common report header */
IAS_REPORT_HEADER *ias_misc_initialize_report_header
(
    const char *l0r_filename,       /* I: Name of L0R file */
    const char *work_order_id,      /* I: Work order ID string */
    const char *algorithm_name      /* I: Name of characterization algorithm */
);

IAS_REPORT_HEADER *ias_misc_copy_report_header
(
    const IAS_REPORT_HEADER *header_to_copy   /* I: Standard report header
                                                    data to copy */
);

int ias_misc_write_report_header
(
    FILE *rpt_fptr,                        /* I: Summary report file */
    const IAS_REPORT_HEADER *header        /* I: Report header data */
);

void ias_misc_free_report_header
(
    IAS_REPORT_HEADER *header   /* I: Pointer to header memory block */
);

/* Function prototypes to set values in IAS_REPORT_HEADER */
void ias_misc_set_header_band_number
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    int band_number                 /* I: Band number to set */
);

void ias_misc_set_header_sca_number
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    int sca_number                  /* I: SCA number to set */
);

void ias_misc_set_header_acquisition_date
(
   IAS_REPORT_HEADER *header,       /* I/O: Report header to modify */
   const char *acquisition_date     /* I: Acquisition date */
);

void ias_misc_set_header_algorithm_name
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *algorithm_name      /* I: Name of application to set */
);

int ias_misc_set_header_pathrow
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    int path,                       /* I: WRS2 path to set */
    int row                         /* I: WRS2 row to set */
);

int ias_misc_set_header_starting_ending_pathrow
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    int starting_path,              /* O: Interval starting path */
    int starting_row,               /* O: Interval start row */
    int ending_row                  /* O: Interval end row */
);

void ias_misc_set_header_cpf_name
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *cpf_name            /* I: Processing CPF name */
);

void ias_misc_set_header_collection_type
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *collection_type     /* I: Collection type */
);

void ias_misc_set_header_l0r_filename
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *l0r_filename        /* L0R file name */
);

void ias_misc_set_header_l1r_filename
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *l1r_filename        /* I: L1R file name */
);

void ias_misc_set_header_spacecraft_id
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *spacecraft_id       /* I: Spacecraft ID */
);

void ias_misc_set_header_sensor_id
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *sensor_id           /* I: Sensor ID */
);

/* Function prototypes to return values from IAS_REPORT_HEADER */
int ias_misc_get_header_band_number
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

int ias_misc_get_header_sca_number
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_algorithm_name
(
    IAS_REPORT_HEADER *header   /* I Report header */
);

void ias_misc_get_header_pathrow
(
    IAS_REPORT_HEADER *header,  /* I: Report header */
    int *path,                  /* O: Returned WRS path */
    int *row                    /* O: Returned WRS row */
);

void ias_misc_get_header_target_pathrow
(
    IAS_REPORT_HEADER *header,  /* I: Report header */
    int *path,                  /* O: Returned WRS target path */
    int *row                    /* O: Returned WRS target row */
);

void ias_misc_get_header_starting_ending_pathrow
(
    IAS_REPORT_HEADER *header,  /* I: Report header */
    int *starting_path,         /* O: Interval start path */
    int *starting_row,          /* O: Interval start row */
    int *ending_row             /* O: Interval end row */
);

char *ias_misc_get_header_work_order_id
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_cpf_name
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_l0r_filename
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_l1r_filename
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_collection_type
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_spacecraft_id
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_sensor_id
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_ias_sw_version
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_ingest_sw_version
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_l0rp_subsetter_sw_version
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_date_acquired
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_ancillary_start_time
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

char *ias_misc_get_header_ancillary_stop_time
(
    IAS_REPORT_HEADER *header   /* I: Report header */
);

/* Prototypes for the GPS report header */
IAS_REPORT_HEADER *ias_misc_initialize_gps_report_header
(
    const char *l1g_filename,       /* I; Path and name of L1G file */
    const char *l0r_filename,       /* I: Name of L0R file, or NULL pointer */
    const char *work_order_id,      /* I: Work order ID string */
    const char *algorithm_name      /* I: Name of characterization algorithm */
);

int ias_misc_write_gps_report_header
(
    FILE *rpt_fptr,                        /* I: Summary report file */
    const IAS_REPORT_HEADER *header        /* I: Report header data */
);

/* Prototypes to initialize and write out RPS report header information */
int ias_misc_initialize_rps_report_header
(
    const char *l0r_filename,       /* I: Name of L0R file */
    RPS_REPORT_HEADER *rps_header   /* O: Populated RPS_REPORT_HEADER
                                          data structure */
);

int ias_misc_write_rps_report_header
(
    char *collection_type,           /* I: Collection type */
    char *cpf_name,                  /* I: Processing CPF name */
    char *sensor_id,                 /* I: Sensor ("OLI", "TIRS", or
                                           "OLI_TIRS") */
    RPS_REPORT_HEADER rps_header,    /* I: RPS-specific header data to write */
    FILE *rpt_fptr                   /* I: Report file handle */
);

/* Additional miscellaneous function prototypes */
int ias_misc_check_year_month_day
(
    int year,     /* I: Year (YYYY) */
    int month,    /* I: Month (MM) */
    int day       /* I: Day (DD) */
);

int ias_misc_convert_data_type_to_string
(
    IAS_DATA_TYPE data_type,        /* I: IAS data type */
    const char **data_type_string   /* O: The data type converted to a string */
);

int ias_misc_convert_string_to_data_type
(
    const char *data_type_string,   /* I: The string data type */
    IAS_DATA_TYPE *data_type        /* O: The string to its IAS data type */
);

char *ias_misc_convert_to_lowercase 
(
    char *string_ptr  /* I/O: pointer to string to convert */
);

char *ias_misc_convert_to_uppercase 
(
    char *string_ptr  /* I/O: pointer to string to convert */
);

int ias_misc_get_sizeof_data_type
(
    IAS_DATA_TYPE dtype,    /* I: The IAS data type to get number of bytes */
    int *type_nbytes        /* O: Number of bytes for the given type */
);

const char *ias_misc_get_software_version();

int ias_misc_get_timestamp
(
    const char *p_format,    /* I: format of output timestamp */
    int stampsize,           /* I: size of timestamp for input */
    char *p_stamp            /* O: timestamp for output */
);

int ias_misc_read_elevation_at_line_sample
(
    const IAS_IMAGE *dem,       /* I: DEM image */
    double line,                /* I: Line in input space (0-rel) */
    double samp,                /* I: Sample in input space (0-rel) */
    double pixel_size_y,        /* I: Line in output space (0-rel) */
    double pixel_size_x,        /* I: Sample in output space (0-rel) */
    double *elevation           /* O: Terrain table & related info */
);

int ias_misc_read_single_band_l1g 
(
    const char *l1g_filename,    /* I: Name of the L1G image file */
    int band_number,             /* I: Band number for band to read */
    IAS_DATA_TYPE *data_type,    /* O: Data type of the read out band */
    IAS_IMAGE *image_ptr         /* O: Populated from the L1G image file */
);

int ias_misc_read_dem 
(
    const char *dem_name,                 /* I: name of DEM to read */
    const IAS_CORNERS *image_corners_ptr, /* I: Corners of image */
    int lines_in_image,                   /* I: Lines in image */
    int samples_per_image_line,           /* I: Samples in an image line */
    double image_pixsize,                 /* I: Input image pixel size */
    IAS_IMAGE *dem_ptr                    /* O: Populated from the L1G file */
);

void ias_misc_split_filename 
(
    const char *filename,       /* I: Name of file to split */
    char *directory,            /* O: Directory portion of file name */
    char *root,                 /* O: Root portion of the file name */
    char *extension             /* O: Extension portion of the file name */
);

/* The following routines add support for 2D arrays that can be dynamically
   allocated.  The arrays are created by allocating a 1D block of memory for
   the data storage and a 1D array of void pointers that point to the first
   member in each row of data in the 2D array.  The void** returned by the
   allocate_2d_array routine is a pointer to the array of void pointers.

   Example use:
        A 2D array of doubles would be allocated and used as follows:

        int rows = calculate number of rows...
        int columns = calculate number of columns...
        double **array = ias_misc_allocate_2d_array(rows, columns, 
                                sizeof(double));
        double value = array[2][3];
        ias_misc_free_2d_array((void **)array);

    Notes:
        - Since the array contents require two memory references to access the
          data, these 2D arrays should not be used in performance critical
          code.  Instead, 1D arrays should be used and the indices calculated
          manually.  For example, it would be best to not use this 2D array
          implementation for imagery.
        - Arrays allocated using this library need to be freed using this
          library due to how it manages the memory.  The library hides extra
          information about the array that can only be accessed by the 
          ias_misc_free_2d_array and ias_misc_get_2d_array_size routines.
        - It would be possible to add a routine to allow access to the 1D array
          of data that underlies the 2D pointer array and allow manually
          calculating the indices into the array for performance critical use
          of the library.
*/

void **ias_misc_allocate_2d_array
(
    int rows,            /* I: Number of rows for the 2D array */
    int columns,         /* I: Number of columns for the 2D array */
    size_t member_size   /* I: Size of the 2D array element */
);

int ias_misc_get_2d_array_size
(
    void **array_ptr,   /* I: Pointer returned by the alloc routine */
    int *rows,          /* O: Pointer to number of rows */
    int *columns        /* O: Pointer to number of columns */
);

int ias_misc_free_2d_array
(
    void **array_ptr    /* I: Pointer returned by the alloc routine */
);

int ias_misc_get_char_id
(
    struct ias_db_connection *dbase,   /* I: Database to use */
    const char *table_name,            /* I: Work order common table to query */
    const char *column_name,           /* I: Column name of WO common ID */
    const char *work_order_id,         /* I: Work order ID to char */
    char *char_id                      /* O: Characterization ID, as a string */
);

/* Function to parse an "IAS-compatible" date/time string */
int ias_misc_parse_datetime_string
(
    const char *datetime_string,            /* I: Date/time string */
    IAS_DATETIME_FORMAT_TYPE format_type,   /* I: Either L0R-format or
                                                  CPF/BPF/RLUT-format */
    IAS_DATETIME *time                      /* I/O: Decomposed date/time
                                                    data */
);

int ias_misc_check_file_type
(
    const char *filename,           /* I: Input file name */
    const char *expected_file_type  /* I: Expected file type */
);

IAS_PROCESSING_SYSTEM_TYPE ias_misc_get_processing_system();

int ias_misc_write_envi_header
(
    const char *image_filename, /* I: Full path name of the image file */
    const IAS_PROJECTION *proj_info, /* I: Optional projection info, set to 
                                           NULL if not known or needed */
    const char *description,    /* I: Optional description, set to NULL if not 
                                      known or needed */
    int lines,                  /* I: Number of lines in the data */
    int samples,                /* I: Number of samples in the data */
    int bands,                  /* I: Number of bands in the data */
    double upper_left_x,        /* I: Optional upper-left X coordinate, set to 
                                      0.0 if not known or needed (requires
                                      proj_info) */
    double upper_left_y,        /* I: Optional upper-left Y coordinate, set to 
                                      0.0 if not known or needed (requires
                                      proj_info) */
    double projection_distance_x, /* Optional pixel size in X projection, set
                                     to 0.0 if not known or needed (requires
                                     proj_info) */
    double projection_distance_y, /* Optional pixel size in Y projection, set 
                                     to 0.0 if not known or needed (requires
                                     proj_info) */
    const char *band_names,     /* I: Optional single string for all band names,
                                      set to NULL if not known or needed */
    IAS_DATA_TYPE data_type     /* I: The IAS type of the data */
);

#endif  /* ifndef  IAS_MISCELLANEOUS_H */
