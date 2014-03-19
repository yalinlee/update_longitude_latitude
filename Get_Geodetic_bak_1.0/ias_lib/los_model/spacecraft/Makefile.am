SUBDIRS = . tests

# define the intermediate library to be created
noinst_LTLIBRARIES = libspacecraft.la

# define the files included in the library
libspacecraft_la_SOURCES = \
    ias_sc_model_attitude_jitter_tools.c \
    ias_sc_model_correct_attitude.c \
    ias_sc_model_correct_ephem_convert_ecef2eci.c \
    ias_sc_model_find_attitude_at_time.c \
    ias_sc_model_get_position_and_velocity_at_time.c \
    ias_sc_model_set_ancillary.c

libspacecraft_la_LIBADD = 

# List of public headers that need to be installed.  Private header files
# should not be included in this list.
include_HEADERS = ias_spacecraft_model.h

# include headers from the IAS include directory
INCLUDES = -I.. @IAS_INCLUDES@ @REMEZ_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

# The following recurses into each of the subdirectories to install the public
# header files before the main build is done
#headers:
#	for d in $(SUBDIRS); do \
#        cd $$d && make headers && cd ..; \
#	done
