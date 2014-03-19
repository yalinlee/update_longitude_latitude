#ifndef CALVAL_BAND_MAP_H
#define CALVAL_BAND_MAP_H

/****************************************************************************

Major HACK warning.  This code is only here to provide a routine to convert
the band order in the cal/val test data to the band order used by LDCM.  This
is only used by the utilities that convert cal/val test data files to the 
format we use for the files.  Nobody else should be using these routines.

FIXME - completely eliminate this module when cal/val is able to provide test
data with the bands in the correct order.

****************************************************************************/

int convert_calval_band_index_to_oli
(
    int version,            /* I: version number since calval will reorder the
                                  bands some day */
    int calval_band_index
);

int convert_oli_band_index_to_calval
(
    int version,            /* I: version number since calval will reorder the
                                  bands some day */
    int oli_band_index
);

void test_band_map(int calval_version);

#endif
