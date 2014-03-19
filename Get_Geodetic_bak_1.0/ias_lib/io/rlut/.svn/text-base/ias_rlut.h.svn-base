#ifndef IAS_RLUT_H
#define IAS_RLUT_H


#include "ias_types.h"


/* Dimensions for file attribute strings (from CPF DFCB).  These DO NOT
   include NULL termination -- that should be accounted for when strings
   with these dimensions are declared */
#define IAS_RLUT_DATE_LENGTH            26    /* Size of date string */
#define IAS_RLUT_STATUS_LENGTH          12    /* Size of status string */
#define IAS_RLUT_FILE_SOURCE_LENGTH    200    /* Size of file source string */
#define IAS_RLUT_DESCRIPTION_LENGTH   4000    /* Size of description string */

#define TIRS_DATASET_LENGTH 100
#define TIRS_DN_LUT         "DN_LUT"
#define TIRS_CORR_FAC       "Correction"



/* Main file attributes data structure declaration.  The +1 is for NULL
   termination.  Dates are assumed to be in the format
   'YYYY[-]MM[-]DD[Tt ]HH24[:]MI[:]SS'.*/
typedef struct ias_rlut_file_attributes
{
    char file_source[IAS_RLUT_FILE_SOURCE_LENGTH + 1];  /* Filename of baseline
                                                         RLUT used as source */
    char effective_date_begin[IAS_RLUT_DATE_LENGTH + 1];/* Effective start date
                                                         for this RLUT */
    char effective_date_end[IAS_RLUT_DATE_LENGTH + 1];  /* Effective stop date
                                                         for this RLUT */
    char status[IAS_RLUT_STATUS_LENGTH + 1];            /* ACTIVE, INACTIVE,
                                                         UNTESTED, TESTED,
                                                         VALIDATED, DENIED */
    char baseline_date[IAS_RLUT_DATE_LENGTH + 1];       /* Date the RLUT was
                                                         entered into baseline
                                                         production */
    char description[IAS_RLUT_DESCRIPTION_LENGTH + 1];  /* Text field intended
                                                         to describe rationale
                                                         for updated RLUT */
    int  version;                                       /* Version number of
                                                         effective date 
                                                         range */
} IAS_RLUT_FILE_ATTRIBUTES;


/* Main linearization parameter data structure definition */
typedef struct ias_rlut_linearization_params
{
    double cutoff_threshold_low;    /* Low DN range cutoff */
    double cutoff_threshold_high;   /* High DN range cutoff */
    double remap_coeff0_low;        /* Low range DN remapping coeffs */
    double remap_coeff1_low;
    double remap_coeff2_low;
    double remap_coeff0_mid;        /* Mid range DN remapping coeffs */
    double remap_coeff1_mid;
    double remap_coeff2_mid;
    double remap_coeff0_high;       /* High range DN remapping coeffs */
    double remap_coeff1_high;
    double remap_coeff2_high;
} IAS_RLUT_LINEARIZATION_PARAMS;


/* TIRS secondary linearization data structure */
typedef struct ias_rlut_tirs_secondary_linearization_params
{
    double *input_dn;      /* Table of input DN lookup values.  Access using
                              dn[detector * num_values + index] to get the
                              input value */
    double *output_correction; /* Table of secondary linearity corrections.
                                  Access using correction[detector *
                                  num_values + index] to get the output
                                  value. */
    int num_detectors;    /* Num detectors per SCA, (1st index) */
    int num_values;       /* Num distinct DN values used, (2nd index) */
} IAS_RLUT_TIRS_SECONDARY_LINEARIZATION_PARAMS;


/* Forward reference to RLUT IO data structure */
typedef struct IAS_RLUT_IO IAS_RLUT_IO;



/*                          FUNCTION PROTOTYPES                 */
IAS_RLUT_IO *ias_rlut_open_file
(
    const char *rlut_filename,            /* I: Name of RLUT file */
    IAS_ACCESS_MODE access_mode           /* I: Requested access mode */
);


int ias_rlut_close_file
(
    IAS_RLUT_IO *rlut_file                /* I: RLUT IO structure */
);


int ias_rlut_write_file_attributes
(
    const IAS_RLUT_IO *rlut_file,             /* I: Open RLUT file */
    const IAS_RLUT_FILE_ATTRIBUTES *file_attr /* I: Attributes to write */
);


int ias_rlut_read_file_attributes
(
    const IAS_RLUT_IO *rlut_file,         /* I: Open HDF5 RLUT file */
    IAS_RLUT_FILE_ATTRIBUTES *file_attr   /* O: Attributes read from file */
);


int ias_rlut_write_linearization_params
(
    const IAS_RLUT_IO *rlut_file,    /* I: Open RLUT file */
    const IAS_RLUT_LINEARIZATION_PARAMS *linearization_params,
                                     /* I: Array of detector-specific
                                        linearization parameters for the
                                        current band/SCA */
    int band_number,                 /* I: Current index into RLUT
                                        band array */
    int sca_number,                  /* I: Current SCA number */
    int num_detectors                /* I: Number of detectors in the
                                        current band/SCA */
);


IAS_RLUT_LINEARIZATION_PARAMS *ias_rlut_read_linearization_params
(
    const IAS_RLUT_IO *rlut_file,    /* I: Open RLUT file */
    int band_number,                 /* I: Current band number */
    int sca_number,                  /* I: Current SCA number */
    int num_detectors                /* I: Number of detectors in the
                                        current band/SCA */
);


int ias_rlut_is_rlut_file
(
    const char *rlut_filename        /* I: Name of (potential) RLUT file */
);

int ias_rlut_write_tirs_secondary_linearization_params
(
    const IAS_RLUT_IO *rlut_file,    /* I: Open RLUT file */
    int band_number,                 /* I: */
    int sca_number,                  /* I: */
    int num_detectors,               /* I: */
    IAS_RLUT_TIRS_SECONDARY_LINEARIZATION_PARAMS *linearization_params
                                     /* I: Pointer to a structure containing
                                           linearization parameters for all
                                           detectors of the band/SCA */
);

void ias_rlut_free_tirs_secondary_linearization
(
    IAS_RLUT_TIRS_SECONDARY_LINEARIZATION_PARAMS *params /* I: structure to
                                                               free */
);

IAS_RLUT_TIRS_SECONDARY_LINEARIZATION_PARAMS *
    ias_rlut_read_tirs_secondary_linearization_params
(
    const IAS_RLUT_IO *rlut, /* I: Open RLUT file */
    int band_number,         /* I: */
    int sca_number,          /* I: */
    int num_detectors        /* I: Number of detectors in the SCA */
);    

#endif
