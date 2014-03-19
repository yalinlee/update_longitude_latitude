# subdirectories to include 
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libbpf_file.la

# define the source files included in the library
libbpf_file_la_SOURCES =  ias_bpf_cache.c \
                          ias_bpf_get_model_parameters.c \
                          ias_bpf_mem.c \
                          ias_bpf_parse_bias_model.c \
                          ias_bpf_parse_file_attributes.c \
                          ias_bpf_parse_orbit_parameters.c \
                          ias_bpf_read.c \
                          ias_bpf_set_groups.c \
                          ias_bpf_write.c

# headers to install
include_HEADERS = ias_bpf.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

