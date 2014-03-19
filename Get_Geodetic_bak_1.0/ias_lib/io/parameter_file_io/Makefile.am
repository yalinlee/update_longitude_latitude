# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libparameter_file_io.la

# define the source files included in the library
libparameter_file_io_la_SOURCES = \
    ias_parm_check_ranges.c \
    ias_parm_read.c \
    ias_parm_update.c \
    ias_parm_provide_help.c \
    ias_parm_map_odl_type.c

# headers to install
include_HEADERS = \
    ias_parm.h \
    ias_parm_standard_parameters.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

