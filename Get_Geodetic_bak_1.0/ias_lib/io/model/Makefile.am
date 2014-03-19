# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libmodelio.la

# define the source files included in the library
libmodelio_la_SOURCES = \
	common_model_io.c \
	ias_model_io.c \
	read_model.c \
	write_model.c

# headers to install
include_HEADERS = ias_model_io.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @HDF5_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

