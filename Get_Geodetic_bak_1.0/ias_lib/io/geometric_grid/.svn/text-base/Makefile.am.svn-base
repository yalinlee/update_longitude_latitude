# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libgeometric_grid.la

# define the source files included in the library
libgeometric_grid_la_SOURCES = \
    ias_geom_grid_io.c 

# headers to install
include_HEADERS = ias_geom_grid.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @HDF5_INCLUDES@ @GCTP_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS


