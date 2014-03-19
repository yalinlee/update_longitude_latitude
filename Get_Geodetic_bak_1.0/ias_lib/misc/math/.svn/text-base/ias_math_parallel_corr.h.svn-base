#ifndef _IAS_MATH_PARALLEL_CORR_H_
#define _IAS_MATH_PARALLEL_CORR_H_

#include "ias_types.h" /* IAS_CORRELATION_FIT_TYPE definition */

/*
**
** TYPE DEFINITIONS
**
*/

typedef struct
{
    int valid;  /* set to non-zero if the correlation was valid */
    double fit_offset[2];/* array to hold the horizontal and vertical 
                            correlation fit offset */
    double strength;     /* strength of correlation */
    double est_err[3];/* Est horiz error, vert error, and h-v cross term in
                         best-fit offsets (3 values) */
    double diag_disp; /* Actual diagonal displacement from nominal tiepoint
                         location to location found by correlation */
} IAS_CORRELATION_RESULT_TYPE;
/* this type stores the results of a single correlation. */

/* forward declaration to the parallel correlator structure.  Nothing outside
   this library should need to access it. */
struct ias_parallel_correlator;

typedef struct ias_parallel_correlator IAS_PARALLEL_CORRELATOR_TYPE;
   
/*
**
** PROTOTYPES
**
*/

IAS_PARALLEL_CORRELATOR_TYPE *ias_math_init_parallel_correlator
(
    int check_edge_flag,  /* I: set to non-zero to check the corr edge flag */
    int check_mult_peak_flag,/* I: set to non-zero to check multiple peak flag*/
    int check_low_peak_flag, /* I: set to non-zero to check low peak flag */
    int check_max_disp_flag, /* I: set to non-zero to check max disp flag */
    int max_ref_chip_lines,  /* I: max lines in reference chip */
    int max_ref_chip_samples, /* I: max samples across reference chip */
    int max_search_chip_lines, /* I: max lines in search chip */
    int max_search_chip_samples, /* I: max samples across search chip */
    IAS_CORRELATION_RESULT_TYPE *results_ptr /* I: pointer to results array */
);

int ias_math_get_corr_chip_buffers
(
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr, /* I: parallel correlator */
    float **ref_buffer_ptr, /* O: pointer to reference chip buffer */
    float **search_buffer_ptr /* O: pointer to search chip buffer */
);

int ias_math_submit_chip_to_corr
(
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr, /* I: parallel correlator */
    int chip_index,          /* I: index to store results at in results array */
    float *search_image_ptr, /* I: search image */
    float *ref_image_ptr,    /* I: reference image */
    int search_size[2],      /* I: size of search image (samp,line) */
    int ref_size[2],         /* I: size of reference image (samp,line) */
    float min_corr,          /* I: minimum acceptable correlation strength */
    IAS_CORRELATION_FIT_TYPE fit_method,/* I: fit method to use */
    float max_disp,          /* I: maximum allowed diagnol displacement */
    float nominal_offset[2], /* I: nominal offset of UL corner of ref to search
                                   (samp,line) */
    int abs_corr_coeff_flag  /* I: flag to use the absolute of the correlation
                                   coefficients */    
);

int ias_math_close_parallel_correlator
(
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr /* I: parallel correlator */
);

int ias_math_parallel_correlator_wait_for_results
(
    IAS_PARALLEL_CORRELATOR_TYPE *correlator_ptr /* I: parallel correlator */
);

#endif
