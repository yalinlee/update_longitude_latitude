/*****************************************************************************
NAME: ias_ancillary_get_quaternion_at_time

PURPOSE: Compute the quaternion values at a specified time from ephemeris 
         reference time.

RETURN VALUE: Type = int
    Value    Description
    -----    -----------
    SUCCESS  Successful completion
    ERROR    Operation failed

NOTES:
    - The euler matrix passed in will be converted into an ACS-to-orb matrix
      for the ECI coordinates when processing Lunar or Stellar collections and
      returned to the caller.  This is to allow it to be used for later calls
      dealing with ECEF transformations.

*****************************************************************************/

#include <string.h>
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_geo.h"
#include "ias_ancillary_private.h"

int ias_ancillary_get_quaternion_at_time
(
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                     /* I: Pointer to ephem data */
    IAS_ACQUISITION_TYPE acq_type,   /* I: Image acquisition type */
    IAS_COORDINATE_SYSTEM coordinate_type, /* I: Coordinate system to generate
                                                 values for */
    double delta_time,               /* I: Time from the current record */
    double error_tolerance,          /* I: error tolerance for the conversion */
    double euler[3][3],              /* I/O: Euler rotational matrix */
    IAS_QUATERNION *euler_quat       /* O: Euler matrix in quaternion */
)
{
    IAS_VECTOR pos;                     /* Satellite positional vector */
    IAS_VECTOR vel;                     /* Satellite velocity vector */
    double orbital_trans_matrix[3][3];  /* Orbital transformation matrix */
    double ecef_eci_trans_matrix[3][3]; /* ECEF/ECI transformation matrix */
    double quat_trans_matrix[3][3];     /* Quaternion transformation matrix */
    IAS_QUATERNION converted_quat;      /* Quaternion from Euler matrix */ 

    ias_ancillary_get_position_and_velocity_at_time(anc_ephemeris_data, 
        coordinate_type, delta_time, &pos, &vel);

    /* Get the orbital transformation matrix */
    if (ias_geo_create_transformation_matrix(&pos, &vel, orbital_trans_matrix)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Determining orbital transformation matix");
        return ERROR;
    }

    /* For the ECI coordinate type, update the euler matrix for lunar and
       stellar acquisitions */
    if (coordinate_type == IAS_ECI
        && (acq_type == IAS_LUNAR || acq_type == IAS_STELLAR))
    {
        double temp_matrix[3][3];       

        /* Multiply ECI-to-orb with ACS-to-ECI (euler) to get ACS-to-orb */
        ias_math_multiply_3x3_matrix(orbital_trans_matrix, euler,
                    temp_matrix);
        memcpy(euler, temp_matrix, sizeof(temp_matrix));
    }

    /* Transpose the orbital matrix to get the ECEF/ECI transformation matrix */
    ias_math_transpose_matrix(*orbital_trans_matrix, *ecef_eci_trans_matrix,
                              3, 3);
 
    /* Multiply the ECEF/ECI transformation matrix with the euler rotational 
       matrix to get the quaternion transformation matrix */
    ias_math_multiply_3x3_matrix(ecef_eci_trans_matrix, euler,
                                 quat_trans_matrix);

    if (ias_math_convert_euler_to_quaternion(
            error_tolerance, quat_trans_matrix, &converted_quat) != SUCCESS)
    {
        IAS_LOG_ERROR("Converting euler to quaternion");
        return ERROR;
    }

    /* Convert the ACS to ECEF/ECI quaternion to ECEF/ECI to ACS */
    ias_math_conjugate_quaternion(&converted_quat, euler_quat); 

    return SUCCESS;
}
