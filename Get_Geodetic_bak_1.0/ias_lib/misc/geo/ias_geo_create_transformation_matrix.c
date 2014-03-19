/*******************************************************************************
NAME: ias_geo_create_transformation_matrix

PURPOSE:
Compose the coordinate transformation matrix from ECI system to Orbit system

RETURN VALUE:
Type = int (SUCCESS or ERROR)

*******************************************************************************/
#include "ias_math.h"
#include "ias_logging.h"
#include "ias_structures.h"
#include "ias_geo.h"

int  ias_geo_create_transformation_matrix
(
    const IAS_VECTOR *satpos, /* I: pointer to sat. position vec */
    const IAS_VECTOR *satvel, /* I: pointer to sat. velocity vec */
    double eci2ob[3][3]       /* O: transformation matrix from ECI
                                    to orbit system */
)
{
    IAS_VECTOR scy, scx; /* Y and X vector in Orbit System */
    double mag; /* magnitude of vector */

    /* Unit Z vector as the last row in the transformation matrix */
    mag = ias_math_compute_vector_length(satpos);
    if (mag == 0.0)
    {
        IAS_LOG_ERROR("Magnitude of vector is zero...will result in divide"
            " by zero");
        return ERROR;
    }    

    eci2ob[2][0] = -satpos->x / mag;
    eci2ob[2][1] = -satpos->y / mag;
    eci2ob[2][2] = -satpos->z / mag;

    /* Unit Y vector as the second row in the transformation matrix */
    ias_math_compute_3dvec_cross(satpos, satvel, &scy);
    mag = ias_math_compute_vector_length(&scy);
    if (mag == 0.0)
    {
        IAS_LOG_ERROR("Magnitude of vector is zero...will result in divide"
            " by zero");
        return ERROR;
    }
    eci2ob[1][0] = -scy.x / mag;
    eci2ob[1][1] = -scy.y / mag;
    eci2ob[1][2] = -scy.z / mag;

    /* Unit X vector as the first row in the transformation matrix */
    ias_math_compute_3dvec_cross(&scy, satpos, &scx);
    mag = ias_math_compute_vector_length(&scx);
    if (mag == 0.0)
    {
        IAS_LOG_ERROR( "Magnitude of vector is zero...will result in divide"
            " by zero");
        return ERROR;
    }
    eci2ob[0][0] =  scx.x / mag;
    eci2ob[0][1] =  scx.y / mag;
    eci2ob[0][2] =  scx.z / mag;

    return SUCCESS;
}
