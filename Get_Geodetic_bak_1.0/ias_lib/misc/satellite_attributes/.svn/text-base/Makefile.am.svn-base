# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libsatellite_attributes.la

# define the source files included in the library
libsatellite_attributes_la_SOURCES = \
    ias_satellite_attributes.c \
    landsat8.c

# headers to install
include_HEADERS = ias_satellite_attributes.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

