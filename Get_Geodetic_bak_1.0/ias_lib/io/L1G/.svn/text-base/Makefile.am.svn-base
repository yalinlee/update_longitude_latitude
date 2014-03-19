# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libL1G.la

# define the source files included in the library
libL1G_la_SOURCES = ias_l1g_open_image.c \
    ias_l1g_close_image.c \
    ias_l1g_close_band.c \
    ias_l1g_open_band.c \
    ias_l1g_band_metadata.c \
    ias_l1g_file_metadata.c \
    ias_l1g_read_image.c \
    ias_l1g_write_image.c

# headers to install
include_HEADERS = ias_l1g.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @HDF5_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

