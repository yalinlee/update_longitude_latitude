# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libgcp.la

# define the source files included in the library
libgcp_la_SOURCES = \
	ias_gcp_read_correlation_results.c \
	ias_gcp_read_gcplib.c \
	ias_gcp_read_image.c \
	ias_gcp_write_correlation_results.c \
	ias_gcp_write_gcplib.c \
	ias_gcp_write_image.c

# headers to install
include_HEADERS = ias_gcp.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@

# install the perl module ias_gcp.pm
install-data-local:
	mkdir -p ${prefix}/perllib
	$(INSTALL) -m 644 ias_gcp.pm -t ${prefix}/perllib

uninstall-local:
	rm -f ${prefix}/perllib/ias_gcp.pm

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

