/*******************************************************************************
NAME: ias_math_compute_full_julian_date

PURPOSE: Find the full julian date.

RETURN VALUE:
Type = double


REFERENCES: "Practical Astronomy With Your Calculator" by Peter Duffett-Smith 
         Peter Duffet-Smith
         Second edition         Page 21

*******************************************************************************/
#include "ias_const.h"
#include "ias_math.h"

double ias_math_compute_full_julian_date 
(
    int yr,              /* I: year */ 
    int mo,              /* I: month */ 
    int day,             /* I: day */
    double seconds       /* I: seconds of day */
)
{

    double fjd;     /* julian date in the Julian or Gregorian calendar */
    int a;          /* a, b, c, and d correspond to intermediate */
    int b;          /* variables used in the book cited in the REFERENCES */
    int c;          /* section of the prologue */
    int d;
    double dday;    /* day of month plus fraction of day */

    dday = (double)(day) + seconds / (double)IAS_SEC_PER_DAY;

    if (mo == 1 || mo == 2) 
    {
        yr--;
        mo += 12;
    }

    a = (int)((double)(yr) / 100.0);
    b = 2 - a + (int)((double)(a) / 4.0);
    c = (int)(365.25 * yr);
    d = (int)(30.6001 * (mo + 1));
    fjd = b + c + d + dday + 1720994.5;
    return fjd;
}
