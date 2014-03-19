#ifndef PM_LOCAL_H
#define PM_LOCAL_H

#include <stdio.h>
#include <sys/types.h>
#include "ias_pixel_mask.h"
#include "ias_linked_list.h"
#include "ias_types.h"


/* These values are used in the read and write routines to denote the
   end of a pixel mask, start of housekeeping data, and end of file in the
   binary array file. */
#define IAS_PM_EOM 8888
#define IAS_PM_SOH 8989
#define IAS_PM_EOF 9999


/* A define to indicate the number of end-of-mask, start-of-housekeeping,
   and/or end-of-file marker values.  Markers are currently read/written
   as 1D integer arrays of this length, with all elements set to IAS_PM_EOM,
   IAS_PM_SOH, or IAS_PM_EOF as appropriate.  */
#define IAS_PM_NUMBER_OF_MARKER_VALUES 4

/* Use a buffer size that is a multiple of the number of integers used to
   represent a span (4, based on the current definition of the pixel mask).
   This should improve overall IO performance by requiring fewer fread/fwrite
   calls to read/write the span data from/to the pixel mask file.  */
#define IAS_PM_MAX_BUFFER_SIZE   8192


/* Internal mask housekeeping data structure. As a single pixel mask is
   written to the file, allocate a record to add to it.  */
typedef struct ias_pixel_mask_file_housekeeping
{
    off_t starting_data_offset;          /* Starting byte of mask data
                                            in bytes */
    int band_number;                     /* Band number */
    int sca_number;                      /* SCA number */
    int mask_index;                      /* Location in mask file */
    unsigned int mask_data_size;         /* Size of mask data set in file
                                            (in bytes) */
} IAS_PIXEL_MASK_FILE_HOUSEKEEPING;

/* Pixel mask file IO data structure. */
struct IAS_PIXEL_MASK_IO
{
    char *pixel_mask_file_name;            /* Name of pixel mask file */
    FILE *fptr;                            /* Pixel mask file handle */
    IAS_PIXEL_MASK_FILE_HOUSEKEEPING *hk;  /* Mask housekeeping data
                                              (i.e. band/SCA numbers,
                                              starting byte offsets in file) */
    IAS_ACCESS_MODE access_mode;           /* Specified access mode
                                              (IAS_WRITE, IAS_READ,
                                               IAS_UPDATE) */
    int number_of_masks_present;           /* Current number of masks in the
                                              pixel mask file */
};

/* Linked list node structure */
typedef struct ias_pixel_mask_node
{
    struct ias_pixel_mask_node *prev_node;
    struct ias_pixel_mask_node *next_node;
    IAS_PIXEL_MASK_SPAN  span_structure;
} IAS_PIXEL_MASK_NODE;

/* Pixel mask structure */
struct IAS_PIXEL_MASK
{
    int band;
    int sca;
    int num_of_detectors;
    int num_of_pixels;
    IAS_LINKED_LIST_NODE **detector_lut;
};

/* Interator structure */
struct IAS_PIXEL_MASK_ITERATOR
{
    struct IAS_PIXEL_MASK *pixel_mask;
    int next_detector;
    int next_span_starting_pixel_index;
    IAS_LINKED_LIST_NODE *curr_node;
    IAS_PIXEL_MASK_ITERATOR_TYPE iterator_type;
    PIXEL_MASK_TYPE mask_set;
    PIXEL_MASK_TYPE complemented_mask_set;
};

/* Functions intended to be visible only within the pixel mask library. */
IAS_PIXEL_MASK *ias_pm_read_mask_data_from_file
(
    const IAS_PIXEL_MASK_IO *pm_file,  /* I: Open pixel mask file */
    int band_number,                   /* I: Band number of requested
                                          mask */
    int sca_number                     /* I: SCA number of requested
                                          mask */
);

int ias_pm_confirm_marker_in_file
(
    const IAS_PIXEL_MASK_IO *pm_file,   /* I: Open pixel mask file */
    int expected_marker_value           /* I: Marker value to confirm */
);

int ias_pm_write_mask_data_to_file
(
    IAS_PIXEL_MASK_IO *pm_file,           /* I: Open pixel mask file */
    IAS_PIXEL_MASK *mask                  /* I: Pixel mask data */
);

int ias_pm_write_marker_to_file
(
    IAS_PIXEL_MASK_IO *pm_file,           /* I: Open pixel mask file */
    int marker_value                      /* I: Marker written to file */
);


#endif
