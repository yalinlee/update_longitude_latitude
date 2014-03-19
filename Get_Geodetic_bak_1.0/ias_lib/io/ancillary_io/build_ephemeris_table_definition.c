/**********************************************************************
 NAME:          ias_ancillary_build_ephemeris_table_definition

 PURPOSE:   Builds the arrays defining the ephemeris table in the HDF5
            ancillary data file.

 RETURNS:   Integer status code of SUCCESS or ERROR

***********************************************************************/
#include <stdlib.h>
#include <hdf5.h>

#include "ias_const.h"
#include "ias_logging.h"

#include "ias_ancillary_io.h"
#include "ias_ancillary_io_private.h"


#define POSITION_DIMS 3                        /* Number of position array
                                                  elements */
#define VELOCITY_DIMS 3                        /* Number of velocity array
                                                  elements */

int ias_ancillary_build_ephemeris_table_definition
(
    const char *field_names[EPHEMERIS_NFIELDS], /* I/O: Names of table
                                                   fields  */
    size_t field_offsets[EPHEMERIS_NFIELDS],    /* I/O: Field offsets */
    hid_t field_types[EPHEMERIS_NFIELDS],       /* I/O: Field datatypes */
    size_t field_sizes[EPHEMERIS_NFIELDS],      /* I/O: Field sizes  */
    hid_t fields_to_close[EPHEMERIS_NFIELDS]    /* I/O: Array of open field
                                                   datatypes  */
)
{
    hsize_t position_dims = POSITION_DIMS;
    hsize_t velocity_dims = VELOCITY_DIMS;

    int i = 0;                        /* Field_counter */

    IAS_ANC_EPHEMERIS_RECORD ephemeris_record;  /* "Template" for record
                                                   data structure */


    /* Initialize all datatype fields as 'closed' */
    for (i = 0; i < EPHEMERIS_NFIELDS; i++)
         fields_to_close[i] = -1; 

    /* Dig into the record data structure.  Start with the elapsed time
       in seconds from the given epoch. */
    i = 0;
    field_offsets[i] = HOFFSET(IAS_ANC_EPHEMERIS_RECORD, seconds_from_epoch);
    field_names[i] = "Time From Epoch";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(ephemeris_record.seconds_from_epoch);

    field_offsets[++i] = HOFFSET(IAS_ANC_EPHEMERIS_RECORD, eci_position);
    field_names[i] = "ECI Position";
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &position_dims);
    if (field_types[i] < 0)
    {
        IAS_LOG_ERROR("Creating ECI position array type");
        ias_ancillary_cleanup_table_definition(fields_to_close,
            EPHEMERIS_NFIELDS);
        return ERROR;
    }
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(ephemeris_record.eci_position);

    field_offsets[++i] = HOFFSET(IAS_ANC_EPHEMERIS_RECORD, eci_velocity);
    field_names[i] = "ECI Velocity";
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &velocity_dims);
    if (field_types[i] < 0)
    {
        IAS_LOG_ERROR("Creating ECI velocity array type");
        ias_ancillary_cleanup_table_definition(fields_to_close,
           EPHEMERIS_NFIELDS);
        return ERROR;
    }
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(ephemeris_record.eci_velocity);

    field_offsets[++i] = HOFFSET(IAS_ANC_EPHEMERIS_RECORD, ecef_position);
    field_names[i] = "ECEF Position";
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &position_dims);
    if (field_types[i] < 0)
    {
        IAS_LOG_ERROR("Creating ECEF position array type");
        ias_ancillary_cleanup_table_definition(fields_to_close,
            EPHEMERIS_NFIELDS);
        return ERROR;
    }
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(ephemeris_record.ecef_position);

    field_offsets[++i] = HOFFSET(IAS_ANC_EPHEMERIS_RECORD, ecef_velocity);
    field_names[i] = "ECEF Velocity";
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &velocity_dims);
    if (field_types[i] < 0)
    {
        IAS_LOG_ERROR("Creating ECEF velocity array type");
        ias_ancillary_cleanup_table_definition(fields_to_close,
            EPHEMERIS_NFIELDS);
        return ERROR;
    }
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(ephemeris_record.ecef_velocity);

    /* Final error checks */
    if (i < (EPHEMERIS_NFIELDS - 1))
    {
        /* fewer fields were added than expected */
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1,
            EPHEMERIS_NFIELDS);
        ias_ancillary_cleanup_table_definition(fields_to_close,
            EPHEMERIS_NFIELDS);
        return ERROR;
    }
    else if (i >= EPHEMERIS_NFIELDS)
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
