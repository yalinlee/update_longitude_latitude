/*****************************************************************************
NAME: ias_ancillary_build_ephemeris

PURPOSE: Generate the ECEF version of the ephemeris data.  Update the
         ephemeris model with both ECI and ECEF versions of the data.
         Copy both versions of the data to the ancillary ephemeris
         structure.  The ephemeris time is also applied to the model and
         ancillary structures.

RETURN VALUE: Type = int
    Value    Description
    -----    -----------
    SUCCESS  Successful completion
    ERROR    Operation failed
*****************************************************************************/

#include "ias_const.h"
#include "ias_logging.h"
#include "ias_cpf.h"
#include "ias_math.h"
#include "ias_geo.h"
#include "ias_ancillary_private.h"

int ias_ancillary_build_ephemeris
(
    IAS_CPF *cpf,                 /* I: CPF structure */
    int valid_ephemeris_count,    /* I: number of ephemeris points for the
                                        smoothed arrays */
    const double *smoothed_ephemeris_seconds_since_j2000, /* I: array of
                                              smoothed ephemeris seconds
                                              since j2000 */
    const IAS_VECTOR *smoothed_eph_pos, /* I: array of smoothed ephemeris
                                              position data */
    const IAS_VECTOR *smoothed_eph_vel, /* I: array of smoothed ephemeris
                                              velocity data */
    IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data /* O: pointer to ephem data */
)
{
    int index = 0;               /* loop counter */

    double epoch_time[3];        /* ephemeris time year, DOY, SOD */
    double pole_wander_x;        /* X shift pole wander */
    double pole_wander_y;        /* Y shift pole wander */
    double ut1_utc_correction;   /* UT1-UTC coord time difference */
    double modified_julian_date; /* modified julian date */

    IAS_VECTOR ecef_pos;         /* ephemeris position data in ECEF */
    IAS_VECTOR ecef_vel;         /* ephemeris velocity data in ECEF */
    IAS_ANC_EPHEMERIS_RECORD *anc_eph_records = NULL;/* ptr to eph records */

#if DEBUG_GENERATE_DATA_FILES == 1
    FILE *ofp = NULL;
#endif

    /* Set a pointer to the records */
    anc_eph_records = anc_ephemeris_data->records;

    /* smoothed_ephemeris_seconds_since_j2000 is still in spacecraft
       use it to recalculate epoch time */
    if (ias_math_convert_j2000_seconds_to_year_doy_sod(
            smoothed_ephemeris_seconds_since_j2000[0], epoch_time) != SUCCESS)
    {
        IAS_LOG_ERROR("Converting J2000 seconds %lf to Year, DOY, SOD format",
                smoothed_ephemeris_seconds_since_j2000[0]);
        return ERROR;
    }

    /* get x and y shift pole wander and UT1-UTC time difference */
    if (ias_geo_compute_getmjdcoords(epoch_time, cpf, &modified_julian_date,
                                     &pole_wander_x, &pole_wander_y,
                                     &ut1_utc_correction) == ERROR)
    {
        IAS_LOG_ERROR("Establishing Earth Model");
        return ERROR;
    }

    /* Load ephemeris data into ancillary data structure */
    anc_ephemeris_data->number_of_samples = valid_ephemeris_count;
    anc_ephemeris_data->utc_epoch_time[0] = epoch_time[0];
    anc_ephemeris_data->utc_epoch_time[1] = epoch_time[1];
    anc_ephemeris_data->utc_epoch_time[2] = epoch_time[2];

    /* Fill ephemeris record structure */
    for (index = 0; index < valid_ephemeris_count; index++)
    {
        anc_eph_records[index].seconds_from_epoch
            = smoothed_ephemeris_seconds_since_j2000[index]
            - smoothed_ephemeris_seconds_since_j2000[0];

        anc_eph_records[index].eci_position[0] = smoothed_eph_pos[index].x;
        anc_eph_records[index].eci_position[1] = smoothed_eph_pos[index].y;
        anc_eph_records[index].eci_position[2] = smoothed_eph_pos[index].z;

        anc_eph_records[index].eci_velocity[0] = smoothed_eph_vel[index].x;
        anc_eph_records[index].eci_velocity[1] = smoothed_eph_vel[index].y;
        anc_eph_records[index].eci_velocity[2] = smoothed_eph_vel[index].z;

        /* Convert true-of-date to ECEF */
        if (ias_geo_eci2ecef(pole_wander_x, pole_wander_y, ut1_utc_correction,
                         &smoothed_eph_pos[index], &smoothed_eph_vel[index],
                         epoch_time, &ecef_pos, &ecef_vel) != SUCCESS)
        {
            IAS_LOG_ERROR("Converting ECI coordinate to ECEF");
            return ERROR;
        }

        /* Save the values */
        anc_eph_records[index].ecef_position[0] = ecef_pos.x;
        anc_eph_records[index].ecef_position[1] = ecef_pos.y;
        anc_eph_records[index].ecef_position[2] = ecef_pos.z;

        anc_eph_records[index].ecef_velocity[0] = ecef_vel.x;
        anc_eph_records[index].ecef_velocity[1] = ecef_vel.y;
        anc_eph_records[index].ecef_velocity[2] = ecef_vel.z;

        /* Adjust epoch time by one sample step. */
        ias_math_add_seconds_to_year_doy_sod(IAS_EPHEM_SAMPLING_PERIOD,
                                             epoch_time);
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    ofp = fopen("anc_eph.eci.dat", "w");

    for (index = 0; index < valid_ephemeris_count; index++)
    {
        fprintf(ofp,"%f %f %e %e %e %e %e %e\n",
        anc_ephemeris_data->records[index].seconds_from_epoch,
        anc_ephemeris_data->utc_epoch_time[2]
            + anc_eph_records[index].seconds_from_epoch,
        anc_eph_records[index].eci_position[0],
        anc_eph_records[index].eci_position[1],
        anc_eph_records[index].eci_position[2],
        anc_eph_records[index].eci_velocity[0],
        anc_eph_records[index].eci_velocity[1],
        anc_eph_records[index].eci_velocity[2]);
    }

    fclose(ofp);

    ofp = fopen("anc_eph.ecef.dat", "w");

    for (index = 0; index < valid_ephemeris_count; index++)
    {
        fprintf(ofp,"%f %f %e %e %e %e %e %e\n",
        anc_ephemeris_data->records[index].seconds_from_epoch,
        anc_ephemeris_data->utc_epoch_time[2]
            + anc_eph_records[index].seconds_from_epoch,
        anc_eph_records[index].ecef_position[0],
        anc_eph_records[index].ecef_position[1],
        anc_eph_records[index].ecef_position[2],
        anc_eph_records[index].ecef_velocity[0],
        anc_eph_records[index].ecef_velocity[1],
        anc_eph_records[index].ecef_velocity[2]);
    }

    fclose(ofp);
#endif

    return SUCCESS;
}
