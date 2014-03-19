# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libL1R.la

# define the source files included in the library
libL1R_la_SOURCES = \
    ias_l1r_open_image.c \
    ias_l1r_close_image.c \
    ias_l1r_close_band.c \
    ias_l1r_open_band.c \
    ias_l1r_band_metadata.c \
    ias_l1r_file_metadata.c \
    ias_l1r_read_image.c \
    ias_l1r_write_image.c

# headers to install
include_HEADERS = ias_l1r.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @HDF5_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

