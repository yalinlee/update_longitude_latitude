///******************************************************************************
//Name: ias_los_model_calc_scene_corners
//
//Purpose: Determines the bounds of the portion of the output frame that contains
//    actual imagery, excluding "ragged" band/SCA edges.
//
//******************************************************************************/
//#include <math.h>
//#include "ias_grid.h"
//#include "ias_los_model.h"
//#include "logging_channel.h" /* define debug logging channel */
//#include "ias_logging.h"
//
//
///* Define a structure for tracking information about each of the corners */
//struct ACTIVE_CORNER
//{
//   int sca_index;           /* SCA index for this corner */
//   double line;             /* line coordinate*/
//   double sample;           /* sample coordinate*/
//   double lat;              /* latitude coordinate */
//   double lon;              /* longitude coordinate */
//   IAS_VECTOR vec;          /* Geocentric unit vector */
//};
//
///*****************************************************************************
//Name: check_edge
//
//Purpose: Check whether the edge (top or bottom) has points that fall inside
//    the current corners due to the curvature of the field of view.  If it
//    does, adjust the corners as needed.
//
//Returns: SUCCESS or ERROR
//
//Notes:
//    - The corner1, corner2, side1, and side2 follow the edge of interest in
//      a clockwise direction.  So, when looking at the top edge, corner1 is the
//      upper left corner, corner2 is the upper right corner, side1 is the left
//      side, and side2 is the right side.  For the bottom edge, corner1 is the
//      lower right corner, corner2 is the lower left corner, side1 is the right
//      side, and side2 is the left side.
//    - The corner1, corner2, and edge vectors have the initial values that are
//      provided on input and updated values are returned if the edges needed to
//      be adjusted.
//
//*****************************************************************************/
//static int check_edge
//(
//    const IAS_LOS_MODEL *model,     /* I: LOS model structure */
//    int band_index,           /* I: Band index */
//    int nsca,                 /* I: Number of SCAs */
//    int line,                 /* I: line number to use */
//    int ns,                   /* I: Number of samples (detectors) across SCA */
//    IAS_SENSOR_DETECTOR_TYPE dettype, /* I: Detector type to use */
//    double elevation,         /* I: Elevation to use for calculations */
//    double eccen_term,        /* I: 1.0 - Earth's eccentricity squared */
//    const IAS_VECTOR *side1,  /* I: First side of the edge to check (see note)*/
//    const IAS_VECTOR *side2,  /* I: Second side of the edge to check */
//    double left_most_sample,  /* I: Sample that is farthest to the left */
//    double right_most_sample, /* I: Sample that is farthest to the right */
//    IAS_VECTOR *corner1,      /* I/O: First corner of the edge to check */
//    IAS_VECTOR *corner2,      /* I/O: Second corner of the edge to check */
//    IAS_VECTOR *edge          /* I/O: edge vector */
//)
//{
//    int sca_index;            /* Loop counter */
//    double mindist;           /* Minimum distance from great circle */
//    double dist;              /* Distance from great circle */
//    double db;                /* Vector projection distances */
//    double dg;
//    IAS_VECTOR vmin;          /* Minimum distance vector */
//    IAS_VECTOR vg;            /* Temporary working vectors */
//    IAS_VECTOR vtemp;
//    IAS_VECTOR vdiff;
//    struct ACTIVE_CORNER pos[2];/* Points used to construct the active corners*/
//
//    /* Initialize the mindist to avoid a compiler warning */
//    mindist = 1.0;
//
//    /* Construct a vector along the edge being checked */
//    vdiff.x = corner2->x - corner1->x;
//    vdiff.y = corner2->y - corner1->y;
//    vdiff.z = corner2->z - corner1->z;
//
//    /* Construct a "vertical" vector in the plane of the great circle, normal
//       to the edge vector */
//    ias_math_compute_3dvec_cross(&vdiff, edge, &vtemp);
//    if (ias_math_compute_unit_vector(&vtemp, &vg) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Error adjusting edge of active area");
//        return ERROR;
//    }
//
//    /* Check the corners of all SCAs on the current edge */
//    for (sca_index = 0; sca_index < nsca; sca_index++)
//    {
//        int i;         /* Corner index loop counter */
//
//        /* Set the bounds for the SCA currently being checked */
//        pos[0].sca_index = sca_index;
//        pos[1].sca_index = sca_index;
//        pos[0].line = line;
//        pos[1].line = line;
//        pos[0].sample = left_most_sample;
//        pos[1].sample = right_most_sample;
//
//        /* Check both corners of the current SCA */
//        for (i = 0; i < 2; i++)
//        {
//            /* Project the point to the ellipsoid */
//            if (ias_los_model_input_line_samp_to_geodetic(pos[i].line,
//                        pos[i].sample, band_index, pos[i].sca_index, elevation,
//                        model, dettype, NULL, &pos[i].lat, &pos[i].lon)
//                    != SUCCESS)
//            {
//                IAS_LOG_ERROR("Error mapping input line and sample to "
//                        "lat/long");
//                return ERROR;
//            }
//
//            /* Convert latitude from geodetic to geocentric */
//            pos[i].lat = atan(eccen_term * tan(pos[i].lat));
//
//            /* Construct geocentric vector */
//            pos[i].vec.x = cos(pos[i].lon) * cos(pos[i].lat);
//            pos[i].vec.y = sin(pos[i].lon) * cos(pos[i].lat);
//            pos[i].vec.z = sin(pos[i].lat);
//
//            /* Compute distance from great circle */
//            db = ias_math_compute_3dvec_dot(&pos[i].vec, edge);
//            dg = ias_math_compute_3dvec_dot(&pos[i].vec, &vg);
//            if (fabs(dg) > 0.0)
//                dist = db/dg;
//            else
//                dist = 1.0;
//
//            /* Keep track of the minimum distance */
//            if ((sca_index == 0 && i == 0) || dist < mindist)
//            {
//                mindist = dist;
//                vmin.x = pos[i].vec.x;
//                vmin.y = pos[i].vec.y;
//                vmin.z = pos[i].vec.z;
//            }
//        }  /* corner loop */
//    }  /* SCA loop */
//
//    /* See if the minimum distance puts the point inside */
//    if (mindist < 0.0)
//    {
//        IAS_LOG_DEBUG("Adjusting by %lf radians", atan(mindist));
//
//        /* Project the "inside" vector onto the normal vector and the
//           "vertical" vector */
//        db = ias_math_compute_3dvec_dot(&vmin, edge);
//        dg = ias_math_compute_3dvec_dot(&vmin, &vg);
//
//        /* And construct an adjusted "vertical" vector */
//        vtemp.x = db * edge->x + dg * vg.x;
//        vtemp.y = db * edge->y + dg * vg.y;
//        vtemp.z = db * edge->z + dg * vg.z;
//
//        /* Adjust the edge normal vector */
//        ias_math_compute_3dvec_cross(&vtemp, &vdiff, &vg);
//        if (ias_math_compute_unit_vector(&vg, edge) != SUCCESS)
//        {
//            IAS_LOG_ERROR("Error adjusting edge of active area");
//            return ERROR;
//        }
//
//        /* Update the updated corner vectors */
//        ias_math_compute_3dvec_cross(edge, side1, corner1);
//        ias_math_compute_3dvec_cross(side2, edge, corner2);
//
//        IAS_LOG_DEBUG("Edge was adjusted for curvature");
//    }
//
//    return SUCCESS;
//}
//
///******************************************************************************
//Name: ias_los_model_calc_scene_corners
//
//Purpose: Determines the bounds of the portion of the output frame that contains
//    actual imagery, excluding "ragged" band/SCA edges.
//
//Returns: SUCCESS or ERROR
//
//NOTES:
//    - This routine will determine the active image area bounds in
//      latitude/longitude degrees.
//    - For IAS, the start_line parameter is zero and the end_line is the
//      number of lines in the image - 1
//    - For Ingest, the start_line and end_line parameters should be the lines in
//      the interval for the scene corners it is currently calculating.
//
//ALGORITHM REFERENCES:
//See ias_grid.h for a description of the grid structure.
//See ias_los_model.h for a description of the OLI model structure.
//
//******************************************************************************/
//int ias_los_model_calc_scene_corners
//(
//    const IAS_LOS_MODEL *model, /* I: LOS model structure */
//    int band_number,            /* I: Band number to base the calculations on */
//    int start_line,             /* I: Starting line (0-relative) to calculate
//                                      the corners at */
//    int end_line,               /* I: Ending line (0-relative) to calculate the
//                                      corners at (inclusive) */
//    double elevation,           /* I: Elevation to calculate the corners at */
//    struct IAS_CORNERS *corners /* O: Scene lat/long corners (degrees) */
//)
//{
//    int nsca;                 /* Number of scas */
//    int i;
//    int max_lines;
//    int ns;                   /* Number of samples in the scene */
//    int left_odd_sca_index;   /* Index of left-most odd numbered SCA */
//    int left_even_sca_index;  /* Index of left-most even numbered SCA */
//    int right_odd_sca_index;  /* Index of right-most odd numbered SCA */
//    int right_even_sca_index; /* Index of right-most even numbered SCA */
//    int left_most_sca_index;  /* Index of SCA that is farthest to the left */
//    int right_most_sca_index; /* Index of SCA that is farthest to the right */
//    double left_most_sample;  /* Sample that is farthest to the left */
//    double right_most_sample; /* Sample that is farthest to the right */
//    int band_index;           /* Band index for the band number */
//    double eccen_term;        /* Eccentricity term used in several equations.
//                                 It is 1 - e^2  which is equal to b^2 / a^2 */
//    double rad2deg = ias_math_get_degrees_per_radian();
//    struct ACTIVE_CORNER pos[8];/* Points used to construct the active corners*/
//    IAS_VECTOR xt;            /* Side normal vectors */
//    IAS_VECTOR xb;
//    IAS_VECTOR xl;
//    IAS_VECTOR xr;
//    IAS_VECTOR xul;           /* Geocentric active area corner vectors */
//    IAS_VECTOR xur;
//    IAS_VECTOR xll;
//    IAS_VECTOR xlr;
//    IAS_VECTOR temp_vec;
//    IAS_SENSOR_DETECTOR_TYPE dettype = IAS_NOMINAL_DETECTOR;
//    const IAS_SENSOR_BAND_MODEL *band_model;
//
//    /* Convert the band number to an index */
//    band_index = ias_sat_attr_convert_band_number_to_index(band_number);
//    if (band_index == ERROR)
//    {
//        IAS_LOG_ERROR("Failed to convert band number %d to an index",
//                band_number);
//        return ERROR;
//    }
//    band_model = &model->sensor.bands[band_index];
//
//    /* Check the band_index to make sure it is present in the model */
//    if (!band_model->band_present)
//    {
//        IAS_LOG_ERROR("Band number %d is not present in the model",
//                band_number);
//        return ERROR;
//    }
//
//    /* Confirm the start and end lines are valid */
//    max_lines = band_model->frame_count
//        * band_model->sampling_char.lines_per_frame;
//    if (start_line >= end_line)
//    {
//        IAS_LOG_ERROR("Start line %d is after the end line %d", start_line,
//                end_line);
//        return ERROR;
//    }
//    if (start_line < 0)
//    {
//        IAS_LOG_ERROR("Start line %d is before the start of the imagery",
//                start_line);
//        return ERROR;
//    }
//    if (end_line >= max_lines)
//    {
//        IAS_LOG_ERROR("End line %d is after the end of the imagery", end_line);
//        return ERROR;
//    }
//
//    /* Use the number of detectors from the SCA as the number of samples */
//    ns = band_model->scas[0].detectors;
//
//    /* Set the number of SCAs and the numbers of the last even and odd SCAs */
//    nsca = band_model->sca_count;
//    eccen_term = model->earth.semi_minor_axis / model->earth.semi_major_axis;
//    eccen_term *= eccen_term;
//
//    /* Set up the SCA numbers for the corner points.  First see if SCA01 is on
//       the left (positive Y) or right (negative Y) side */
//    if (band_model->scas[0].sca_coef_y[0] >
//        band_model->scas[1].sca_coef_y[0])
//    {
//        left_odd_sca_index = 0;
//        left_even_sca_index = 1;
//        right_odd_sca_index = nsca - 2 + (nsca % 2);
//        right_even_sca_index = nsca - 1 - (nsca % 2);
//        left_most_sca_index = 0;
//        right_most_sca_index = nsca - 1;
//    }
//    else
//    {
//        left_odd_sca_index = nsca - 2 + (nsca % 2);
//        left_even_sca_index = nsca - 1 - (nsca % 2);
//        right_odd_sca_index = 0;
//        right_even_sca_index = 1;
//        left_most_sca_index = nsca - 1;
//        right_most_sca_index = 0;
//    }
//
//    /* See if the odd SCAs lead or lag the even SCAs */
//    if (band_model->scas[0].sca_coef_x[0] > band_model->scas[1].sca_coef_x[0])
//    {
//        /* Odd SCAs lead even SCAs */
//        pos[0].sca_index = left_odd_sca_index;
//        pos[1].sca_index = right_odd_sca_index;
//        pos[4].sca_index = right_even_sca_index;
//        pos[5].sca_index = left_even_sca_index;
//    }
//    else
//    {
//        /* Even SCAs lead odd SCAs */
//        pos[0].sca_index = left_even_sca_index;
//        pos[1].sca_index = right_even_sca_index;
//        pos[4].sca_index = right_odd_sca_index;
//        pos[5].sca_index = left_odd_sca_index;
//    }
//    pos[2].sca_index = right_most_sca_index;
//    pos[3].sca_index = right_most_sca_index;
//    pos[6].sca_index = left_most_sca_index;
//    pos[7].sca_index = left_most_sca_index;
//
//    /* See which direction the sample numbers run */
//    if (band_model->scas[0].sca_coef_y[1] < 0.0)
//    {
//        /* Samples run from left to right */
//        left_most_sample = 0.0;
//        right_most_sample = (double)(ns - 1);
//    }
//    else
//    {
//        /* Samples run from right to left */
//        left_most_sample = (double)(ns - 1);
//        right_most_sample = 0.0;
//    }
//
//    /* Set up the line and sample numbers */
//    pos[0].line = pos[1].line = pos[2].line = pos[7].line = start_line;
//    pos[3].line = pos[4].line = pos[5].line = pos[6].line = end_line;
//    pos[0].sample = pos[5].sample = pos[6].sample = pos[7].sample
//        = left_most_sample;
//    pos[1].sample = pos[2].sample = pos[3].sample = pos[4].sample
//        = right_most_sample;
//
//    for (i = 0; i < 8; i++)
//    {
//        /* Project the point to the ellipsoid */
//        if (ias_los_model_input_line_samp_to_geodetic(pos[i].line,
//                    pos[i].sample, band_index, pos[i].sca_index, elevation,
//                    model, dettype, NULL, &pos[i].lat, &pos[i].lon)
//                != SUCCESS)
//        {
//            IAS_LOG_ERROR("Error mapping input line and sample to lat/long");
//            return ERROR;
//        }
//
//        /* Convert latitude from geodetic to geocentric */
//        pos[i].lat = atan(eccen_term * tan(pos[i].lat));
//
//        /* Construct geocentric vector */
//        pos[i].vec.x = cos(pos[i].lon) * cos(pos[i].lat);
//        pos[i].vec.y = sin(pos[i].lon) * cos(pos[i].lat);
//        pos[i].vec.z = sin(pos[i].lat);
//
//    } /* Loop on corner points */
//
//    /* Construct the great circles that form the sides of the active area */
//    /* Top */
//    ias_math_compute_3dvec_cross(&pos[0].vec, &pos[1].vec, &temp_vec);
//    if (ias_math_compute_unit_vector(&temp_vec, &xt) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Error constructing top of active area");
//        return ERROR;
//    }
//    /* Right side */
//    ias_math_compute_3dvec_cross(&pos[2].vec, &pos[3].vec, &temp_vec);
//    if (ias_math_compute_unit_vector(&temp_vec, &xr) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Error constructing right side of active area");
//        return ERROR;
//    }
//    /* Bottom */
//    ias_math_compute_3dvec_cross(&pos[4].vec, &pos[5].vec, &temp_vec);
//    if (ias_math_compute_unit_vector(&temp_vec, &xb) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Error constructing bottom of active area");
//        return ERROR;
//    }
//    /* Left side */
//    ias_math_compute_3dvec_cross(&pos[6].vec, &pos[7].vec, &temp_vec);
//    if (ias_math_compute_unit_vector(&temp_vec, &xl) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Error constructing left side of active area");
//        return ERROR;
//    }
//
//    /* Construct the active area corner vectors */
//    ias_math_compute_3dvec_cross(&xt, &xl, &xul);
//    ias_math_compute_3dvec_cross(&xr, &xt, &xur);
//    ias_math_compute_3dvec_cross(&xb, &xr, &xlr);
//    ias_math_compute_3dvec_cross(&xl, &xb, &xll);
//
//    /* Check the top edge for points that fall inside due to curvature in the
//       field of view */
//    if (check_edge(model, band_index, nsca, start_line, ns, dettype, elevation,
//                eccen_term, &xl, &xr, left_most_sample, right_most_sample,
//                &xul, &xur, &xt) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Top edge trimming failed");
//        return ERROR;
//    }
//
//    /* Check the bottom edge for points that fall inside due to curvature in
//       the field of view */
//    if (check_edge(model, band_index, nsca, end_line, ns, dettype, elevation,
//                eccen_term, &xr, &xl, left_most_sample, right_most_sample,
//                &xlr, &xll, &xb) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Bottom edge trimming failed");
//        return ERROR;
//    }
//
//    /* Calculate geocentric longitudes for the 4 corners (which are the same
//       as the geodetic longitudes) */
//    corners->upleft.x  = atan2(xul.y, xul.x) * rad2deg;
//    corners->upright.x = atan2(xur.y, xur.x) * rad2deg;
//    corners->loleft.x  = atan2(xll.y, xll.x) * rad2deg;
//    corners->loright.x = atan2(xlr.y, xlr.x) * rad2deg;
//
//    /* Calculate the geocentric latitudes */
//    corners->upleft.y  = atan2(xul.z, sqrt(xul.x * xul.x + xul.y * xul.y));
//    corners->upright.y = atan2(xur.z, sqrt(xur.x * xur.x + xur.y * xur.y));
//    corners->loleft.y  = atan2(xll.z, sqrt(xll.x * xll.x + xll.y * xll.y));
//    corners->loright.y = atan2(xlr.z, sqrt(xlr.x * xlr.x + xlr.y * xlr.y));
//
//    /* Convert the geocentric latitudes to geodetic */
//    corners->upleft.y  = atan(tan(corners->upleft.y) / eccen_term) * rad2deg;
//    corners->upright.y = atan(tan(corners->upright.y) / eccen_term) * rad2deg;
//    corners->loleft.y  = atan(tan(corners->loleft.y) / eccen_term) * rad2deg;
//    corners->loright.y = atan(tan(corners->loright.y) / eccen_term) * rad2deg;
//
//    IAS_LOG_DEBUG("Active Area:\n\tUL: %lf, %lf\n\tUR: %lf, %lf\n"
//            "\tLL: %lf, %lf\n\tLR: %lf, %lf\n", corners->upleft.x,
//            corners->upleft.y, corners->upright.x,
//            corners->upright.y, corners->loleft.x,
//            corners->loleft.y, corners->loright.x,
//            corners->loright.y);
//
//    return SUCCESS;
//}
