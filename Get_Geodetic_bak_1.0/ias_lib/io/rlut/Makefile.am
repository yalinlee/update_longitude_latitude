# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = librlut_io.la

# define the source files included in the library
librlut_io_la_SOURCES = \
    ias_rlut2.c \
    ias_rlut_build_file_attributes_table_description.c \
    ias_rlut_build_linearization_params_table_description.c \
    ias_rlut_open_close_file.c \
    ias_rlut_read_file_attributes.c \
    ias_rlut_read_linearization_params.c \
    ias_rlut_write_file_attributes.c \
    ias_rlut_write_linearization_params.c \
    ias_rlut_write_tirs_secondary_linearization_params.c \
    ias_rlut_read_tirs_secondary_linearization_params.c

# headers to install
include_HEADERS = ias_rlut.h ias_rlut2.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @HDF5_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

