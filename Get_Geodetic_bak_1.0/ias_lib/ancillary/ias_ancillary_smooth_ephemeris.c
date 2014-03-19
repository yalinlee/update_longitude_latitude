/*****************************************************************************
NAME: ias_ancillary_smooth_ephemeris

PURPOSE: Perform outlier rejection on the L0R ephemeris records.  Call the
         routine to smooth them before returning to the calling code.

RETURN VALUE: Type = int
    Value    Description
    -----    -----------
    SUCCESS  Successful completion
    ERROR    Operation failed
*****************************************************************************/

#include <stdlib.h>
#include <math.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_l0r.h"
#include "ias_cpf.h"
#include "ias_math.h"
#include "ias_geo.h"
#include "ias_ancillary_private.h"

#define ERROR_MSG_MEMORY "Allocating memory for %s"

int ias_ancillary_smooth_ephemeris
(
    IAS_ACQUISITION_TYPE acquisition_type, /* I: Image acquisition type */
    IAS_CPF *cpf,                      /* I: CPF structure */
    const IAS_L0R_EPHEMERIS *l0r_ephemeris, /* I: L0R ephemeris structure */
    int l0r_ephemeris_count,           /* I: number of records in L0R data */
    const double *interval_start_time, /* I: interval start YEAR, DOY, SOD */
    const double *interval_stop_time,  /* I: interval stop YEAR, DOY, SOD */
    int *valid_ephemeris_count,        /* O: number of ephemeris points in the
                                             smoothed arrays */
    int *invalid_ephemeris_count,      /* O: number of bad ephemeris points */
    double *smoothed_ephemeris_seconds_since_j2000, /* O: array of smoothed
                                             ephemeris seconds since j2000 */
    IAS_VECTOR *smoothed_eph_pos,      /* O: array of smoothed ephemeris
                                             position data */
    IAS_VECTOR *smoothed_eph_vel       /* O: array of smoothed ephemeris
                                             velocity data */
)
{
    /* variables for counters and loop control */
    int max_lagrange_start_index;
    int eph_index;
    int eph_count;
    int lagrange_start_index;

    int number_of_eph_points;    /* number of ephemeris points */
    int valid_eph_count;         /* number of valid ephemeris points */
    int invalid_eph_count;       /* number of bad ephemeris points */
    int first_valid_record;      /* the first valid ephemeris record */
    int last_valid_record;       /* the last valid ephemeris record */

    double current_seconds;      /* current seconds time */
    double interval_start;       /* start time of the interval */
    double interval_stop;        /* stop time of the interval */
    double epoch_time[3];        /* ephemeris time year, DOY, SOD */
    double ecef2eci_time[3];     /* ephemeris time year, DOY, SOD, for the
                                    ecef2eci conversion */
    double mag_of_ang_momentum;  /* magnitude of angular momentum */
    double orbit_radius;         /* orbital radius of satellite */
    double lagrange_stop_time;   /* Lagrange interpolation time */
    double pole_wander_x;        /* X shift pole wander */
    double pole_wander_y;        /* Y shift pole wander */
    double ut1_utc_correction;   /* ut1-utc coord time difference */
    double modified_julian_date; /* modified Julian date */
    double nom_orbit_radius_in_meters; /* nominal orbit radius in meters */
    double delta_time;           /* Ephemeris propagation time step */
    double acceleration_x;       /* Propagation gravitational acceleration */
    double acceleration_y;       /* terms for x, y, and z directions */
    double acceleration_z;


    IAS_VECTOR *eci_pos = NULL;  /* ephemeris position data in ECI */
    IAS_VECTOR *eci_vel = NULL;  /* ephemeris velocity data in ECI */
    const IAS_VECTOR *ecef_pos = NULL; /* ephemeris position data in ECEF */
    const IAS_VECTOR *ecef_vel = NULL; /* ephemeris velocity data in ECEF */
    IAS_VECTOR *raw_pos = NULL;  /* interpolated ephemeris position data */
    IAS_VECTOR *raw_vel = NULL;  /* interpolated ephemeris velocity data */
    IAS_VECTOR momentum_vector;  /* angular momentum vector for qc check */
 
    const struct IAS_CPF_ORBIT_PARAMETERS *orbit_parameters = NULL;
        /* CPF parameters for the orbit parameters */
    const struct IAS_CPF_ANCILLARY_QA_THRESHOLDS *anc_qa_thresholds = NULL;
        /* CPF parameters for the ancillary QA thresholds */
    const struct IAS_CPF_EARTH_CONSTANTS *earth_constants = NULL;
        /* CPF parameters for Earth constants */

#if DEBUG_GENERATE_DATA_FILES == 1
    FILE *ofp = NULL;
#endif

    /* get the orbit parameters from the cpf */
    orbit_parameters = ias_cpf_get_orbit(cpf);
    if (orbit_parameters == NULL)
    {
        IAS_LOG_ERROR("Reading orbit parameters from the CPF");
        return ERROR;
    }

    /* convert the radius to meters so that the multiplications are reduced */
    nom_orbit_radius_in_meters =
        orbit_parameters->nominal_orbit_radius * 1000.0;

    /* get the QA thresholds from the cpf */
    anc_qa_thresholds = ias_cpf_get_ancil_qa_thresholds(cpf);
    if (anc_qa_thresholds == NULL)
    {
        IAS_LOG_ERROR("Reading ancillary qa thresholds from the CPF");
        return ERROR;
    }

    /* get the Earth constants from the cpf */
    earth_constants = ias_cpf_get_earth_const(cpf);
    if (earth_constants == NULL)
    {
        IAS_LOG_ERROR("Reading Earth constants from the CPF");
        return ERROR;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    ofp = fopen("l0rp.ecef.dat", "w");

    for (eph_index = 0; eph_index < l0r_ephemeris_count; eph_index++)
    {
        double temp_seconds = IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
            l0r_ephemeris[eph_index].l0r_time);
        if (ias_math_convert_j2000_seconds_to_year_doy_sod(
                temp_seconds, epoch_time) != SUCCESS)
        {
            IAS_LOG_ERROR(
                "Converting J2000 seconds %lf to Year, DOY, SOD format",
                temp_seconds);
            return ERROR;
        }

        fprintf(ofp,
                "%d %f %f %f %f %e %e %e %e %e %e\n",
                eph_index, temp_seconds,
                epoch_time[0], epoch_time[1], epoch_time[2],
                l0r_ephemeris[eph_index].ecef_position_meters.x,
                l0r_ephemeris[eph_index].ecef_position_meters.y,
                l0r_ephemeris[eph_index].ecef_position_meters.z,
                l0r_ephemeris[eph_index].ecef_velocity_meters_per_sec.x,
                l0r_ephemeris[eph_index].ecef_velocity_meters_per_sec.y,
                l0r_ephemeris[eph_index].ecef_velocity_meters_per_sec.z);
    }

    fclose(ofp);
#endif

    /* Find seconds from J2000 for the interval start time */
    if (ias_math_convert_year_doy_sod_to_j2000_seconds(
            interval_start_time, &interval_start) != SUCCESS)
    {
        IAS_LOG_ERROR("Converting Year, DOY, SOD format to J2000 seconds");
        return ERROR;
    }

    /* Find seconds from J2000 for the interval stop time */
    if (ias_math_convert_year_doy_sod_to_j2000_seconds(
            interval_stop_time, &interval_stop) != SUCCESS)
    {
        IAS_LOG_ERROR("Converting Year, DOY, SOD format to J2000 seconds");
        return ERROR;
    }

    /* Test for enough valid data */
    IAS_LOG_INFO("L0R ephemeris record count = %d", l0r_ephemeris_count);

    /* Find the first valid record */
    first_valid_record = 0;
    while (l0r_ephemeris[first_valid_record].warning_flag &&
           first_valid_record < l0r_ephemeris_count)
        first_valid_record++;

    IAS_LOG_DEBUG("First valid record = %d", first_valid_record);

    /* Find the last valid record */
    last_valid_record = l0r_ephemeris_count - 1;
    while (l0r_ephemeris[last_valid_record].warning_flag &&
           last_valid_record > first_valid_record)
        last_valid_record--;

    IAS_LOG_DEBUG("Last valid record = %d", last_valid_record);


    /* If the number of points is below the Lagrange interpolation minimum,
       exit */
    if (l0r_ephemeris_count < IAS_LAGRANGE_PTS)
    {
        IAS_LOG_WARNING("Only %d ephemeris points are available",
            l0r_ephemeris_count);
        
        /* Don't exit if it's a solar collection; those are likely to be 
           short */
        if ((acquisition_type == IAS_EARTH) || (acquisition_type == IAS_LUNAR)
            || (acquisition_type == IAS_STELLAR))
        {
            IAS_LOG_ERROR("%d points are required", IAS_LAGRANGE_PTS);
            return ERROR;
        }
    }

    current_seconds = IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
        l0r_ephemeris[first_valid_record].l0r_time);

    IAS_LOG_DEBUG("Lower bounds on ephemeris time %lf", current_seconds);

    if (interval_start < current_seconds)
    {
        IAS_LOG_WARNING("Scene time %lf starts before start of ephemeris "
            "time %lf", interval_start, current_seconds);

        /* Don't exit if it's a solar collection; those are likely to be
           short */
        if ((acquisition_type == IAS_EARTH) || (acquisition_type == IAS_LUNAR)
            || (acquisition_type == IAS_STELLAR))
        {
            IAS_LOG_ERROR("Ephemeris is required to cover the entire collect");
            return ERROR;
        }
    }

    if (ias_math_convert_j2000_seconds_to_year_doy_sod(
            current_seconds, epoch_time) != SUCCESS)
    {
        IAS_LOG_ERROR("Converting J2000 seconds %lf to Year, DOY, SOD format",
                      current_seconds);
        return ERROR;
    }

    current_seconds = IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
        l0r_ephemeris[last_valid_record].l0r_time);

    IAS_LOG_DEBUG("Upper bounds on ephemeris time %lf", current_seconds);

    if (interval_stop > current_seconds)
    {
        IAS_LOG_WARNING("Scene time %lf ends after end of ephemeris time %lf",
                      interval_stop, current_seconds);

        /* Don't exit if it's a solar collection; those are likely to be
           short */
        if ((acquisition_type == IAS_EARTH) || (acquisition_type == IAS_LUNAR)
            || (acquisition_type == IAS_STELLAR))
        {
            IAS_LOG_ERROR("Ephemeris is required to cover the entire collect");
            return ERROR;
        }
    }

    IAS_LOG_DEBUG("L0R ephemeris count %d", l0r_ephemeris_count);
    IAS_LOG_DEBUG("Ephemeris new epoch %f, %f, %f",
                  epoch_time[0], epoch_time[1], epoch_time[2]);

    /* Allocate memory for "fixing" and adjusting ephemeris */
    number_of_eph_points = l0r_ephemeris_count;
    if (number_of_eph_points < IAS_LAGRANGE_PTS)
    {
        number_of_eph_points = IAS_LAGRANGE_PTS;
    }

    eci_pos = (IAS_VECTOR *)malloc(sizeof(IAS_VECTOR) * number_of_eph_points);
    if (eci_pos == NULL)
    {
        IAS_LOG_ERROR(ERROR_MSG_MEMORY, "eci_pos");
        return ERROR;
    }

    eci_vel = (IAS_VECTOR *)malloc(sizeof(IAS_VECTOR) * number_of_eph_points);
    if (eci_vel == NULL)
    {
        IAS_LOG_ERROR(ERROR_MSG_MEMORY, "eci_vel");
        free(eci_pos);
        return ERROR;
    }

    raw_pos = (IAS_VECTOR *)malloc(sizeof(IAS_VECTOR) * number_of_eph_points);
    if (raw_pos == NULL)
    {
        IAS_LOG_ERROR(ERROR_MSG_MEMORY, "raw_pos");
        free(eci_pos);
        free(eci_vel);
        return ERROR;
    }

    raw_vel = (IAS_VECTOR *)malloc(sizeof(IAS_VECTOR) * number_of_eph_points);
    if (raw_vel == NULL)
    {
        IAS_LOG_ERROR(ERROR_MSG_MEMORY, "raw_vel");
        free(eci_pos);
        free(eci_vel);
        free(raw_pos);
        return ERROR;
    }

    /* get x and y shift pole wander and UT1-UTC time difference */
    if (ias_geo_compute_getmjdcoords(epoch_time, cpf, &modified_julian_date,
            &pole_wander_x, &pole_wander_y, &ut1_utc_correction) == ERROR)
    {
        IAS_LOG_ERROR("Establishing Earth Model");
        free(eci_pos);
        free(eci_vel);
        free(raw_pos);
        free(raw_vel);
        return ERROR;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    ofp = fopen("ecef.vectors.dat", "w");
#endif

    /* Log start of ephemeris check in log */
    IAS_LOG_DEBUG("Identifying ephemeris outliers");
    valid_eph_count = 0;
    invalid_eph_count = 0;
    for (eph_index = first_valid_record;
         eph_index <= last_valid_record;
         eph_index++)
    {
        /* Perform angular momentum check */
        ecef_pos = &l0r_ephemeris[eph_index].ecef_position_meters;
        ecef_vel = &l0r_ephemeris[eph_index].ecef_velocity_meters_per_sec;
        smoothed_ephemeris_seconds_since_j2000[valid_eph_count]
            = IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
                l0r_ephemeris[eph_index].l0r_time);

        /* Get the Year, DOY, and SOD for the eci value to generate. */
        if (ias_math_convert_j2000_seconds_to_year_doy_sod(
                smoothed_ephemeris_seconds_since_j2000[valid_eph_count],
                ecef2eci_time) != SUCCESS)
        {
            IAS_LOG_ERROR("Converting J2000 seconds %lf to Year, DOY, SOD"
                " format",
                smoothed_ephemeris_seconds_since_j2000[valid_eph_count]);
            free(eci_pos);
            free(eci_vel);
            free(raw_pos);
            free(raw_vel);
            return ERROR;
        }

        /* Convert to inertial for angular momentum checks */
        if (ias_geo_transform_ecef2j2k(pole_wander_x, pole_wander_y,
                ut1_utc_correction, ecef_pos, ecef_vel, ecef2eci_time,
                &eci_pos[valid_eph_count], &eci_vel[valid_eph_count])
                != SUCCESS)
        {
            IAS_LOG_ERROR("Converting ECEF coordinate to ECI J2000");
            free(eci_pos);
            free(eci_vel);
            free(raw_pos);
            free(raw_vel);
            return ERROR;
        }

        ias_math_compute_3dvec_cross(&eci_pos[valid_eph_count],
            &eci_vel[valid_eph_count], &momentum_vector);
        mag_of_ang_momentum = ias_math_compute_vector_length(&momentum_vector);

        /* Perform orbital radius check */
        orbit_radius = ias_math_compute_vector_length(
            &eci_pos[valid_eph_count]);

        if (fabs(mag_of_ang_momentum
                 - orbit_parameters->nominal_angular_momentum)
                <= anc_qa_thresholds->angular_momentum_tolerance
            && /* both conditions must be met */
            fabs(orbit_radius - nom_orbit_radius_in_meters)
                <= anc_qa_thresholds->orbit_radius_tolerance)
        {
            /* If angular momentum and orbital radius check out, increment the 
               count of valid ephemeris points */
                
#if DEBUG_GENERATE_DATA_FILES == 1
            fprintf(ofp,
                "%d %f %f %f %e %e %e %e %e %e\n",
                eph_index,
                ecef2eci_time[0], ecef2eci_time[1], ecef2eci_time[2],
                ecef_pos->x, ecef_pos->y, ecef_pos->z,
                ecef_vel->x, ecef_vel->y, ecef_vel->z);
#endif
            /* Increment to the next save position in the arrays. */
            valid_eph_count++;
        }
        else
        {
            IAS_LOG_DEBUG("Eliminated Ephemeris outlier index:%d x pos:%f "
                         "y pos:%f z pos:%f x vel:%f y vel:%f z vel:%f",
                         eph_index, 
                         ecef_pos->x, ecef_pos->y, ecef_pos->z,
                         ecef_vel->x, ecef_vel->y, ecef_vel->z);
            invalid_eph_count++;
        }
    }

    /* Exit if no valid ephemeris points were found */
    if (valid_eph_count < 1)
    {
        IAS_LOG_ERROR("No valid ephemeris points were found.");
        free(eci_pos);
        free(eci_vel);
        free(raw_pos);
        free(raw_vel);
        return ERROR;
    }

    if (valid_eph_count >= IAS_LAGRANGE_PTS)
    {
        number_of_eph_points = valid_eph_count;
    }
    else
    {
        number_of_eph_points = IAS_LAGRANGE_PTS;
    }

    /* Propagate additional points if there are not enough for Lagrange 
       interpolation */
    for (eph_index = valid_eph_count; eph_index < number_of_eph_points;
        eph_index++)
    {
        delta_time = 1.0;

        IAS_LOG_INFO("Propagating ephemeris data into index %d to have enough "
            "to do Lagrange interpolation", eph_index);

        /* Propagate time */
        smoothed_ephemeris_seconds_since_j2000[eph_index] =
            smoothed_ephemeris_seconds_since_j2000[eph_index - 1] + delta_time;

        /* Propagate position */
        eci_pos[eph_index].x = eci_pos[eph_index - 1].x
            + eci_vel[eph_index - 1].x * delta_time;
        eci_pos[eph_index].y = eci_pos[eph_index - 1].y
            + eci_vel[eph_index - 1].y * delta_time;
        eci_pos[eph_index].z = eci_pos[eph_index - 1].z
            + eci_vel[eph_index - 1].z * delta_time;

        /* Propagate velocity */
        acceleration_x = ias_geo_compute_earth_second_partial_x(
            earth_constants->semi_major_axis,
            earth_constants->gravity_constant,
            eci_pos[eph_index - 1].x, eci_pos[eph_index - 1].y,
            eci_pos[eph_index - 1].z); 
        acceleration_y = ias_geo_compute_earth_second_partial_y(
            earth_constants->semi_major_axis,
            earth_constants->gravity_constant,
            eci_pos[eph_index - 1].x, eci_pos[eph_index - 1].y,
            eci_pos[eph_index - 1].z); 
        acceleration_z = ias_geo_compute_earth_second_partial_z(
            earth_constants->semi_major_axis,
            earth_constants->gravity_constant,
            eci_pos[eph_index - 1].x, eci_pos[eph_index - 1].y,
            eci_pos[eph_index - 1].z); 
        eci_vel[eph_index].x = eci_vel[eph_index - 1].x + acceleration_x
            * delta_time;
        eci_vel[eph_index].y = eci_vel[eph_index - 1].y + acceleration_y
            * delta_time;
        eci_vel[eph_index].z = eci_vel[eph_index - 1].z + acceleration_z
            * delta_time;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    fclose(ofp);

    ofp = fopen("eci.vectors.dat", "w");

    for (eph_index = 0; eph_index < valid_eph_count; eph_index++)
    {
        fprintf(ofp, "%f %e %e %e %e %e %e\n",
            epoch_time[2] - smoothed_ephemeris_seconds_since_j2000[0]
                          + smoothed_ephemeris_seconds_since_j2000[eph_index],
            eci_pos[eph_index].x, eci_pos[eph_index].y, eci_pos[eph_index].z,
            eci_vel[eph_index].x, eci_vel[eph_index].y, eci_vel[eph_index].z);
    }

    fclose(ofp);
#endif

    /* Smooth GPS data to remove small random errors present in the data. */
    IAS_LOG_DEBUG("Number of valid ephemeris found %d", valid_eph_count);

    /* Fix any inconsistencies in ephemeris time stamps */
    if (ias_ancillary_correct_ephemeris_time(eci_pos, eci_vel, 
            number_of_eph_points, smoothed_ephemeris_seconds_since_j2000) 
            != SUCCESS)
    {
        IAS_LOG_ERROR("Smoothing ephemeris time");
        free(eci_pos);
        free(eci_vel);
        free(raw_pos);
        free(raw_vel);
        return ERROR;
    }

    /* Loop over all the smoothed time stamps to interpolate the ephemeris info
       at that time, stopping when the full time span is covered or until all
       the array entries are filled.  At a minimum, the ephemeris count needs
       to reach IAS_LAGRANGE_PTS. */
    lagrange_stop_time = smoothed_ephemeris_seconds_since_j2000[0];
    eph_count = 0;
    max_lagrange_start_index = number_of_eph_points - IAS_LAGRANGE_PTS;
    lagrange_start_index = 0;
    while ((lagrange_stop_time <=
           smoothed_ephemeris_seconds_since_j2000[number_of_eph_points - 1]
            || eph_count < IAS_LAGRANGE_PTS)
           && eph_count < number_of_eph_points)
    {
        /* Find Lagrange interpolation start index */
        while ((lagrange_start_index < number_of_eph_points) &&
               (smoothed_ephemeris_seconds_since_j2000[lagrange_start_index]
                <= lagrange_stop_time))
        {
            lagrange_start_index++;
        }

        lagrange_start_index -= IAS_LAGRANGE_PTS / 2;

        if (lagrange_start_index < 0)
            lagrange_start_index = 0;

        if (lagrange_start_index > max_lagrange_start_index)
            lagrange_start_index = max_lagrange_start_index;

        /* Interpolate position and velocity vector using Lagrange
           interpolation. */
        ias_geo_lagrange_interpolate(
                 &smoothed_ephemeris_seconds_since_j2000[lagrange_start_index],
                 &eci_pos[lagrange_start_index],
                 &eci_vel[lagrange_start_index],
                 IAS_LAGRANGE_PTS,
                 lagrange_stop_time,
                 &raw_pos[eph_count],
                 &raw_vel[eph_count]);

        lagrange_stop_time += IAS_EPHEM_SAMPLING_PERIOD;

        eph_count++;
    }
    valid_eph_count = eph_count;

    /* free memory no longer used */
    free(eci_pos);
    free(eci_vel);
    eci_pos = NULL;
    eci_vel = NULL;

    /* Time is now at evenly spaced increments */
    for (eph_index = 1; eph_index < valid_eph_count; eph_index++)
    {
        smoothed_ephemeris_seconds_since_j2000[eph_index]
            = smoothed_ephemeris_seconds_since_j2000[eph_index - 1]
            + IAS_EPHEM_SAMPLING_PERIOD;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    ofp = fopen("raw.eci.dat", "w");

    for (eph_index = 0; eph_index < valid_eph_count; eph_index++)
    {
        fprintf(ofp, "%f %e %e %e %e %e %e\n",
            epoch_time[2] - smoothed_ephemeris_seconds_since_j2000[0]
                          + smoothed_ephemeris_seconds_since_j2000[eph_index],
            raw_pos[eph_index].x, raw_pos[eph_index].y, raw_pos[eph_index].z,
            raw_vel[eph_index].x, raw_vel[eph_index].y, raw_vel[eph_index].z);
    }

    fclose(ofp);
#endif

    /* Smooth GPS data to remove small random errors present in the data. */
    if (ias_ancillary_kalman_smooth_ephemeris(cpf,
            valid_eph_count, smoothed_ephemeris_seconds_since_j2000,
            raw_pos, raw_vel, IAS_EPHEM_SAMPLING_PERIOD,
            smoothed_eph_pos, smoothed_eph_vel) != SUCCESS)
    {
        IAS_LOG_ERROR("Smoothing ephemeris");
        free(raw_pos);
        free(raw_vel);
        return ERROR;
    }

    /* assign output values */
    *valid_ephemeris_count = valid_eph_count;
    *invalid_ephemeris_count = invalid_eph_count;

    /* free memory no longer used */
    free(raw_pos);
    free(raw_vel);

    return SUCCESS;
}


int ias_ancillary_smooth_ephemeris_for_MWD
(
    IAS_ACQUISITION_TYPE acquisition_type, /* I: Image acquisition type */
    IAS_CPF *cpf,                      /* I: CPF structure */
    const IAS_L0R_EPHEMERIS *l0r_ephemeris, /* I: L0R ephemeris structure */
    int l0r_ephemeris_count,           /* I: number of records in L0R data */
    int *valid_ephemeris_count,        /* O: number of ephemeris points in the
                                             smoothed arrays */
    int *invalid_ephemeris_count,      /* O: number of bad ephemeris points */
    double *smoothed_ephemeris_seconds_since_j2000, /* O: array of smoothed
                                             ephemeris seconds since j2000 */
    IAS_VECTOR *smoothed_eph_pos,      /* O: array of smoothed ephemeris
                                             position data */
    IAS_VECTOR *smoothed_eph_vel,       /* O: array of smoothed ephemeris
                                             velocity data */
    double *ephemeris_start_time,		/* O: first valid ephemeris data time */
    double *ephemeris_end_time			/* O: last valid ephemeris data time */
)
{
    /* variables for counters and loop control */
    int max_lagrange_start_index;
    int eph_index;
    int eph_count;
    int lagrange_start_index;

    int number_of_eph_points;    /* number of ephemeris points */
    int valid_eph_count;         /* number of valid ephemeris points */
    int invalid_eph_count;       /* number of bad ephemeris points */
    int first_valid_record;      /* the first valid ephemeris record */
    int last_valid_record;       /* the last valid ephemeris record */

    double current_seconds;      /* current seconds time */
    double epoch_time[3];        /* ephemeris time year, DOY, SOD */
    double ecef2eci_time[3];     /* ephemeris time year, DOY, SOD, for the
                                    ecef2eci conversion */
    double mag_of_ang_momentum;  /* magnitude of angular momentum */
    double orbit_radius;         /* orbital radius of satellite */
    double lagrange_stop_time;   /* Lagrange interpolation time */
    double pole_wander_x;        /* X shift pole wander */
    double pole_wander_y;        /* Y shift pole wander */
    double ut1_utc_correction;   /* ut1-utc coord time difference */
    double modified_julian_date; /* modified Julian date */
    double nom_orbit_radius_in_meters; /* nominal orbit radius in meters */
    double delta_time;           /* Ephemeris propagation time step */
    double acceleration_x;       /* Propagation gravitational acceleration */
    double acceleration_y;       /* terms for x, y, and z directions */
    double acceleration_z;


    IAS_VECTOR *eci_pos = NULL;  /* ephemeris position data in ECI */
    IAS_VECTOR *eci_vel = NULL;  /* ephemeris velocity data in ECI */
    const IAS_VECTOR *ecef_pos = NULL; /* ephemeris position data in ECEF */
    const IAS_VECTOR *ecef_vel = NULL; /* ephemeris velocity data in ECEF */
    IAS_VECTOR *raw_pos = NULL;  /* interpolated ephemeris position data */
    IAS_VECTOR *raw_vel = NULL;  /* interpolated ephemeris velocity data */
    IAS_VECTOR momentum_vector;  /* angular momentum vector for qc check */

    const struct IAS_CPF_ORBIT_PARAMETERS *orbit_parameters = NULL;
        /* CPF parameters for the orbit parameters */
    const struct IAS_CPF_ANCILLARY_QA_THRESHOLDS *anc_qa_thresholds = NULL;
        /* CPF parameters for the ancillary QA thresholds */
    const struct IAS_CPF_EARTH_CONSTANTS *earth_constants = NULL;
        /* CPF parameters for Earth constants */

#if DEBUG_GENERATE_DATA_FILES == 1
    FILE *ofp = NULL;
#endif

    /* get the orbit parameters from the cpf */
    orbit_parameters = ias_cpf_get_orbit(cpf);
    if (orbit_parameters == NULL)
    {
        IAS_LOG_ERROR("Reading orbit parameters from the CPF");
        return ERROR;
    }

    /* convert the radius to meters so that the multiplications are reduced */
    nom_orbit_radius_in_meters =
        orbit_parameters->nominal_orbit_radius * 1000.0;

    /* get the QA thresholds from the cpf */
    anc_qa_thresholds = ias_cpf_get_ancil_qa_thresholds(cpf);
    if (anc_qa_thresholds == NULL)
    {
        IAS_LOG_ERROR("Reading ancillary qa thresholds from the CPF");
        return ERROR;
    }

    /* get the Earth constants from the cpf */
    earth_constants = ias_cpf_get_earth_const(cpf);
    if (earth_constants == NULL)
    {
        IAS_LOG_ERROR("Reading Earth constants from the CPF");
        return ERROR;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    ofp = fopen("l0rp.ecef.dat", "w");

    for (eph_index = 0; eph_index < l0r_ephemeris_count; eph_index++)
    {
        double temp_seconds = IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
            l0r_ephemeris[eph_index].l0r_time);
        if (ias_math_convert_j2000_seconds_to_year_doy_sod(
                temp_seconds, epoch_time) != SUCCESS)
        {
            IAS_LOG_ERROR(
                "Converting J2000 seconds %lf to Year, DOY, SOD format",
                temp_seconds);
            return ERROR;
        }

        fprintf(ofp,
                "%d %f %f %f %f %e %e %e %e %e %e\n",
                eph_index, temp_seconds,
                epoch_time[0], epoch_time[1], epoch_time[2],
                l0r_ephemeris[eph_index].ecef_position_meters.x,
                l0r_ephemeris[eph_index].ecef_position_meters.y,
                l0r_ephemeris[eph_index].ecef_position_meters.z,
                l0r_ephemeris[eph_index].ecef_velocity_meters_per_sec.x,
                l0r_ephemeris[eph_index].ecef_velocity_meters_per_sec.y,
                l0r_ephemeris[eph_index].ecef_velocity_meters_per_sec.z);
    }

    fclose(ofp);
#endif

//    /* Find seconds from J2000 for the interval start time */
//    if (ias_math_convert_year_doy_sod_to_j2000_seconds(
//            interval_start_time, &interval_start) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Converting Year, DOY, SOD format to J2000 seconds");
//        return ERROR;
//    }
//
//    /* Find seconds from J2000 for the interval stop time */
//    if (ias_math_convert_year_doy_sod_to_j2000_seconds(
//            interval_stop_time, &interval_stop) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Converting Year, DOY, SOD format to J2000 seconds");
//        return ERROR;
//    }

    /* Test for enough valid data */
    IAS_LOG_INFO("L0R ephemeris record count = %d", l0r_ephemeris_count);

    /* Find the first valid record */
    first_valid_record = 0;
    int i = l0r_ephemeris[first_valid_record].warning_flag;
    while (l0r_ephemeris[first_valid_record].warning_flag &&
           first_valid_record < l0r_ephemeris_count)
        first_valid_record++;

    IAS_LOG_DEBUG("First valid record = %d", first_valid_record);

    /* Find the last valid record */
    last_valid_record = l0r_ephemeris_count - 1;
    while (l0r_ephemeris[last_valid_record].warning_flag &&
           last_valid_record > first_valid_record)
        last_valid_record--;

    IAS_LOG_DEBUG("Last valid record = %d", last_valid_record);


    /* If the number of points is below the Lagrange interpolation minimum,
       exit */
    if (l0r_ephemeris_count < IAS_LAGRANGE_PTS)
    {
        IAS_LOG_WARNING("Only %d ephemeris points are available",
            l0r_ephemeris_count);

        /* Don't exit if it's a solar collection; those are likely to be
           short */
        if ((acquisition_type == IAS_EARTH) || (acquisition_type == IAS_LUNAR)
            || (acquisition_type == IAS_STELLAR))
        {
            IAS_LOG_ERROR("%d points are required", IAS_LAGRANGE_PTS);
            return ERROR;
        }
    }

    current_seconds = IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
        l0r_ephemeris[first_valid_record].l0r_time);

   /* ***********************************************************************
    * Added by LYL at 2014/3/13
    * PURPOSE: to get the first valid ephemeris data's time
    * ***********************************************************************/
    *ephemeris_start_time = current_seconds;
    IAS_LOG_DEBUG("Lower bounds on ephemeris time %lf", current_seconds);

//    if (interval_start < current_seconds)
//    {
//        IAS_LOG_WARNING("Scene time %lf starts before start of ephemeris "
//            "time %lf", interval_start, current_seconds);
//
//        /* Don't exit if it's a solar collection; those are likely to be
//           short */
//        if ((acquisition_type == IAS_EARTH) || (acquisition_type == IAS_LUNAR)
//            || (acquisition_type == IAS_STELLAR))
//        {
//            IAS_LOG_ERROR("Ephemeris is required to cover the entire collect");
//            return ERROR;
//        }
//    }
    ///////////////////////////////////////////////////////////////
    //added by LYL at 2014/3/6
    if (ias_math_init_leap_seconds(current_seconds, &earth_constants->leap_seconds_data))
	{
		IAS_LOG_ERROR("Init leap seconds");
		ias_cpf_free(cpf);
		exit(EXIT_FAILURE);
	}



    if (ias_math_convert_j2000_seconds_to_year_doy_sod(
            current_seconds, epoch_time) != SUCCESS)
    {
        IAS_LOG_ERROR("Converting J2000 seconds %lf to Year, DOY, SOD format",
                      current_seconds);
        return ERROR;
    }

    current_seconds = IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
        l0r_ephemeris[last_valid_record].l0r_time);

    /* ***********************************************************************
     * Added by LYL at 2014/3/13
     * PURPOSE: to get the first valid ephemeris data's time
     * ***********************************************************************/
    *ephemeris_end_time = current_seconds;
    IAS_LOG_DEBUG("Upper bounds on ephemeris time %lf", current_seconds);

//    if (interval_stop > current_seconds)
//    {
//        IAS_LOG_WARNING("Scene time %lf ends after end of ephemeris time %lf",
//                      interval_stop, current_seconds);
//
//        /* Don't exit if it's a solar collection; those are likely to be
//           short */
//        if ((acquisition_type == IAS_EARTH) || (acquisition_type == IAS_LUNAR)
//            || (acquisition_type == IAS_STELLAR))
//        {
//            IAS_LOG_ERROR("Ephemeris is required to cover the entire collect");
//            return ERROR;
//        }
//    }

    IAS_LOG_DEBUG("L0R ephemeris count %d", l0r_ephemeris_count);
    IAS_LOG_DEBUG("Ephemeris new epoch %f, %f, %f",
                  epoch_time[0], epoch_time[1], epoch_time[2]);

    /* Allocate memory for "fixing" and adjusting ephemeris */
    number_of_eph_points = l0r_ephemeris_count;
    if (number_of_eph_points < IAS_LAGRANGE_PTS)
    {
        number_of_eph_points = IAS_LAGRANGE_PTS;
    }

    eci_pos = (IAS_VECTOR *)malloc(sizeof(IAS_VECTOR) * number_of_eph_points);
    if (eci_pos == NULL)
    {
        IAS_LOG_ERROR(ERROR_MSG_MEMORY, "eci_pos");
        return ERROR;
    }

    eci_vel = (IAS_VECTOR *)malloc(sizeof(IAS_VECTOR) * number_of_eph_points);
    if (eci_vel == NULL)
    {
        IAS_LOG_ERROR(ERROR_MSG_MEMORY, "eci_vel");
        free(eci_pos);
        return ERROR;
    }

    raw_pos = (IAS_VECTOR *)malloc(sizeof(IAS_VECTOR) * number_of_eph_points);
    if (raw_pos == NULL)
    {
        IAS_LOG_ERROR(ERROR_MSG_MEMORY, "raw_pos");
        free(eci_pos);
        free(eci_vel);
        return ERROR;
    }

    raw_vel = (IAS_VECTOR *)malloc(sizeof(IAS_VECTOR) * number_of_eph_points);
    if (raw_vel == NULL)
    {
        IAS_LOG_ERROR(ERROR_MSG_MEMORY, "raw_vel");
        free(eci_pos);
        free(eci_vel);
        free(raw_pos);
        return ERROR;
    }

    /* get x and y shift pole wander and UT1-UTC time difference */
    if (ias_geo_compute_getmjdcoords(epoch_time, cpf, &modified_julian_date,
            &pole_wander_x, &pole_wander_y, &ut1_utc_correction) == ERROR)
    {
        IAS_LOG_ERROR("Establishing Earth Model");
        free(eci_pos);
        free(eci_vel);
        free(raw_pos);
        free(raw_vel);
        return ERROR;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    ofp = fopen("ecef.vectors.dat", "w");
#endif

    /* Log start of ephemeris check in log */
    IAS_LOG_DEBUG("Identifying ephemeris outliers");
    valid_eph_count = 0;
    invalid_eph_count = 0;
    for (eph_index = first_valid_record;
         eph_index <= last_valid_record;
         eph_index++)
    {
        /* Perform angular momentum check */
        ecef_pos = &l0r_ephemeris[eph_index].ecef_position_meters;
        ecef_vel = &l0r_ephemeris[eph_index].ecef_velocity_meters_per_sec;
        smoothed_ephemeris_seconds_since_j2000[valid_eph_count]
            = IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
                l0r_ephemeris[eph_index].l0r_time);

        /* Get the Year, DOY, and SOD for the eci value to generate. */
        if (ias_math_convert_j2000_seconds_to_year_doy_sod(
                smoothed_ephemeris_seconds_since_j2000[valid_eph_count],
                ecef2eci_time) != SUCCESS)
        {
            IAS_LOG_ERROR("Converting J2000 seconds %lf to Year, DOY, SOD"
                " format",
                smoothed_ephemeris_seconds_since_j2000[valid_eph_count]);
            free(eci_pos);
            free(eci_vel);
            free(raw_pos);
            free(raw_vel);
            return ERROR;
        }

        /* Convert to inertial for angular momentum checks */
        if (ias_geo_transform_ecef2j2k(pole_wander_x, pole_wander_y,
                ut1_utc_correction, ecef_pos, ecef_vel, ecef2eci_time,
                &eci_pos[valid_eph_count], &eci_vel[valid_eph_count])
                != SUCCESS)
        {
            IAS_LOG_ERROR("Converting ECEF coordinate to ECI J2000");
            free(eci_pos);
            free(eci_vel);
            free(raw_pos);
            free(raw_vel);
            return ERROR;
        }

        ias_math_compute_3dvec_cross(&eci_pos[valid_eph_count],
            &eci_vel[valid_eph_count], &momentum_vector);
        mag_of_ang_momentum = ias_math_compute_vector_length(&momentum_vector);

        /* Perform orbital radius check */
        orbit_radius = ias_math_compute_vector_length(
            &eci_pos[valid_eph_count]);

        if (fabs(mag_of_ang_momentum
                 - orbit_parameters->nominal_angular_momentum)
                <= anc_qa_thresholds->angular_momentum_tolerance
            && /* both conditions must be met */
            fabs(orbit_radius - nom_orbit_radius_in_meters)
                <= anc_qa_thresholds->orbit_radius_tolerance)
        {
            /* If angular momentum and orbital radius check out, increment the
               count of valid ephemeris points */

#if DEBUG_GENERATE_DATA_FILES == 1
            fprintf(ofp,
                "%d %f %f %f %e %e %e %e %e %e\n",
                eph_index,
                ecef2eci_time[0], ecef2eci_time[1], ecef2eci_time[2],
                ecef_pos->x, ecef_pos->y, ecef_pos->z,
                ecef_vel->x, ecef_vel->y, ecef_vel->z);
#endif
            /* Increment to the next save position in the arrays. */
            valid_eph_count++;
        }
        else
        {
            IAS_LOG_DEBUG("Eliminated Ephemeris outlier index:%d x pos:%f "
                         "y pos:%f z pos:%f x vel:%f y vel:%f z vel:%f",
                         eph_index,
                         ecef_pos->x, ecef_pos->y, ecef_pos->z,
                         ecef_vel->x, ecef_vel->y, ecef_vel->z);
            invalid_eph_count++;
        }
    }

    /* Exit if no valid ephemeris points were found */
    if (valid_eph_count < 1)
    {
        IAS_LOG_ERROR("No valid ephemeris points were found.");
        free(eci_pos);
        free(eci_vel);
        free(raw_pos);
        free(raw_vel);
        return ERROR;
    }

    if (valid_eph_count >= IAS_LAGRANGE_PTS)
    {
        number_of_eph_points = valid_eph_count;
    }
    else
    {
        number_of_eph_points = IAS_LAGRANGE_PTS;
    }

    /* Propagate additional points if there are not enough for Lagrange
       interpolation */
    for (eph_index = valid_eph_count; eph_index < number_of_eph_points;
        eph_index++)
    {
        delta_time = 1.0;

        IAS_LOG_INFO("Propagating ephemeris data into index %d to have enough "
            "to do Lagrange interpolation", eph_index);

        /* Propagate time */
        smoothed_ephemeris_seconds_since_j2000[eph_index] =
            smoothed_ephemeris_seconds_since_j2000[eph_index - 1] + delta_time;

        /* Propagate position */
        eci_pos[eph_index].x = eci_pos[eph_index - 1].x
            + eci_vel[eph_index - 1].x * delta_time;
        eci_pos[eph_index].y = eci_pos[eph_index - 1].y
            + eci_vel[eph_index - 1].y * delta_time;
        eci_pos[eph_index].z = eci_pos[eph_index - 1].z
            + eci_vel[eph_index - 1].z * delta_time;

        /* Propagate velocity */
        acceleration_x = ias_geo_compute_earth_second_partial_x(
            earth_constants->semi_major_axis,
            earth_constants->gravity_constant,
            eci_pos[eph_index - 1].x, eci_pos[eph_index - 1].y,
            eci_pos[eph_index - 1].z);
        acceleration_y = ias_geo_compute_earth_second_partial_y(
            earth_constants->semi_major_axis,
            earth_constants->gravity_constant,
            eci_pos[eph_index - 1].x, eci_pos[eph_index - 1].y,
            eci_pos[eph_index - 1].z);
        acceleration_z = ias_geo_compute_earth_second_partial_z(
            earth_constants->semi_major_axis,
            earth_constants->gravity_constant,
            eci_pos[eph_index - 1].x, eci_pos[eph_index - 1].y,
            eci_pos[eph_index - 1].z);
        eci_vel[eph_index].x = eci_vel[eph_index - 1].x + acceleration_x
            * delta_time;
        eci_vel[eph_index].y = eci_vel[eph_index - 1].y + acceleration_y
            * delta_time;
        eci_vel[eph_index].z = eci_vel[eph_index - 1].z + acceleration_z
            * delta_time;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    fclose(ofp);

    ofp = fopen("eci.vectors.dat", "w");

    for (eph_index = 0; eph_index < valid_eph_count; eph_index++)
    {
        fprintf(ofp, "%f %e %e %e %e %e %e\n",
            epoch_time[2] - smoothed_ephemeris_seconds_since_j2000[0]
                          + smoothed_ephemeris_seconds_since_j2000[eph_index],
            eci_pos[eph_index].x, eci_pos[eph_index].y, eci_pos[eph_index].z,
            eci_vel[eph_index].x, eci_vel[eph_index].y, eci_vel[eph_index].z);
    }

    fclose(ofp);
#endif

    /* Smooth GPS data to remove small random errors present in the data. */
    IAS_LOG_DEBUG("Number of valid ephemeris found %d", valid_eph_count);

    /* Fix any inconsistencies in ephemeris time stamps */
    if (ias_ancillary_correct_ephemeris_time(eci_pos, eci_vel,
            number_of_eph_points, smoothed_ephemeris_seconds_since_j2000)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Smoothing ephemeris time");
        free(eci_pos);
        free(eci_vel);
        free(raw_pos);
        free(raw_vel);
        return ERROR;
    }

    /* Loop over all the smoothed time stamps to interpolate the ephemeris info
       at that time, stopping when the full time span is covered or until all
       the array entries are filled.  At a minimum, the ephemeris count needs
       to reach IAS_LAGRANGE_PTS. */
    lagrange_stop_time = smoothed_ephemeris_seconds_since_j2000[0];
    eph_count = 0;
    max_lagrange_start_index = number_of_eph_points - IAS_LAGRANGE_PTS;
    lagrange_start_index = 0;
    while ((lagrange_stop_time <=
           smoothed_ephemeris_seconds_since_j2000[number_of_eph_points - 1]
            || eph_count < IAS_LAGRANGE_PTS)
           && eph_count < number_of_eph_points)
    {
        /* Find Lagrange interpolation start index */
        while ((lagrange_start_index < number_of_eph_points) &&
               (smoothed_ephemeris_seconds_since_j2000[lagrange_start_index]
                <= lagrange_stop_time))
        {
            lagrange_start_index++;
        }

        lagrange_start_index -= IAS_LAGRANGE_PTS / 2;

        if (lagrange_start_index < 0)
            lagrange_start_index = 0;

        if (lagrange_start_index > max_lagrange_start_index)
            lagrange_start_index = max_lagrange_start_index;

        /* Interpolate position and velocity vector using Lagrange
           interpolation. */
        ias_geo_lagrange_interpolate(
                 &smoothed_ephemeris_seconds_since_j2000[lagrange_start_index],
                 &eci_pos[lagrange_start_index],
                 &eci_vel[lagrange_start_index],
                 IAS_LAGRANGE_PTS,
                 lagrange_stop_time,
                 &raw_pos[eph_count],
                 &raw_vel[eph_count]);

        lagrange_stop_time += IAS_EPHEM_SAMPLING_PERIOD;

        eph_count++;
    }
    valid_eph_count = eph_count;

    /* free memory no longer used */
    free(eci_pos);
    free(eci_vel);
    eci_pos = NULL;
    eci_vel = NULL;

    /* Time is now at evenly spaced increments */
    for (eph_index = 1; eph_index < valid_eph_count; eph_index++)
    {
        smoothed_ephemeris_seconds_since_j2000[eph_index]
            = smoothed_ephemeris_seconds_since_j2000[eph_index - 1]
            + IAS_EPHEM_SAMPLING_PERIOD;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    ofp = fopen("raw.eci.dat", "w");

    for (eph_index = 0; eph_index < valid_eph_count; eph_index++)
    {
        fprintf(ofp, "%f %e %e %e %e %e %e\n",
            epoch_time[2] - smoothed_ephemeris_seconds_since_j2000[0]
                          + smoothed_ephemeris_seconds_since_j2000[eph_index],
            raw_pos[eph_index].x, raw_pos[eph_index].y, raw_pos[eph_index].z,
            raw_vel[eph_index].x, raw_vel[eph_index].y, raw_vel[eph_index].z);
    }

    fclose(ofp);
#endif

    /* Smooth GPS data to remove small random errors present in the data. */
    if (ias_ancillary_kalman_smooth_ephemeris(cpf,
            valid_eph_count, smoothed_ephemeris_seconds_since_j2000,
            raw_pos, raw_vel, IAS_EPHEM_SAMPLING_PERIOD,
            smoothed_eph_pos, smoothed_eph_vel) != SUCCESS)
    {
        IAS_LOG_ERROR("Smoothing ephemeris");
        free(raw_pos);
        free(raw_vel);
        return ERROR;
    }

    /* assign output values */
    *valid_ephemeris_count = valid_eph_count;
    *invalid_ephemeris_count = invalid_eph_count;

    /* free memory no longer used */
    free(raw_pos);
    free(raw_vel);

    return SUCCESS;
}
