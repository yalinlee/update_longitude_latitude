/******************************************************************************
Name: ias_ancillary_get_position_and_velocity_at_time

Purpose: Compute the satellite position and velocity at a time from ephemeris 
    reference time, by interpolating each component.

Returns:
    nothing / void

******************************************************************************/

#include <math.h>

#include "ias_geo.h"
#include "ias_ancillary_io.h"
#include "ias_ancillary_private.h"

void ias_ancillary_get_position_and_velocity_at_time
(
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,/* I: ephemeris struct */
    IAS_COORDINATE_SYSTEM coordinate_type, /* I: Coordinate system to generate
                                                 values for */
    double eph_time,         /* I: Delta time from the reference time */
    IAS_VECTOR *satpos,      /* O: New satellite position at "eph_time" */
    IAS_VECTOR *satvel       /* O: New satellite velocity at "eph_time" */
)
{
    int index;
    int start_index;
    double times[IAS_LAGRANGE_PTS];
    IAS_VECTOR positions[IAS_LAGRANGE_PTS];
    IAS_VECTOR velocities[IAS_LAGRANGE_PTS];

    /* Compute the starting Lagrange index, limiting the index to fall within
       the available data */
    start_index = (int)floor(eph_time / IAS_EPHEM_SAMPLING_PERIOD -
                             IAS_LAGRANGE_PTS / 2);
    if (start_index < 0)
        start_index = 0;
    if (start_index > anc_ephemeris_data->number_of_samples - IAS_LAGRANGE_PTS)
        start_index = anc_ephemeris_data->number_of_samples - IAS_LAGRANGE_PTS;

    /* Populate the time, position, and velocity arrays */
    for (index = 0; index < IAS_LAGRANGE_PTS; index++)
    {
        const IAS_ANC_EPHEMERIS_RECORD *current =
            &anc_ephemeris_data->records[start_index + index];

        times[index] = current->seconds_from_epoch;

        /* Depending on the coordinate type, use Earth-fixed or intertial 
           for the positions and velocities */
        if (coordinate_type == IAS_ECEF)
        {
            positions[index].x = current->ecef_position[0];
            positions[index].y = current->ecef_position[1];
            positions[index].z = current->ecef_position[2];

            velocities[index].x = current->ecef_velocity[0];
            velocities[index].y = current->ecef_velocity[1];
            velocities[index].z = current->ecef_velocity[2];
        }
        else
        {
            positions[index].x = current->eci_position[0];
            positions[index].y = current->eci_position[1];
            positions[index].z = current->eci_position[2];

            velocities[index].x = current->eci_velocity[0];
            velocities[index].y = current->eci_velocity[1];
            velocities[index].z = current->eci_velocity[2];
        }
    }

    /* Calculate the satellite position */
    ias_geo_lagrange_interpolate(times, positions, velocities,
            IAS_LAGRANGE_PTS, eph_time, satpos, satvel);
}
