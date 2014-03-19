/**********************************************************************
Name:              ias_ancillary_io_private

Purpose: Provides references to HDF5 library and internal data structures
         not intended to be publically accessible.

***********************************************************************/
#ifndef IAS_ANCILLARY_IO_PRIVATE_H
#define IAS_ANCILLARY_IO_PRIVATE_H


#include <hdf5.h>                  /* Core HDF5 library interface */

#define ATTITUDE_NFIELDS  9
#define EPHEMERIS_NFIELDS 5

#define FILE_FORMAT_VERSION 1

/*********************************************************************
   Attitude and ephemeris table name/dataset #defines
**********************************************************************/
#define ATTITUDE_EPOCH_TIME_ATTRIBUTE_NAME "ATTITUDE_EPOCH_TIME"
#define ATTITUDE_DATA_TABLE_NAME "Attitude Data"
#define ATTITUDE_DATA_DATASET_NAME "Attitude Records"
#define EPHEMERIS_EPOCH_TIME_ATTRIBUTE_NAME "EPHEMERIS_EPOCH_TIME"
#define EPHEMERIS_DATA_TABLE_NAME "Ephemeris Data"
#define EPHEMERIS_DATA_DATASET_NAME "Ephemeris Records"

#define FILE_FORMAT_VERSION_ATTRIBUTE_NAME "FILE_FORMAT_VERSION"



/* HDF5-based function prototypes  */
int ias_ancillary_build_attitude_table_definition
(
    const char *field_names[ATTITUDE_NFIELDS], /* IO: Names of table fields  */
    size_t field_offsets[ATTITUDE_NFIELDS],    /* IO: Field offsets */
    hid_t field_types[ATTITUDE_NFIELDS],       /* IO: Field datatypes */
    size_t field_sizes[ATTITUDE_NFIELDS],      /* IO: Field sizes  */
    hid_t fields_to_close[ATTITUDE_NFIELDS]    /* IO: Array of open field 
                                                      datatypes  */
);

int ias_ancillary_build_ephemeris_table_definition
(
    const char *field_names[EPHEMERIS_NFIELDS], /* IO: Names of table fields  */
    size_t field_offsets[EPHEMERIS_NFIELDS],    /* IO: Field offsets */
    hid_t field_types[EPHEMERIS_NFIELDS],       /* IO: Field datatypes */
    size_t field_sizes[EPHEMERIS_NFIELDS],      /* IO: Field sizes  */
    hid_t fields_to_close[EPHEMERIS_NFIELDS]    /* IO: Array of open field
                                                       datatypes  */
);

void ias_ancillary_cleanup_table_definition
(
    hid_t fields_to_close[],       /* I: Array of open field datatypes */
    int number_of_fields           /* I: Total number of fields in
                                      fields_to_close[] array   */
);


#endif
