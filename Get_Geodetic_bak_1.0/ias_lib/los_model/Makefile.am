SUBDIRS = sensor spacecraft . tests

# define the intermediate library to be created
noinst_LTLIBRARIES = liblos_model.la

# define the files included in the library
liblos_model_la_SOURCES = \
    ias_los_model_allocate.c \
    ias_los_model_apply_precision_params.c \
    ias_los_model_build_jitter_table.c \
    ias_los_model_calc_scene_corners.c \
    ias_los_model_get_moon_position_at_location.c \
    ias_los_model_get_satellite_state_vector_at_location.c \
    ias_los_model_initialize.c \
    ias_los_model_input_line_samp_to_geodetic.c \
    ias_los_model_lunar_projection.c \
    ias_los_model_set_cpf.c \
    ias_los_model_set_l0r.c

liblos_model_la_LIBADD = \
    sensor/libsensor.la \
    spacecraft/libspacecraft.la

# List of public headers that need to be installed.  Private header files
# should not be included in this list.
include_HEADERS = \
    ias_los_model.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS
	for d in $(SUBDIRS); do \
        if [ $$d != "." ]; then \
            cd $$d && make headers && cd ..; \
        fi \
	done
