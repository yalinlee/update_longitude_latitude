#ifndef IAS_PIXEL_MASK_H
#define IAS_PIXEL_MASK_H
#include "ias_linked_list.h"
#include "ias_types.h"

/* This defines the artifact mask type and a value for internal use of no
   value.  There are memsets currently in use in the library.  If the pixel
   mask type is changed to a larger size the memset to set spans of pixels
   will not work.  Those will have to change to straight loop copies. */
typedef unsigned char PIXEL_MASK_TYPE;
#define IAS_PIXEL_MASK_DATA_TYPE IAS_BYTE;
#define PM_NOVALUE              0x00

/* External definition of pixel mask file IO data structure. */
typedef struct IAS_PIXEL_MASK_IO IAS_PIXEL_MASK_IO;

/* External definitions of pixel mask and iterator */
typedef struct IAS_PIXEL_MASK IAS_PIXEL_MASK;
typedef struct IAS_PIXEL_MASK_ITERATOR IAS_PIXEL_MASK_ITERATOR;

/* Iterator types */
typedef enum
{
    IAS_PM_ALL,      /* Return a span for all types of pixels regardless of
                        whether it is masked.  For pixels not included in
                        a node in the pixel mask, a span with a pixel mask
                        of zero is returned. */

    IAS_PM_INCLUDE,  /* Return spans that have pixel masks that match the 
                        mask passed to the create iterator call.  For
                        example:

                        PIXEL_MASK_TYPE mask = 0x1 | 0x2;
                        it = ias_pm_get_iterator( pm, IAS_PM_INCLUDE, mask );

                        will create an iterator that will return all pixel
                        masks that have bit 1 or bit 2 set. */

    IAS_PM_EXCLUDE,  /* Returns all spans, with the exception of spans that
                        only have the mask bits set.  For example:

                        PIXEL_MASK_TYPE mask = 0x1 | 0x2;
                        it = ias_pm_get_iterator( pm, IAS_PM_EXCLUDE, mask );

                        will create an iterator that will exclude masks that
                        only have bits 1 and 2 set.  It will return a
                        pixel mask that is (0x1 | 0x4) but not a mask that
                        is (0x1) or one that is (0x1 | 0x2) */
} IAS_PIXEL_MASK_ITERATOR_TYPE;

/* The globally visible pixel mask structure */
typedef struct IAS_PIXEL_MASK_SPAN
{
    IAS_LINKED_LIST_NODE node;
    int  detector_index;
    int  starting_pixel_index;
    int  length_of_span;
    PIXEL_MASK_TYPE pixel_mask;
} IAS_PIXEL_MASK_SPAN;

/* Publically accessible structure that can hold the band and SCA numbers for
   any given pixel mask.  An array of these can be populated with the
   band/SCA combinations present in all of the masks in a pixel
   mask file.  */
typedef struct ias_pixel_mask_band_sca_list
{
    int band_number;             /* Band number of mask */
    int sca_number;              /* SCA number of mask */
} IAS_PIXEL_MASK_BAND_SCA_LIST;


/*---------------------------------------------------------------------
   Routine Prototypes
---------------------------------------------------------------------*/
IAS_PIXEL_MASK_IO *ias_pm_open_pixel_mask
(
    const char *pixel_mask_file_name,  /* I: Name of mask file */
    IAS_ACCESS_MODE access_mode        /* I: IAS access mode */
);

int ias_pm_close_pixel_mask
(
    IAS_PIXEL_MASK_IO *pm_file         /* I: Currently open pixel mask file */
);

IAS_PIXEL_MASK *ias_pm_create
(
    int band_number,            /* I: Band number               */
    int sca,                    /* I: SCA number                */
    int num_of_detectors,       /* I: Number of detectors       */
    int num_of_pixels           /* I: Number of pixels          */
);

void ias_pm_destroy
(
    IAS_PIXEL_MASK *pm    /* I: Pointer to pixel mask */
);

int ias_pm_add_pixels
(
    IAS_PIXEL_MASK *pixel_struct,   /* I: Pointer to a pixel mask structure */
    int detector_index,             /* I: Detector index */
    int start_pixel_index,          /* I: Starting pixel */
    int length,                     /* I: Length of span */
    PIXEL_MASK_TYPE mask            /* I: Pixel mask for the span */
);

int ias_pm_get_mask_at
(
    IAS_PIXEL_MASK *pixel_struct,   /* I: Pointer to a pixel mask structure */
    int detector_index,             /* I: Detector index */
    int pixel_index,                /* I: Location of pixel */
    PIXEL_MASK_TYPE *mask           /* O: pixel mask */
);

int ias_pm_get_mask_index
(
    IAS_PIXEL_MASK **pm_array,      /* I: Pixel mask array to search */
    int number_of_masks,            /* I: Total number of masks in array */
    int band_number,                /* I: Current 1-based band number */
    int sca_number                  /* I: Current 1-based SCA number */
);

IAS_PIXEL_MASK_ITERATOR *ias_pm_get_iterator
(
    IAS_PIXEL_MASK *pixel_mask, /* I: Pointer to a pixel mask structure */
    IAS_PIXEL_MASK_ITERATOR_TYPE iterator_type, /* I: Type of iterator to
                                                      create */
    PIXEL_MASK_TYPE mask_set    /* I: Pixel mask set */
);

IAS_PIXEL_MASK_ITERATOR *ias_pm_get_iterator_at_detector
(
    IAS_PIXEL_MASK *pixel_mask, /* I: Pointer to a pixel mask structure */
    IAS_PIXEL_MASK_ITERATOR_TYPE iterator_type, /* I: Type of iterator to
                                                      create */
    PIXEL_MASK_TYPE mask_set,   /* I: Pixel mask set */
    int detector                /* I: Detector (0-based) to set the iterator
                                      next detector value */
);

void ias_pm_destroy_iterator
(
    IAS_PIXEL_MASK_ITERATOR *iterator /* I: Pointer to iterator structure */
);

int ias_pm_get_next_span
(
    IAS_PIXEL_MASK_ITERATOR *iterator, /* I: Pointer to the iterator */
    IAS_PIXEL_MASK_SPAN *span          /* I/O: Span structure */
);

int ias_pm_get_image
(
    IAS_PIXEL_MASK *pixel_mask,  /* I: Pointer to a pixel mask structure */
    PIXEL_MASK_TYPE *image_data  /* I/O: Pointer to allocated array for
                                    image data */
);

int ias_pm_write_array_to_file
(
    IAS_PIXEL_MASK *pixel_mask_array[],
                                        /* I: Array of pixel masks */
    int num_of_masks,                   /* I: Number of pixel masks in array */
    const char *output_file_name        /* I: File name of output */
);

int ias_pm_write_single_mask_to_file
(
    IAS_PIXEL_MASK_IO *pm_file,         /* I: Open pixel mask file */
    IAS_PIXEL_MASK *mask                /* I: Pixel mask structure for
                                           a single band/SCA */
);

IAS_PIXEL_MASK **ias_pm_read_array_from_file
(
    const char *input_file_name,   /* I: File name of input */
    int *num_of_masks              /* O: Number of pixel masks read */
);

IAS_PIXEL_MASK *ias_pm_read_single_mask_from_file
(
    const IAS_PIXEL_MASK_IO *pm_file,  /* I: Open pixel mask file */
    int band_number,                   /* I: Requested band number of mask */
    int sca_number                     /* I: Requested SCA number of mask */
);

int ias_pm_is_detector_flagged
(
    IAS_PIXEL_MASK  *pm,        /* I: Pixel mask for the current
                                   band/SCA */
    int   det_index,            /* I: (0-based) detector index */
    PIXEL_MASK_TYPE mask        /* I: Mask (individual or
                                   composite) to check  */
);

int ias_pm_get_image_subset
(
    IAS_PIXEL_MASK *pixel_mask,         /* I: Pixel mask */
    int start_detector,                 /* I: Starting detector 0-based */
    int start_line,                     /* I: Starting line number 0-based */
    int number_of_detectors,            /* I: Number of detectors */
    int number_of_lines,                /* I: Number of lines */
    PIXEL_MASK_TYPE *image_mask_subset  /* O: Image subset */
);

int ias_pm_get_sca_number
(
    IAS_PIXEL_MASK *pm      /* I: Pixel mask */
);

int ias_pm_get_band_number
(
    IAS_PIXEL_MASK *pm      /* I: Pixel mask */
);

int ias_pm_get_number_of_detectors
(
    IAS_PIXEL_MASK *pm      /* I: Pixel mask */
);

int ias_pm_get_number_of_pixels
(
    IAS_PIXEL_MASK *pm      /* I: Pixel mask */
);

int ias_pm_get_file_band_sca_list
(
    const IAS_PIXEL_MASK_IO *pm_file,    /* I: Open pixel mask file */
    IAS_PIXEL_MASK_BAND_SCA_LIST **band_sca_list,
                                         /* O: Populated band/SCA list */
    int *list_size                       /* O: Number of items in list
                                            (equivalent to the number of
                                            masks present in the file) */
);

#endif
