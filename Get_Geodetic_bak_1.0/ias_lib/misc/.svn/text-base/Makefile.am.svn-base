SUBDIRS = database_access geo math miscellaneous odl pixel_mask \
        satellite_attributes threading . tests

# define the intermediate library to be created
noinst_LTLIBRARIES = libmisc.la

# define the files included in the library
libmisc_la_SOURCES = \
    calval_band_map.c \
    ias_linked_list.c \
    ias_logging.c

libmisc_la_LIBADD = \
    database_access/libdatabase_access.la \
    geo/libgeo.la \
    math/libiasmath.la \
    miscellaneous/libmiscellaneous.la \
    odl/libodl.la \
    pixel_mask/libpixel_mask.la \
    satellite_attributes/libsatellite_attributes.la \
    threading/libthreading.la

# List of public headers that need to be installed.  Private header files
# should not be included in this list.
include_HEADERS = \
    ias_const.h \
    ias_linked_list.h \
    ias_logging.h \
    ias_structures.h \
    ias_types.h \
    ias_gcc_pragmas.h \
    calval_band_map.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS
	for d in $(SUBDIRS); do \
        if [ $$d != "." ]; then \
            cd $$d && make headers && cd ..; \
        fi \
	done
