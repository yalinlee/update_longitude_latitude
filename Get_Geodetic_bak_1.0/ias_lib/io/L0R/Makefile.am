# subdirectories to include
SUBDIRS = tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libL0R.la

# define the source files included in the library
libL0R_la_SOURCES = \
    ias_l0r_band.c \
	ias_l0r.c \
	ias_l0r_anc.c \
	ias_l0r_hdf.c \
	ias_l0r_mta.c \
	ias_l0r_header.c

# headers to install
include_HEADERS = ias_l0r.h \
    ias_l0r_structures.h \
    ias_l0r_write_routines.h \
    ias_l0r_read_routines.h \
    ias_l0r_constants.h \
    ias_l0r_compare_routines.h \
    ias_l0r_version.h 

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @HDF5_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

