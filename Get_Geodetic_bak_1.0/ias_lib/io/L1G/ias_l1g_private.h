#ifndef IAS_L1G_PRIVATE_H
#define IAS_L1G_PRIVATE_H

#include "hdf5.h"
#include "ias_l1g.h"
#include "ias_linked_list.h"

/* define the file format version.  This needs to be incremented any time an
   incompatible change is made to the format and special logic should be 
   added to allow reading the previous format. */
#define FILE_FORMAT_VERSION 1

/* define the table names */
#define BAND_METADATA_TABLE "Band Metadata"
#define FILE_METADATA_TABLE "File Metadata"

/* use a define for the compression flag to the various HDF5 routines that
   allow compression to make it easier to understand the code */
#define NO_COMPRESSION 0

/* This structure caches information about the band datasets in the image */
typedef struct band_dataset_cache_node
{
    IAS_LINKED_LIST_NODE node;  /* linked list node */
    int band_number;            /* band number for this record */
    IAS_DATA_TYPE data_type;    /* datatype in the band */
    int sca_count;              /* number of scas in the band */
    int line_count;             /* number of lines in the band */
    int sample_count;           /* number of samples in the band */
} BAND_DATASET_CACHE_NODE;

/* macro to make it easy to delete the band dataset cache data */
#define BAND_DATASET_LINKED_LIST_DELETE(list) \
    ias_linked_list_delete((list), offsetof(BAND_DATASET_CACHE_NODE, node));

typedef struct band_metadata_cache_node
{
    IAS_LINKED_LIST_NODE node;      /* linked list node */
    IAS_L1G_BAND_METADATA metadata; /* band metadata for this cached entry */
} BAND_METADATA_CACHE_NODE;

/* macro to make it easy to delete the band metadata cache data */
#define BAND_METADATA_LINKED_LIST_DELETE(list) \
    ias_linked_list_delete((list), offsetof(BAND_METADATA_CACHE_NODE, node));

/* the main L1GIO structure.  This structure is only visible internally.
   External interfaces use the forward reference to the L1GIO typedef to hide
   the contents of the structure from applications. */
struct L1GIO
{
    int file_format_version;    /* format version of the current file (to allow
                                   backwards support for old versions) */
    char *filename;             /* name of the HDF file */
    IAS_ACCESS_MODE access_mode;/* access mode for the file */

    hid_t file_id;              /* file id */
    int file_metadata_present;  /* flag to indicate the file metadata table
                                   should be present in the file */
    int band_metadata_valid;    /* flag to indicate the cached band metadata is
                                   valid */
    int band_metadata_in_file;  /* flag to indicate the presence of the band
                                   metadata table in the file */
    int band_metadata_records_in_file; /* records in table in file */
    IAS_LINKED_LIST_NODE band_datasets; /* cache of the band dataset info */
    IAS_LINKED_LIST_NODE band_metadata; /* cache of band metadata */
    IAS_LINKED_LIST_NODE open_bands; /* linked list of open bands */
};

/* structure for tracking open bands */
struct L1G_BAND_IO
{
    hid_t band_id;              /* dataset id for the currently opened band */
    hid_t band_dataspace_id;    /* currently open band dataset dataspace id */
    hid_t band_memory_data_type;/* HDF datatype for the current band */
    int band_number;            /* band number of currently open band */
    int sca_count;              /* SCAs in currently open band */
    int line_count;             /* lines in currently open band */
    int sample_count;           /* samples in currently open band */
    struct L1GIO *l1g_file;     /* pointer to L1G file structure */
    IAS_LINKED_LIST_NODE node;  /* linked list node */
};


int ias_l1g_add_band_to_cache
(
    L1GIO *hdf_file,        /* I: pointer to HDF5 file structure */
    int band_number,        /* I: band number being added */
    IAS_DATA_TYPE data_type,/* I: data type in the band */
    int sca_count,          /* I: number of scas in the band */
    int line_count,         /* I: number of lines in the band */
    int sample_count        /* I: number of samples in the band */
);

#endif
