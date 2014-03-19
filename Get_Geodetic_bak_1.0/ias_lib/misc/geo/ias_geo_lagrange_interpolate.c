/******************************************************************************
Name: ias_geo_lagrange_interpolate

Purpose: Use Lagrange interpolation to interpolate the spacecraft position and
    velocity at a requested time.

Returns:
    No return value

******************************************************************************/
#include "ias_geo.h"

void ias_geo_lagrange_interpolate
( 
    const double *seconds_from_ref, /* I: Array of n_pts reference times */
    const IAS_VECTOR *position, /* I: Array of n_pts position vectors */
    const IAS_VECTOR *velocity, /* I: Array of n_pts velocity vectors */
    int n_pts,           /* I: Number of points to use in interpolation */
    double delta_time,   /* I: Delta time from the reference time */
    IAS_VECTOR *interpolated_position,
                         /* O: New satellite position at delta_time */
    IAS_VECTOR *interpolated_velocity
                         /* O: New satellite velocity at delta_time */
)
{
    double ptermx;      /* position vector X term */
    double ptermy;      /* position vector Y term */
    double ptermz;      /* position vector Z term */
    double vtermx;      /* velocity vector X term */
    double vtermy;      /* velocity vector Y term */
    double vtermz;      /* velocity vector Z term */
    double psumx;       /* position vector sum X term */
    double psumy;       /* position vector sum Y term */
    double psumz;       /* position vector sum Z term */
    double vsumx;       /* velocity vector sum X term */
    double vsumy;       /* velocity vector sum Y term */
    double vsumz;       /* velocity vector sum Z term */
    int i;              /* point loop counter */
    int j;              /* second point loop counter */

    psumx = psumy = psumz = 0.0;
    vsumx = vsumy = vsumz = 0.0;

    for (i = 0; i < n_pts; i++)
    {
        const IAS_VECTOR *pos = &position[i];
        const IAS_VECTOR *vel = &velocity[i];
        ptermx = pos->x;
        ptermy = pos->y;
        ptermz = pos->z;
        vtermx = vel->x;
        vtermy = vel->y;
        vtermz = vel->z;

        for (j = 0; j < n_pts; j++)
        {
            if (j != i)
            {
                double scale;
                scale = (delta_time - seconds_from_ref[j])
                      / (seconds_from_ref[i] - seconds_from_ref[j]);
                ptermx = ptermx * scale;
                ptermy = ptermy * scale;
                ptermz = ptermz * scale;
                vtermx = vtermx * scale;
                vtermy = vtermy * scale;
                vtermz = vtermz * scale;
            }
        }
        psumx = psumx + ptermx;
        psumy = psumy + ptermy;
        psumz = psumz + ptermz;
        vsumx = vsumx + vtermx;
        vsumy = vsumy + vtermy;
        vsumz = vsumz + vtermz;
    }

    interpolated_position->x = psumx;
    interpolated_position->y = psumy;
    interpolated_position->z = psumz;
    interpolated_velocity->x = vsumx;
    interpolated_velocity->y = vsumy;
    interpolated_velocity->z = vsumz;
}
