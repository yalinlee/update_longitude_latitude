#ifndef IAS_MISC_GCP_RESIDUALS_H
#define IAS_MISC_GCP_RESIDUALS_H

#include "ias_gcp.h"
#include "ias_miscellaneous.h"

#define GCP_RECLEN 250
#define IAS_GCP_RESIDUALS_FIRST_ITER      0  /* Pre-fit residual iteration
                                                number to use */
#define IAS_GCP_RESIDUALS_LAST_ITER      -1  /* Post-fit residual iteration
                                                number to use */

/* GCP_RESIDUAL is GCP residual information for one point read from the 
   precision residual file */
typedef struct ias_misc_gcp_residual
{
    char point_id[IAS_GCP_ID_SIZE]; /* GCP ID (See GCP Correlation ADD for
                                       details) */
    int band_number;                /* Band number */
    double predicted_line;          /* Predicted L1G line location */
    double predicted_sample;        /* Predicted L1G sample number */
    double seconds_from_epoch;      /* Seconds from the epoch time */
    double latitude;                /* GCP position in degrees */
    double longitude;               /* GCP position in degrees */
    double height;                  /* GCP position in meters */
    double across_track_angle;      /* LOS angle (delta) in degrees */
    double along_track_residual;    /* Residual on psi converted to meters */
    double across_track_residual;   /* Residual on delta converted to meters */
    double residual_y;              /* Residual in Y/line direction in meters */
    double residual_x;              /* Residual in X/sample direction; meters */
    int outlier_flag;               /* 0 if the point is an outlier,
                                       1 if it's valid */
    char gcp_source[IAS_GCP_SOURCE_SIZE]; /* GCP source: DOQ or GLS */
} IAS_MISC_GCP_RESIDUAL;

/* -------------- Routine prototypes --------------- */
int ias_misc_read_gcp_residuals
(
    const char *residuals_filename,     /* I: Precision residual file name */
    int iteration_number,               /* I: The iteration number to get */
    IAS_MISC_GCP_RESIDUAL **gcp_res,    /* O: GCP residual information */
    int *number_of_residuals            /* O: # of residuals read from file */
);

int ias_misc_write_gcp_residuals
(
    FILE *res_fptr,             /* I: Pointer to the rfile (residual file) */
    int num_gcp,                /* I: The # of GCPs used in the solution */
    int iter_num,               /* I: The iteration #; -1 is final iteration */
    const IAS_MISC_GCP_RESIDUAL *gcp_res,       /* I: Array of GCP information
                                                      structures */
    const IAS_REPORT_HEADER *header_information /* I: Header information */
);

#endif
