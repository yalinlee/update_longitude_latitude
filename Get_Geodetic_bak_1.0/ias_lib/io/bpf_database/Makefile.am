# subdirectories to include 
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libbpf_database.la

# define the source files included in the library
libbpf_database_la_SOURCES = ias_bpf_database_insert.c \
    ias_bpf_database_populate_version.c

# headers to install
include_HEADERS = ias_bpf_database.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @ODL_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

