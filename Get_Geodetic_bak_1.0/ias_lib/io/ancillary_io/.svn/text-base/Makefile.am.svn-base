# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libancillary_io.la

# define the source files included in the library
libancillary_io_la_SOURCES = \
    ancillary_allocation.c \
    build_attitude_table_definition.c \
    build_ephemeris_table_definition.c \
    ancillary_write.c \
    ancillary_read.c \
    ancillary_miscellaneous.c

# headers to install
include_HEADERS = ias_ancillary_io.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @HDF5_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

