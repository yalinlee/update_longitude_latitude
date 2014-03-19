#ifndef IAS_GCP_H
#define IAS_GCP_H

#include <limits.h>                      /* PATH_MAX definition */
#include <stdio.h>
#include "ias_types.h"
#include "ias_structures.h"

#define IAS_GCP_ID_SIZE 16               /* Length of a point id */
#define IAS_GCP_CHIP_NAME_SIZE 32        /* Length of chip name */
#define IAS_GCP_SOURCE_SIZE 5            /* Length of control source */
#define IAS_GCP_TYPE_SIZE 11             /* Length of control type */
#define IAS_GCP_PROJECTION_SIZE 14       /* Length of control projection */
#define IAS_GCP_SEASON_LEN 7             /* Length of season */
#define IAS_GCP_NUM_SEASONS 4            /* User may enter 4 seasons */
#define IAS_GCP_NUM_CHIP_SOURCES 10      /* User may enter 10 sources */
#define IAS_GCP_DATE_FORMAT "%02d-%02d-%04d"
#define IAS_GCP_DATE_LEN (sizeof("mm-dd-yyyy")) /* Length of date based on
                                            on date format */
#define IAS_GCP_ABS_REL_SIZE 4           /* Length of ABS or REL value */
#define IAS_GCP_DATA_TYPE_MAX_SIZE 10    /* Max size a string buffer for the
                                            data type needs to be */

typedef struct ias_gcp_record
{
    char point_id[IAS_GCP_ID_SIZE];         /* Ground control point ID */
    char chip_name[IAS_GCP_CHIP_NAME_SIZE]; /* Name of the chip image */
    double reference_line;                  /* Reference line */
    double reference_sample;                /* Reference sample */
    double latitude;                        /* True latitude of reference point
                                               in degrees */
    double longitude;                       /* True longitude of reference point
                                               in degrees */
    double projection_x;                    /* Projection x value of reference
                                               point in meters */
    double projection_y;                    /* Projection y value of reference
                                               point in meters */
    double elevation;                       /* Elevation at the GCP in meters */
    double pixel_size_x;                    /* Pixel size x value in meters */
    double pixel_size_y;                    /* Pixel size y value in meters */
    double chip_size_lines;                 /* Chip size in lines */
    double chip_size_samples;               /* Chip size in samples */
    char source[IAS_GCP_SOURCE_SIZE];       /* GCP source */
    char chip_type[IAS_GCP_TYPE_SIZE];      /* GCP type */
    char projection[IAS_GCP_PROJECTION_SIZE]; /* Chip projection */ 
    int zone;                               /* Projection UTM zone */
    char date[IAS_GCP_DATE_LEN];            /* Acquisition date form 
                                               mm-dd-yyyy */
    char absolute_or_relative[IAS_GCP_ABS_REL_SIZE]; /* ABS or REL */
    IAS_DATA_TYPE chip_data_type;           /* Image chip data type */
} IAS_GCP_RECORD;

typedef struct ias_gcp_results
{
    char point_id[IAS_GCP_ID_SIZE];     /* Ground Control Point ID */
    double reference_line;              /* Reference line */
    double reference_sample;            /* Reference sample */
    double latitude;                    /* True latitude */
    double longitude;                   /* True longitude */
    double elevation;                   /* Ground Control Point elevation */
    IAS_VECTOR gcp_position;            /* Satellite position in cartesian 
                                           space */
    double predicted_search_line;       /* Predicted line on the 1G image */
    double predicted_search_sample;     /* Predicted sample on the 1G image */
    double fit_line_offset;             /* Best-fit vertical (line)
                                           offsets of correlation peak */
    double fit_sample_offset;           /* Best-fit horizontal (sample) 
                                           offsets of correlation peak */
    int accept_flag;                    /* Acceptance flag of the ground 
                                           control point */
    double correlation_coefficient;     /* Correlation coefficient */
    int search_band;                    /* Band number of the search image */
    int search_sca;                     /* SCA number, 1 for SCA-combined 
                                           images */
    char chip_source[IAS_GCP_SOURCE_SIZE]; /* The chip source (DOQ, GLS) */
    int reference_band;                 /* Reference image band number */
    int reference_sca;                  /* Reference SCA number */
    char reference_name[PATH_MAX];      /* Reference image name with path */
    char search_name[PATH_MAX];         /* Full search image name with path */
    IAS_DATA_TYPE chip_data_type;       /* Image chip data type */
} IAS_GCP_RESULTS;

int ias_gcp_read_gcplib
(
    const char *gcplib_file_name, /* I: Name of the GCPLIB file */
    IAS_GCP_RECORD **gcp_lib,     /* O: Structure of chip information */
    int *num_gcp                  /* O: Number of ground control points */
);

int ias_gcp_read_gcplib_filtered
(
    const char *gcplib_file_name, /* I: Name of the GCPLIB file */
    /* For dates, 1 = January, etc. Year is YYYY */
    const int *begin_date,        /* I: Beginning date [0] = month [1] = year */
    const int *end_date,          /* I: Ending date [0] = month [1] = year */
                                  /* I: Season of chip */
    char season[IAS_GCP_NUM_SEASONS][IAS_GCP_SEASON_LEN],
                                  /* I: Source of chip */
    char chip_source[IAS_GCP_NUM_CHIP_SOURCES][IAS_GCP_SOURCE_SIZE],
    const char *chip_type,        /* I: Type of chip */
    IAS_GCP_RECORD **gcp_lib,     /* O: Structure of chip information */
    int *num_gcp                  /* O: Number of ground control points */
);

int ias_gcp_write_gcplib
(
    const char *gcplib_filename,       /* I: Output GCP filename */
    const IAS_GCP_RECORD *gcp_records, /* I: Structure of chip information */
    int number_of_gcps           /* I: Number of ground control points */
);

int ias_gcp_read_image 
(
    const char *chip_name,          /* I: Name of the image chip file */
    /* Chip lines and samples are doubles because that's how
       they're defined in the GCP Lib file */
    double chip_size_lines,         /* I: Expected number of chip lines */
    double chip_size_samples,       /* I: Expected number of chip samples */
    IAS_DATA_TYPE chip_data_type,   /* I: Image chip data type */
    float *chip_buffer              /* O: Buffer of chip image data converted
                                          to float */
);

int ias_gcp_write_image 
(
    const char *chip_name,      /* I: Name of the image chip file */
    int chip_size_lines,        /* I: Number of chip lines in data buffer */
    int chip_size_samples,      /* I: Number of chip samples in data buffer */
    IAS_DATA_TYPE dtype,        /* I: Data type for byte swapping if needed */
    const void *buf             /* I: Image chip data buffer to write to file */
);

int ias_gcp_read_correlation_results
(
    const char *gcp_data_filename,  /* I: GCP data file name */
    const double semi_major_axis,   /* I: CPF earth semi major axis constant */
    const double ellipticity,       /* I: CPF earth ellipticity constant */
    IAS_GCP_RESULTS **gcp_data,     /* O: GCP data records */
    int *num_gcp                    /* O: Number of GCPs read */
);

int ias_gcp_write_correlation_results 
(
    const char *gcp_data_filename,    /* I: Name of GCP data file to add to */
    const IAS_GCP_RESULTS gcp_data[], /* I: GCP data structures */
    int num_gcp                       /* I: Number of GCPs */
);

#endif
