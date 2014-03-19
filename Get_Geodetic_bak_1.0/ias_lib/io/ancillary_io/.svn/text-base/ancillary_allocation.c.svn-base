/**********************************************************************

 NAME:                    ancillary_allocation

 PURPOSE: Implements routines to allocate and free attitude and
          ephemeris data buffers

***********************************************************************/
#include <stdlib.h>
#include <string.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_ancillary_io.h"


/*-----------------------------------------------------------------------
 NAME:              ias_ancillary_allocate_attitude

 PURPOSE: Initializes the number of records fields in an
          IAS_ANC_ATTITUDE_DATA structure, then allocates a buffer for it

 RETURNS: Pointer to an allocated IAS_ANC_ATTITUDE_DATA structure buffer if
          successful, NULL pointer if invalid number of attitude records
          or buffer allocation fails

-------------------------------------------------------------------------*/
IAS_ANC_ATTITUDE_DATA *ias_ancillary_allocate_attitude
(
    int number_of_records              /*I: number of ephemeris records */
)
{
    int allocated_structure_size = 0;      /* Final size of allocated
                                              IAS_ANC_ATTITUDE_DATA structure */
    IAS_ANC_ATTITUDE_DATA *attitude_data = NULL;
                                           /* Pointer to allocated
                                              data structure buffer */


    /* Calculate the size of the required buffer.  To do this, we'll
    have to add another

            (number_of_records-1)*sizeof(IAS_ANC_ATTITUDE_RECORD)

    bytes to the required number of bytes for the base IAS_ANC_ATTITUDE_DATA
    structure.   Since the attitude record array is the last member of
    the structure, the extra allocated space can be used by it.  */

    /* Verify that a "reasonable" number of attitude records has been
       input.  */
    if (number_of_records <= 0)
    {
        IAS_LOG_ERROR("Invalid number of attitude data records:  %d",
                      number_of_records);
    }
    else
    {
        allocated_structure_size = sizeof(IAS_ANC_ATTITUDE_DATA);
        allocated_structure_size += ((number_of_records - 1)
            * sizeof(IAS_ANC_ATTITUDE_RECORD));

        /* Allocate and initialize the buffer. */
        attitude_data = malloc(allocated_structure_size);
        if (attitude_data == NULL)
        {
            IAS_LOG_ERROR("Allocating main attitude data buffer");
        }
        else
        {
            memset(attitude_data, 0, allocated_structure_size);
            attitude_data->number_of_samples = number_of_records;
        }
    }

    /* Done */
    return attitude_data;
}



/*-----------------------------------------------------------------------
 NAME:              ias_ancillary_allocate_ephemeris

 PURPOSE: Initializes the number of records fields in an
          IAS_ANC_EPHEMERIS_DATA structure, then allocates a buffer for it

 RETURNS: Pointer to an allocated IAS_ANC_EPHEMERIS_DATA structure if
          successful, NULL pointer if invalid number of attitude records
          or buffer allocation fails

-------------------------------------------------------------------------*/
IAS_ANC_EPHEMERIS_DATA *ias_ancillary_allocate_ephemeris
(
    int number_of_records             /* I: number of ephemeris records */
)
{
    int allocated_structure_size = 0;        /* Final size of allocated
                                                IAS_ANC_EPHEMERIS_DATA
                                                structure */

    IAS_ANC_EPHEMERIS_DATA *ephemeris_data = NULL;
                                             /* Pointer to allocated data
                                                structure buffer */


    /* Calculate the size of the required buffer.  To do this, we'll
    have to add another
 
            (number_of_records-1)*sizeof(IAS_ANC_EPHEMERIS_RECORD)

    bytes to the required number of bytes for the base IAS_ANC_EPHEMERIS_DATA
    structure.   Since the ephemeris record array is the last member of
    the structure, the extra allocated space can be used by it.  */

    /* Verify that a "reasonable" number of ephemeris records has been
       input.  */
    if (number_of_records <= 0)
    {
        IAS_LOG_ERROR("Invalid number of ephemeris data records:  %d",
                      number_of_records);
    }
    else
    {
        allocated_structure_size = sizeof(IAS_ANC_EPHEMERIS_DATA);
        allocated_structure_size += ((number_of_records - 1)
            * sizeof(IAS_ANC_EPHEMERIS_RECORD));

        /* Allocate and initialize the buffer. */
        ephemeris_data = malloc(allocated_structure_size);
        if (ephemeris_data == NULL)
        {
            IAS_LOG_ERROR("Allocating ephemeris data buffer");
        }
        else
        {
            memset(ephemeris_data, 0, allocated_structure_size);
            ephemeris_data->number_of_samples = number_of_records;
        }
    }

    /* Done */
    return ephemeris_data;
}



/*-----------------------------------------------------------------------
 NAME:              ias_ancillary_free_attitude

 PURPOSE: Frees an allocated IAS_ANC_ATTITUDE_DATA structure

 RETURNS: Nothing

------------------------------------------------------------------------*/
void ias_ancillary_free_attitude
(
    IAS_ANC_ATTITUDE_DATA *attitude_data    /* I: Pointer to allocated
                                               attitude data buffer */
)
{
    /* Free the main data structure */
    free(attitude_data);
}



/*---------------------------------------------------------------------
 NAME:              ias_ancillary_free_ephemeris

 PURPOSE: Frees an allocated IAS_ANC_EPHEMERIS_DATA structure

 RETURNS: Nothing

-----------------------------------------------------------------------*/
void ias_ancillary_free_ephemeris
(
    IAS_ANC_EPHEMERIS_DATA *ephemeris_data   /* I: Pointer to allocated
                                                ephemeris data buffer */
)
{
    /* Free the main data structure */
    free(ephemeris_data);
}
