/**********************************************************************
 NAME:          ias_ancillary_build_attitude_table_definition

 PURPOSE:   Builds the arrays defining the attitude table in the HDF5
            ancillary data file.

 RETURNS:   Integer status code of SUCCESS or ERROR

***********************************************************************/
#include <stdlib.h>
#include <hdf5.h>

#include "ias_logging.h"
#include "ias_structures.h"
#include "ias_const.h"

#include "ias_ancillary_io.h"
#include "ias_ancillary_io_private.h"

#define QDIMS 4                                /* Number of quaternion
                                                  array elements */


int ias_ancillary_build_attitude_table_definition
(
    const char *field_names[ATTITUDE_NFIELDS], /* I/O: Names of table
                                                  fields */
    size_t field_offsets[ATTITUDE_NFIELDS],    /* I/O: Field offsets */
    hid_t field_types[ATTITUDE_NFIELDS],       /* I/O: Field datatypes */
    size_t field_sizes[ATTITUDE_NFIELDS],      /* I/O: Field sizes  */
    hid_t fields_to_close[ATTITUDE_NFIELDS]    /* I/O: Array of open field 
                                                  datatypes  */
)

{
    hsize_t quaternion_dims = QDIMS;          /* Number of quaternion
                                                 components */

    int i;                                    /* Field counter */

    IAS_ANC_ATTITUDE_RECORD attitude_record;  /* "Template" for record
                                                 data structure */


    /* Initialize all datatype fields as 'closed' */
    for (i = 0; i < ATTITUDE_NFIELDS; i++)
         fields_to_close[i] = -1;

    /* Dig into the record data structure.  Start with the elapsed time
       in seconds from the given epoch. */
    i = 0;
    field_offsets[i] = HOFFSET(IAS_ANC_ATTITUDE_RECORD, seconds_from_epoch);
    field_names[i] = "Time From Epoch";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(attitude_record.seconds_from_epoch);

    field_offsets[++i] = HOFFSET(IAS_ANC_ATTITUDE_RECORD, eci_quaternion);
    field_names[i] = "ECI Quaternion";
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &quaternion_dims);
    if (field_types[i] < 0)
    {
        IAS_LOG_ERROR("Creating ECI quaternion array type");
        ias_ancillary_cleanup_table_definition(fields_to_close,
            ATTITUDE_NFIELDS);
        return ERROR;
    }
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(attitude_record.eci_quaternion);

    field_offsets[++i] = HOFFSET(IAS_ANC_ATTITUDE_RECORD, ecef_quaternion);
    field_names[i] = "ECEF Quaternion";
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &quaternion_dims);
    if (field_types[i] < 0)
    {
        IAS_LOG_ERROR("Creating ECEF quaternion array type");
        ias_ancillary_cleanup_table_definition(fields_to_close,
            ATTITUDE_NFIELDS);
        return ERROR;
    }
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(attitude_record.ecef_quaternion);

    field_offsets[++i] = HOFFSET(IAS_ANC_ATTITUDE_RECORD, roll);
    field_names[i] = "Roll";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(attitude_record.roll);

    field_offsets[++i] = HOFFSET(IAS_ANC_ATTITUDE_RECORD, roll_rate);
    field_names[i] = "Roll Rate";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(attitude_record.roll_rate);

    field_offsets[++i] = HOFFSET(IAS_ANC_ATTITUDE_RECORD, pitch);
    field_names[i] = "Pitch";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(attitude_record.pitch);

    field_offsets[++i] = HOFFSET(IAS_ANC_ATTITUDE_RECORD, pitch_rate);
    field_names[i] = "Pitch Rate";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(attitude_record.pitch_rate);

    field_offsets[++i] = HOFFSET(IAS_ANC_ATTITUDE_RECORD, yaw);
    field_names[i] = "Yaw";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(attitude_record.yaw);

    field_offsets[++i] = HOFFSET(IAS_ANC_ATTITUDE_RECORD, yaw_rate);
    field_names[i] = "Yaw Rate";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(attitude_record.yaw_rate);

    /* Final error checks */
    if (i < (ATTITUDE_NFIELDS - 1))
    {
        /* fewer fields were added than expected */
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1,
            ATTITUDE_NFIELDS);
        ias_ancillary_cleanup_table_definition(fields_to_close,
            ATTITUDE_NFIELDS);
        return ERROR;
    }
    else if (i >= ATTITUDE_NFIELDS)
    {
        /* more fields than expected.  The stack is probably now corrupt so
           just exit since this is an obvious programming error that was just
           introduced. */
        IAS_LOG_ERROR("Too many fields found - stack probably "
                      "corrupted - exiting");
        exit(EXIT_FAILURE);
    }


    /* Done */
    return SUCCESS;
}
