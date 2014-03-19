/*************************************************************************
Name: ias_los_model_lunar_projection

Purpose: The ias_los_model_lunar_projection module constructs and manages a
    pseudo-inertial coordinate system that tracks the apparent motion of the
    moon during an imaging interval. It is pseudo-inertial in the sense that it
    uses inertial right ascension/declination coordinates, but these
    coordinates are adjusted so as to be relative to the time-varying apparent
    position of the moon. The adjustment is based on a reference lunar position
    taken at the center of the imaging interval. Inertial lines of sight
    observed at other image times are adjusted by the difference in the lunar
    position at the time of imaging relative to the lunar position at the
    reference time. This allows the output space lunar coordinate system to
    remain registered to the lunar target over the time period it takes to
    image the moon. Note that apparent lunar motion is due to both actual
    motion of the moon in its orbit around the Earth, and motion of the
    spacecraft.
 
Notes:
    Includes the following routines:
        ias_los_model_create_lunar_projection
        ias_los_model_free_lunar_projection
        ias_los_model_transform_lunar_projection
        ias_los_model_get_moon_center
        find_reference_location (local routine)

**************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "ias_types.h"
#include "logging_channel.h" /* define the debug logging channel */
#include "ias_logging.h"
#include "ias_los_model.h"
#include "ias_math.h"

/* The IAS_LUNAR_PROJECTION structure is used to track information for a
   lunar projection transformation for a specific band, sca, detector type
   and model. */
struct IAS_LUNAR_PROJECTION
{
    const IAS_LOS_MODEL *model; /* LOS model for this projection */
    int band_index;     /* band index for this projection */
    int sca_index;      /* sca index for this projection */
    IAS_SENSOR_DETECTOR_TYPE dettype; /* detector type for this projection */
    double ref_rasc;    /* Reference apparent right ascension of the moon */
    double ref_dec;     /* Reference declination of the moon */
    double ref_dist;    /* Reference Earth-moon distance in kilometers */
    double unit_scale;  /* Scale for output coordinate units */

    /* The following members are used to optionally cache the moon position 
       for each of the lines in the image */
    int img_lines;      /* Number of entries in arrays */
    double *img_rasc;   /* Array of right ascensions for each image line */
    double *img_dec;    /* Array of declinations for each image line */
    double *img_dist;   /* Array of distances for each image line */
};

/*************************************************************************
Name: ias_los_model_free_lunar_projection

Purpose: Frees a lunar projection

Returns: Nothing

**************************************************************************/
void ias_los_model_free_lunar_projection
(
    IAS_LUNAR_PROJECTION *proj  /* I: pointer to structure to free */
)
{
    if (proj != NULL)
    {
        free(proj->img_rasc);
        free(proj->img_dec);
        free(proj->img_dist);
        free(proj);
    }
}

/*************************************************************************
Name: find_reference_location

Purpose: Finds the reference location to use as the rough center of the 
    collected imagery.  It uses the rough center of the first band present
    in the model.

Returns: SUCCESS or ERROR

**************************************************************************/
static int find_reference_location
(
    const IAS_LOS_MODEL *model,       /* I: model information */
    int *ref_band_index,              /* O: reference band index */
    int *ref_sca_index,               /* O: reference sca index */
    int *ref_line,                    /* O: reference line index */
    int *ref_detector_index           /* O: reference detector index */
)
{
    int band_index;

    /* Look for the first band present in the model.  It isn't important which
       one it uses as long as the same one is used every time for the current
       model and it is close to the center of the image. */
    for (band_index = 0; band_index < model->sensor.band_count; band_index++)
    {
        IAS_SENSOR_BAND_MODEL *band = &model->sensor.bands[band_index];

        if (band->band_present)
        {
            /* Found a band that is present, so use it as the reference, along
               with an SCA close to the center */
            *ref_band_index = band_index;
            *ref_sca_index = band->sca_count / 2;
            *ref_line = band->frame_count * band->sampling_char.lines_per_frame
                      / 2;
            *ref_detector_index = band->scas[*ref_sca_index].detectors / 2;
            return SUCCESS;
        }
    }

    /* no band found (which should never happen) */
    IAS_LOG_ERROR("No bands found in the model");
    return ERROR;
}

/*************************************************************************
Name: ias_los_model_create_lunar_projection

Purpose: Creates a lunar projection with the indicated parameters.  If
    requested with the use_cache parameter, it creates a cache of moon 
    positions for each line of the image since calculating the moon position
    is slow.  The cache should be used if a lot of projection transformations
    need to be done.

Returns: Pointer to the created projection or NULL if an error.

**************************************************************************/
IAS_LUNAR_PROJECTION *ias_los_model_create_lunar_projection
(
    const IAS_LOS_MODEL *model,  /* I: model information */
    int band_index,              /* I: Band index */
    int sca_index,               /* I: SCA index */
    IAS_SENSOR_DETECTOR_TYPE dettype, /* I: Detector type */
    double unit_scale,           /* I: Scale for output coordinate units */
    int use_cache                /* I: Flag to use caching to speed up
                                       transformations when a lot of
                                       transformations will be done */
)
{
    int ref_band_index;     /* Reference location on the focal plane */
    int ref_sca_index;
    int ref_image_line;
    int ref_detector;
    int line;               /* Line in the input image */
    double samp;            /* Sample in the input image */
    IAS_LUNAR_PROJECTION *proj = NULL;
    double pi = ias_math_get_pi();

    /* Get the reference location */
    if (find_reference_location(model, &ref_band_index, &ref_sca_index,
            &ref_image_line, &ref_detector) != SUCCESS)
    {
        IAS_LOG_ERROR("Could not find the focal plane reference location");
        return NULL;
    }
    samp = ref_detector;

    /* Allocate the memory for the cached information */
    proj = calloc(1, sizeof(*proj));
    if (!proj)
    {
        IAS_LOG_ERROR("Failed allocating memory Lunar projection cache");
        return NULL;
    }

    /* Fill in the constants in the projection structure */
    proj->model = model;
    proj->band_index = band_index;
    proj->sca_index = sca_index;
    proj->dettype = dettype;
    proj->unit_scale = unit_scale;

    /* assume we won't cache information */
    proj->img_lines = 0;

    /* Find moon's position at the center of L0R. This will serve as the
       reference.  All other points are adjusted according to it's moon
       position and this location.  Note the reference always uses the nominal
       detector type */
    if (ias_los_model_get_moon_position_at_location(model, ref_band_index,
            ref_sca_index, ref_image_line, ref_detector, IAS_NOMINAL_DETECTOR,
            &proj->ref_rasc, &proj->ref_dec, &proj->ref_dist) != SUCCESS)
    {
        IAS_LOG_ERROR("Error in the calculating moon position for L/S");
        free(proj);
        return NULL;
    }

    IAS_LOG_DEBUG("Ref RASC: %13.8lf  Ref DEC: %13.8lf  Ref Dist: %17.6lf",
            proj->ref_rasc, proj->ref_dec, proj->ref_dist);

    /* Set up the cache if it was requested */
    if (use_cache)
    {
        /* Calculate the number of lines in the input image using the model
           information */
        proj->img_lines = model->sensor.bands[band_index].frame_count
            * model->sensor.bands[band_index].sampling_char.lines_per_frame;

        /* Allocate the buffers for the moon position at each line of the
           image */
        proj->img_rasc = (double *)malloc(sizeof(double) * proj->img_lines);
        if (!proj->img_rasc)
        {
            IAS_LOG_ERROR("Error allocating RA look up table");
            ias_los_model_free_lunar_projection(proj);
            return NULL;
        }

        proj->img_dec = (double *)malloc(sizeof(double) * proj->img_lines);
        if (!proj->img_dec)
        {
            IAS_LOG_ERROR("Error allocating DEC look up table");
            ias_los_model_free_lunar_projection(proj);
            return NULL;
        }

        proj->img_dist = (double *)malloc(sizeof(double) * proj->img_lines);
        if (!proj->img_dist)
        {
            IAS_LOG_ERROR("Error allocating Dist look up table");
            ias_los_model_free_lunar_projection(proj);
            return NULL;
        }

        /* Build look up table for each line in the image */
        for (line = 0; line < proj->img_lines; line++)
        {
            if (ias_los_model_get_moon_position_at_location(model, band_index,
                    sca_index, line, samp, dettype, 
                    &proj->img_rasc[line], &proj->img_dec[line],
                    &proj->img_dist[line]) != SUCCESS)
            {
                IAS_LOG_ERROR("Error calculating moon position look up table "
                    "for line %d", line);
                ias_los_model_free_lunar_projection(proj);
                return NULL;
            }

            /* Make sure the RASC doesn't walk across +/- 180 degree line */
            if (line > 0)
            {
                if ((proj->img_rasc[line] - proj->img_rasc[line - 1]) >  pi/2.0)
                    proj->img_rasc[line] -= 2.0 * pi;
                if ((proj->img_rasc[line] - proj->img_rasc[line - 1]) < -pi/2.0)
                    proj->img_rasc[line] += 2.0 * pi;
            }
        }
    }

    return proj;
}

/*************************************************************************
Name: ias_los_model_transform_lunar_projection

Purpose: Transform the input lat/long to account for the apparent lunar motion.

Returns: SUCCESS or ERROR

**************************************************************************/
int ias_los_model_transform_lunar_projection
(
    const IAS_LUNAR_PROJECTION *proj, /* I: Lunar projection information */
    double iline,          /* I: Input line location */
    double isamp,          /* I: Input sample location */
    double *lunar_lat,     /* I/O: Declination of LOS */
    double *lunar_long,    /* I/O: Right ascension of LOS */
    double *distance_scale /* O: Scale factor for pixel size to apply */
)
{
    int line_index;          /* Index into lunar position look up table */
    double right_ascension;  /* Apparent right ascension of the moon */
    double declination;      /* Declination of the moon            */
    double moon_earth_dist;  /* Earth-moon distance in meters  */
    double out_lat;          /* output latitude calculated */
    double out_long;         /* output longitude calculated */
    double pi = ias_math_get_pi();

    /* Find moon's position for current line/sample time.  Note that the line
       index is rounded to the nearest integer.  That is acceptable since
       this routine should only ever be called with whole number lines. */
    line_index = (int)floor(iline + 0.5);
    if (line_index >= 0 && line_index < proj->img_lines)
    {
        /* The requested line is in the cache, so get the value from there */
        right_ascension = proj->img_rasc[line_index];
        declination = proj->img_dec[line_index];
        moon_earth_dist = proj->img_dist[line_index];
    }
    else
    {
        /* The requested value is outside the cached set, so calculate it */
        if (ias_los_model_get_moon_position_at_location(proj->model,
                proj->band_index, proj->sca_index, iline, isamp, proj->dettype,
                &right_ascension, &declination, &moon_earth_dist) != SUCCESS)
        {
            IAS_LOG_ERROR("Failed to calculate the Moon's position at "
                "line %f, sample %f", iline, isamp);
            return ERROR;
        }
    }

    /* Adjust for the apparent lunar motion */
    *distance_scale = moon_earth_dist / proj->ref_dist;

    /* Check for +/-180 right ascension wrap around */
    if ((right_ascension - proj->ref_rasc) > pi/2.0)
        right_ascension -= 2.0 * pi;
    if ((right_ascension - proj->ref_rasc) < -pi/2.0)
        right_ascension += 2.0 * pi;
    if ((*lunar_long - proj->ref_rasc) > pi/2.0)
        *lunar_long -= 2.0 * pi;
    if ((*lunar_long - proj->ref_rasc) < -pi/2.0)
        *lunar_long += 2.0 * pi;

    out_lat = (*lunar_lat - declination) * (*distance_scale);
    out_lat += proj->ref_dec;
    out_lat /= proj->unit_scale;

    out_long = (*lunar_long - right_ascension) * (*distance_scale);
    out_long += proj->ref_rasc;
    out_long /= proj->unit_scale;

    /* copy the output values to return */
    *lunar_lat = out_lat;
    *lunar_long = out_long;

    return SUCCESS;
}

/*************************************************************************
Name: ias_los_model_get_moon_center

Purpose: Return the declination and right ascension of the moon from the
    lunar structure.

**************************************************************************/
void ias_los_model_get_moon_center
(
    const IAS_LUNAR_PROJECTION *proj, /* I: Lunar projection information */
    double *lunar_dec,      /* O: Declination of moon center */
    double *lunar_rasc      /* O: Right ascension of moon center */
)
{
    *lunar_dec = proj->ref_dec;
    *lunar_rasc = proj->ref_rasc;
}
