SUBDIRS = . tests

# define the intermediate library to be created
noinst_LTLIBRARIES = libgrid.la

# define the files included in the library
libgrid_la_SOURCES = \
    ias_grid_compute_rough_polynomial.c \
    ias_grid_find_cell.c \
    ias_grid_ils2ols_at_elevation.c \
    ias_grid_ols2ils.c

# List of public headers that need to be installed.  Private header files
# should not be included in this list.
include_HEADERS = ias_grid.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

# The following recurses into each of the subdirectories to install the public
# header files before the main build is done
#headers:
#	for d in $(SUBDIRS); do \
#        cd $$d && make headers && cd ..; \
#	done
