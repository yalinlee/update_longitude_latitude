#ifndef LOCAL_NOVAS_WRAPPER_H
#define LOCAL_NOVAS_WRAPPER_H
/* This file provides defines for NOVAS routines that are used by the routines
   in this library.  All NOVAS calls should stay in this library (therefore the
   "local" in the file name).  These defines are not technically needed, but
   calling NOVAS routines with these defines makes it clear where NOVAS code
   is being called since the routine names don't make that obvious. */
#include "novas.h"

/*****************************************************/
/* Define NOVAS values for transformation directions */
/*****************************************************/

/* Used to tell NOVAS to convert mean pole position to true */
#define NOVAS_MEAN_TO_TRUE_DIRECTION 0

/* Used to tell NOVAS to convert true pole position to mean */
#define NOVAS_TRUE_TO_MEAN_DIRECTION -1

/* Defines for NOVAS full or reduced accuracy */
#define NOVAS_FULL_ACCURACY 0
#define NOVAS_REDUCED_ACCURACY 1

/* Define for computing Greenwich appparent sidereal time (GAST) */
#define NOVAS_APPARENT_GAST 1

/* Define for using the equinox method */
#define NOVAS_EQUINOX_METHOD 1

/* Define the return value for a NOVAS routine succeeding */
#define NOVAS_SUCCESS 0

/* This routine calculates the small periodic difference between Terrestrial
   and Barycentric times */
#define NOVAS_TDB2TT tdb2tt

/* This routine performs precession for a given Barycentric time */
#define NOVAS_PRECESSION precession

/* This routine performs nutation for a given Barycentric time */
#define NOVAS_NUTATION nutation

/* This routine transforms polar motion */
#define NOVAS_WOBBLE wobble

/* This routine calculates the Greenwich sidereal time */
#define NOVAS_SIDEREAL_TIME sidereal_time

#endif
