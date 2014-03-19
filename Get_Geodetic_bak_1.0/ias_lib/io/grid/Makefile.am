# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libgrid.la

# define the source files included in the library
libgrid_la_SOURCES = \
        ias_grid_io.c 

# headers to install
include_HEADERS = ias_grid_io.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @HDF5_INCLUDES@ @GCTP_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

