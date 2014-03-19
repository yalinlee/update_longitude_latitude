/******************************************************************************
Name: ias_sc_model_get_position_and_velocity_at_time

Purpose: Compute the satellite position and velocity at a time from ephemeris 
    reference time, by interpolating each component.

RETURNS: nothing

******************************************************************************/
#include <math.h>
#include "ias_geo.h"
#include "ias_spacecraft_model.h"

void ias_sc_model_get_position_and_velocity_at_time
(
    const IAS_SC_EPHEMERIS_MODEL *eph,/* I: ephemeris structure */
    IAS_ACQUISITION_TYPE acq_type, /* I: Image acquisition type */
    double eph_time,         /* I: Delta time from the reference time */
    IAS_VECTOR *satpos,      /* O: New satellite position at "dtime" */
    IAS_VECTOR *satvel       /* O: New satellite velocity at "dtime" */
)
{
    int index;
    int i;
    double times[IAS_LAGRANGE_PTS];
    IAS_VECTOR positions[IAS_LAGRANGE_PTS];
    IAS_VECTOR velocities[IAS_LAGRANGE_PTS];

    /* Compute the starting Lagrange index, limiting the index to fall within
       the available data */
    index = (int)floor(eph_time/eph->nominal_sample_time - IAS_LAGRANGE_PTS/2);
    if (index < 0)
        index = 0;
    if (index > eph->sample_count - IAS_LAGRANGE_PTS)
        index = eph->sample_count - IAS_LAGRANGE_PTS;

    /* Populate the time, position, and velocity arrays */
    for (i = 0; i < IAS_LAGRANGE_PTS; i++)
    {
        const IAS_SC_EPHEMERIS_RECORD *current
                = &eph->sample_records[index + i];

        times[i] = current->seconds_from_epoch;

        /* If acquisition is earth based used earth-fixed ephemeris.
           If acquisition is stellar or lunar based then use ECI ephemeris. */
        if (acq_type == IAS_EARTH)
        {
            positions[i] = current->precision_ecef_position;
            velocities[i] = current->precision_ecef_velocity;
        }
        else
        {
            positions[i] = current->precision_eci_position;
            velocities[i] = current->precision_eci_velocity;
        }
    }

    /* Calculate the satellite position */
    ias_geo_lagrange_interpolate(times, positions, velocities,
            IAS_LAGRANGE_PTS, eph_time, satpos, satvel);
}
