/******************************************************************************
Name: ias_sensor_find_los_vector

Purpose: Find the line of sight vector in sensor coordinates.  LOS is created
    from sensor specific information stored within the line-of-sight model.
    Adjustments are made according to SCA related parameters (NOMINAL) or
    detector specific parameters (ACTUAL and EXACT) or based on the detector
    offsets.  There is also a LOS calculation made based on a maximum offset,
    which is really an arbritrary number (MAXIMUM).  This maximum prevents
    calculations done with small detector offsets when generating odd/even
    adjustments that are to be placed in the resampling grid.

RETURN VALUE:
    SUCCESS     successful computations and LOS vector returned through param
    ERROR       error calculating LOS vector; LOS vector param not set

******************************************************************************/
#include <math.h>
#include "ias_structures.h"
#include "ias_math.h"
#include "ias_logging.h"
#include "ias_sensor_model.h"
#include "local_defines.h"

/* Provide a prototype for the math.h round function to avoid compiling the
   code with -std=c99, since this routine prototype is only included in the
   header used with the compiler -std=c99 option active, which we are not
   using. By including the prototype here, the routine in the library is now
   accessible even without compiling for c99. */
double round(double);

int ias_sensor_find_los_vector
(
    int sca_index,                     /* I: Input point SCA index (0-based) */
    double detector,                   /* I: Input point detector number */
    IAS_SENSOR_DETECTOR_TYPE type,     /* I: Type of detector model to use */
    const IAS_SENSOR_BAND_MODEL *band, /* I: band model structure */
    IAS_VECTOR *losv                   /* O: Line of sight vector */
)
{
    double norm_det;        /* Legendre normalized detector number  */
    double x;               /* Look vector X coordinate */
    double y;               /* Look vector Y coordinate */
    double maglos;          /* LOS vector magnitude */
    double maglos_inverse;  /* Used to allow mult rather than div to optimize */
    double ifov_x;          /* OLI IFOV (band dependent) */
    double ifov_y;          /* OLI IFOV (band dependent) */
    double x_off;           /* Along-track detector offset */
    double y_off;           /* Across-track detector offset */
    IAS_VECTOR losvec;      /* Local LOS vector */
    int ndet;               /* Nearest integer detector */

    IAS_SENSOR_SCA_MODEL *sca = &band->scas[sca_index];

    /* Compute the normalized detector number */
    norm_det = 2.0 * detector / ((double)sca->detectors - 1) - 1.0;

    /* Evaluate the Legendre polynomials from the model */
    x = ias_math_eval_legendre(norm_det, sca->sca_coef_x, 4);
    y = ias_math_eval_legendre(norm_det, sca->sca_coef_y, 4);

    /* Do extra processing for ACTUAL and EXACT detectors */
    if (type != IAS_NOMINAL_DETECTOR)
    {
        ndet = (int)floor(detector + 0.5);
        if ((ndet < 0) || (ndet >= sca->detectors))
        {
            IAS_LOG_ERROR("Calculated invalid detector number (%d) for SCA %d",
                ndet, sca_index);
            return ERROR;
        }

        ifov_x = band->sampling_char.along_ifov; 
        ifov_y = band->sampling_char.across_ifov; 
        x_off = sca->detector_offsets_along_track[ndet];
        y_off = sca->detector_offsets_across_track[ndet];
        
        if (type == IAS_MAXIMUM_DETECTOR) /* Only along track adjustment */
        {
            x += ifov_x * band->sampling_char.maximum_detector_delay;
        }
        else if (type == IAS_EXACT_DETECTOR)
        {
            x += ifov_x * x_off;
            y += ifov_y * y_off;
        }
        else /* ACTUAL */
        {
            x_off = round(x_off);
            y_off = round(y_off);
            x += ifov_x * x_off;
            y += ifov_y * y_off;
        }
    }

    /* Construct the NOMINAL LOS vector */
    losvec.x = x;
    losvec.y = y;
    losvec.z = 1.0;

    /* Normalize for output */
    maglos = ias_math_compute_vector_length(&losvec);
    maglos_inverse = 1.0 / maglos;
    losv->x = losvec.x * maglos_inverse;
    losv->y = losvec.y * maglos_inverse;
    losv->z = losvec.z * maglos_inverse;

    return SUCCESS;
}
