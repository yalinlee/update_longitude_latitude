/****************************************************************************

Major HACK warning.  This code is only here to provide a routine to convert
the band order in the cal/val test data to the band order used by LDCM.  This
is only used by the utilities that convert cal/val test data files to the 
format we use for the files.  Nobody else should be using these routines.

FIXME - completely eliminate this module when cal/val is able to provide test
data with the bands in the correct order.

****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "calval_band_map.h"

int convert_calval_band_index_to_oli
(
    int version,            /* I: version number since calval will reorder the
                                  bands some day */
    int calval_band_index
)
{
    static const int calval_to_oli_lut_v1[9] =
        {   7, 1, 0, 4, 3, 2, 6, 5, 8 };
    static const int calval_to_oli_lut_v2[9] =
        {   7, 0, 1, 2, 3, 4, 5, 6, 8 };

    if (calval_band_index < 0 || calval_band_index > 8)
    {
        printf("Error: illegal calval band index %d\n", calval_band_index);
        exit(1);
    }

    if (version == 1)
    {
        return calval_to_oli_lut_v1[calval_band_index];
    }
    else if (version == 2)
    {
        return calval_to_oli_lut_v2[calval_band_index];
    }
    else
    {
        printf("Error: unrecognized calval band version %d\n", version);
        exit(1);
    }
}

int convert_oli_band_index_to_calval
(
    int version,            /* I: version number since calval will reorder the
                                  bands some day */
    int oli_band_index
)
{
    static const int oli_to_calval_lut_v1[9] =
        { 2, 1, 5, 4, 3, 7, 6, 0, 8};
    static const int oli_to_calval_lut_v2[9] =
        { 1, 2, 3, 4, 5, 6, 7, 0, 8};

    if (oli_band_index < 0 || oli_band_index > 8)
    {
        printf("Error: illegal oli band index %d\n", oli_band_index);
        exit(1);
    }

    if (version == 1)
    {
        return oli_to_calval_lut_v1[oli_band_index];
    }
    else if (version == 2)
    {
        return oli_to_calval_lut_v2[oli_band_index];
    }
    else
    {
        printf("Error: unrecognized calval band version %d\n", version);
        exit(1);
    }
}

#if 0
void test_band_map(int calval_version)
{
    int i;
    for (i = 0; i < 9; i++)
    {
        int oli_index = convert_calval_band_index_to_oli(calval_version, i);
        int calval_index = convert_oli_band_index_to_calval(calval_version,
                oli_index);
        printf("index %d: oli band number %d, calval band index %d\n",
               i, oli_index + 1, calval_index);
    }
}
#endif
