/*------------------------------------------------------------------------------
NAME: ias_rlut2.h

PURPOSE:
    The detectors have a non-linear relationship between the radiance viewed
    and the DN value output by the detectors.  The response of the detectors
    must be linearized as part of the radiometric calibration process.

    This alternative linearization algorithm uses lookup tables (LUT) that
    covers the full range of input values, using linear interpolation between
    the sparsely defined input values.  For each input value provided, the
    output correction value is provided.  These tables have been provided by
    the instrument vendors.

ALGORITHM REFERENCES:
    Alternate Response Linearization ADD
------------------------------------------------------------------------------*/
#ifndef IAS_RLUT2_H
#define IAS_RLUT2_H

#include "ias_types.h"
#include "ias_rlut.h"

/* Forward declaration for the RLUT2 IO structure */
typedef struct IAS_RLUT2_IO IAS_RLUT2_IO;

/*------------------------------------------------------------------------------
 * This structure contains two 2-dimensional arrays of floating point values.
 * These array contain a single row for each of the detectors in an SCA,
 * 'dn[detector]' and 'correction[detector]'.  An application will read one of
 * these structures from the RLUT2 file that contains DN values which are used
 * as a lookup table.  When a DN value from a detector is located within a row,
 * the index location is used in the parallel array 'correction' to find either
 * the linearity correction (LIN) or the non-uniformity correction (NUC) that
 * should be applied to the DN value received from the spacecraft.
 * 
 * The LIN and NUC corrections will have their own, independent set of DN lookup
 * values.  This allows these two corrections to use a different compression
 * rate to obtain the desired accuracy.
 *
 *----------------------------------------------------------------------------*/
typedef struct ias_rlut2_table
{
    float *input_dn;      /* Table of input DN lookup values.  Access using
                             dn[detector * num_values + index] to get the
                             input value */
    float *output_correction;/* Table of linearity or nonuniformity corrections.
                             Access using correction[detector * num_values
                              + index] to get the output value. */
    int band_num;         /* Band this structure contains data for */
    int sca_num;          /* SCA this structure contains data for */
    int num_detectors;    /* Num detectors per SCA, (1st index) */
    int num_values;       /* Num distinct DN values used, (2nd index) */
} IAS_RLUT2_TABLE;


/*------------------------------------------------------------------------------
 * Function Prototypes
 *----------------------------------------------------------------------------*/
int ias_rlut2_close_file
(
    IAS_RLUT2_IO *rlut /* I/O: An open RLUT2 structure */
);

void ias_rlut2_free_table
(
    IAS_RLUT2_TABLE *table /* I/O: An RLUT2 table */
);

int ias_rlut2_is_rlut2_file
(
    const char *path /* I: Path to an RLUT2 file */
);

IAS_RLUT2_IO *ias_rlut2_open_read
(
    const char *path /* I: Path to an RLUT2 file */
);

IAS_RLUT2_IO *ias_rlut2_open_write
(
    const char *path /* I: Path to an RLUT2 file */
);

int ias_rlut2_read
(
    IAS_RLUT2_IO *fd,        /* I/O: File descriptor opened for writing */
    int is_linearity,        /* I: Values to read, TRUE=LIN, FALSE=NUC */
    int band_num,            /* I: Band number of interest */
    int sca_num,             /* I: SCA number of interest */
    IAS_RLUT2_TABLE **table  /* O: RLUT2 table */
);

int ias_rlut2_realloc_table
(
    int band_num,            /* I: Band number of interest */
    int sca_num,             /* I: SCA number of interest */
    int num_values,          /* I: Number of values per detector */
    IAS_RLUT2_TABLE **table  /* I/O: Ptr to a dynamically allocated table */
);

int ias_rlut2_write
(
    IAS_RLUT2_IO *fd,       /* I/O: File descriptor opened for writing */
    int is_linearity,       /* I: TRUE=LIN values, FALSE=NUC values */
    IAS_RLUT2_TABLE *table  /* I: RLUT2 table */
);

int ias_rlut2_write_file_attributes
(
    const IAS_RLUT2_IO *rlut_file,            /* I: Open RLUT file */
    const IAS_RLUT_FILE_ATTRIBUTES *file_attr /* I: Attributes to write */
);

int ias_rlut2_read_file_attributes
(
    const IAS_RLUT2_IO *rlut_file,         /* I: Open HDF5 RLUT file */
    IAS_RLUT_FILE_ATTRIBUTES *attr         /* O: Attributes to read */
);

#endif
