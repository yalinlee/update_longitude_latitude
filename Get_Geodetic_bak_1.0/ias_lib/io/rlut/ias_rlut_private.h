#ifndef IAS_RLUT_PRIVATE_H
#define IAS_RLUT_PRIVATE_H

#include "hdf5.h"
#include "ias_types.h"
#include "ias_rlut.h"


/* Define the RLUT file format version */
#define IAS_RLUT_FORMAT_VERSION 1

/* Maximum number of fields representing all attribute parameters */
#define IAS_RLUT_ATTR_NFIELDS   7

/* Maximum number of fields representing all linearization parameters */
#define IAS_RLUT_PARAM_NFIELDS 11

/* String identifying file as a (Landsat 8-specific) RLUT file */
#define IAS_RLUT_FILE_TYPE "L8 RLUT Parameter File"

/* String sizes (DOES NOT include NULL termination -- declarations MUST
   add + 1 to include it) */
#define IAS_RLUT_BANDSCA_GROUP_NAME_LENGTH  38

/* Group name / table name / dataset names */
#define FILE_ATTRIBUTES_GROUP_NAME               "/FILE_ATTRIBUTES"
#define FILE_ATTRIBUTES_TABLE_NAME               "File Attributes"
#define FILE_ATTRIBUTES_DATASET_NAME             "Attribute Values"
#define LINEARIZATION_PARAMS_GROUP_NAME          "/LINEARIZATION_PARAMETERS"
#define LINEARIZATION_PARAMS_TABLE_NAME          "Parameters"
#define LINEARIZATION_PARAMS_DATASET_NAME        "Parameter Values"
#define TIRS_SECONDARY_LINEARIZATION_GROUP_NAME  "TIRS_SECONDARY_LINEARIZATION"


/* IAS_RLUT_IO data structure */
struct IAS_RLUT_IO
{
    char *filename;                    /* RLUT file name */
    hid_t file_id;                     /* Open RLUT file handle */
    int file_format_version;           /* Format version of current file --
                                          placeholder for future version
                                          compatibility */
};

/* IAS_RLUT2_IO data structure.  While this is identical to the IAS_RLUT_IO
   structure at this time, it uses a different structure to allow for future
   changes and also make sure users of the library don't try to mix RLUT and
   RLUT2 calls together. */
struct IAS_RLUT2_IO
{
    char *filename;                    /* RLUT file name */
    hid_t file_id;                     /* Open RLUT file handle */
    int file_format_version;           /* Format version of current file --
                                          placeholder for future version
                                          compatibility */
};


/*                  PRIVATE FUNCTION PROTOYTPES                          */
void ias_rlut_initialize_rlut_file
(
    IAS_RLUT_IO *rlut_file                  /* O:  HDF IO data structure */
);


int ias_rlut_build_linearization_params_table_description
(
    size_t offsets[IAS_RLUT_PARAM_NFIELDS],          /* O: Offsets into data
                                                        structure */
    const char *field_names[IAS_RLUT_PARAM_NFIELDS], /* O: Field names */
    hid_t field_types[IAS_RLUT_PARAM_NFIELDS],       /* O: Field data types */
    hid_t fields_to_close[IAS_RLUT_PARAM_NFIELDS],   /* O: List of fields to
                                                        be closed */
    size_t field_sizes[IAS_RLUT_PARAM_NFIELDS]       /* O: Field sizes */
);


int ias_rlut_build_file_attributes_table_description
(
    size_t offsets[IAS_RLUT_ATTR_NFIELDS],           /* O: Offsets into data
                                                        structure */
    const char *field_names[IAS_RLUT_ATTR_NFIELDS],  /* O: Field names */
    hid_t field_types[IAS_RLUT_ATTR_NFIELDS],        /* O: Field data types */
    hid_t fields_to_close[IAS_RLUT_ATTR_NFIELDS],    /* O: List of fields to
                                                        be closed */
    size_t field_sizes[IAS_RLUT_ATTR_NFIELDS]        /* O: Field sizes  */
);


void ias_rlut_cleanup_table_description
(
    hid_t *fields_to_close,             /* I: IDs to close */
    int num_fields                      /* I: Number of fields to close */
);

#endif
