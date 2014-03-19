/******************************************************************************
Name: ias_geo_convert_sensor_los_to_spacecraft

Purpose:
Find the line of sight vector from the spacecraft to a point on the ground
by transforming the line of sight vector in sensor coordinates to perturbed
spacecraft coordinates.

Returns:
    SUCCESS or ERROR

NOTES:

ALGORITHM REFERENCES:
******************************************************************************
                        Property of the U.S. Government
                            USGS EROS Data Center
******************************************************************************/
#include "ias_types.h"
#include "ias_const.h"
#include "ias_structures.h"
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_geo.h"

int ias_geo_convert_sensor_los_to_spacecraft
(
    double oli2acs[3][3],          /* I: Sensor to attitude control reference */
    IAS_ACQUISITION_TYPE acq_type, /* I: Image acquisition type */
    const IAS_VECTOR *satpos,      /* I: Satellite position (meters) */
    const IAS_VECTOR *satvel,      /* I: Satellite velocity (m/s) */
    const IAS_VECTOR *sensor_los,  /* I: LOS to target vector (sensor coord)*/
    double roll,                   /* I: Corrected roll (radians) */
    double pitch,                  /* I: Corrected pitch (radians) */
    double yaw,                    /* I: Corrected yaw (radians) */
    double orb2ecf[3][3],          /* I: Orbit to ECEF transformation */
    double attpert[3][3],          /* I: Attitude perturbation matrix */
    IAS_VECTOR *pert_los,          /* O: Perturbed LOS (OCS-orbital coord) */
    IAS_VECTOR *new_los            /* O: New line of sight (ECEF coord) */
)
{
    IAS_VECTOR nav_los;         /* LOS vector in navigation coords */
    int status;

    /* Calculate the orbital-to-ECF and body-to-orbital orientation matrices */
    status = ias_geo_compute_orientation_matrices(satpos, satvel, roll,
            pitch, yaw, orb2ecf, attpert);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating orientation matrices");
        return ERROR;
    }

    /* Transform LOS from sensor to spacecraft coordinates 
       (navigation reference) */
    ias_math_transform_3dvec(sensor_los, oli2acs, &nav_los);

    /* Calculate the perturbed los vector (OCI Coords) */
    ias_math_transform_3dvec(&nav_los, attpert, pert_los);

    if (acq_type == IAS_STELLAR || acq_type == IAS_LUNAR)
    {
        new_los->x = pert_los->x;
        new_los->y = pert_los->y;
        new_los->z = pert_los->z;
    }
    else
    {
        /* Calculate the new line of sight vector */
        ias_math_transform_3dvec(pert_los, orb2ecf, new_los);
    }

    return SUCCESS;
}
