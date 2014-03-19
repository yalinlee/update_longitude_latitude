/*-----------------------------------------------------------------------------

NAME: ias_l0r.h

PURPOSE: Includes all the separate header files which are needed to use
         the L0R Library

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/

#ifndef IAS_L0R_H
#define IAS_L0R_H

#include "ias_l0r_constants.h"
#include "ias_l0r_structures.h"
#include "ias_l0r_read_routines.h"
#include "ias_l0r_write_routines.h"
#include "ias_l0r_compare_routines.h"

/* Macro to determine the information in an IAS_L0R_TIME structure into
   an equivalent timestamp */
#define IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(a) \
    (((a).days_from_J2000 * IAS_SEC_PER_DAY) + (a).seconds_of_day)

#endif
