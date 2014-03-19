/******************************************************************************
NAME: ias_geo_find_ariesha

PURPOSE: Find the Greenwich Hour Angle of Aries

RETURN VALUE: none

REFERENCE: "Methods Of Orbit Determination" by Escobal

      Code is from Naval Observatory Vector Astronomy Subroutines (NOVAS)
      C Version 1.0
      June 1996

      U. S. Naval Observatory
      Astronomical Applications Dept.
      3450 Massachusetts Ave., NW
      Washington, DC  20392-5420

NOTES:

- This routine isn't currently used by IAS, but it is used by Ingest.  So
  do not remove it without confirming with Ingest that they do not need it.
*****************************************************************************/
#include "ias_const.h"
#include "ias_geo.h"

void ias_geo_find_ariesha 
(
    double tjd,      /* I: full julian day */
    double seconds,  /* I: seconds of the day */
    double *gha      /* O: Greenwich hour angle */
)
{
    double tjd2;
    double tu;
    double gst0;
    double gst;
    double mid;
    double mid2;

    tjd2 = tjd - seconds / IAS_SEC_PER_DAY;
    tu = (tjd2 - 2415020.0) / 36525;
    mid = 36000.7689 * tu;
    mid2 = (mid / 360.0 - (int)(mid / 360.0)) * 360.0;
    gst0 = 99.69098 + mid2 + 0.00038708 * tu * tu;
    gst = (gst0 + seconds / 60.0 * 0.25068447);
    gst = (gst / 360.0 - (int)(gst / 360.0)) * 360.0;
    *gha = gst * ias_math_get_radians_per_degree();
}
