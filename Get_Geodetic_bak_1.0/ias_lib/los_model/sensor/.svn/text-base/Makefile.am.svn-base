SUBDIRS = . tests

# define the intermediate library to be created
noinst_LTLIBRARIES = libsensor.la

# define the files included in the library
libsensor_la_SOURCES = \
    ias_sensor_adjust_los_for_ssm.c \
    ias_sensor_align_ssm_data.c \
    ias_sensor_check_ssm_encoder_data.c \
    ias_sensor_find_los_vector.c \
    ias_sensor_find_time.c \
    ias_sensor_get_jitter.c \
    ias_sensor_get_maximum_detector_delay.c \
    ias_sensor_set_cpf.c \
    ias_sensor_set_l0r.c \
    ias_sensor_set_frame_times.c \
    ias_sensor_set_ssm_from_l0r.c \
    ias_sensor_smooth_ssm_data.c

libsensor_la_LIBADD =

# List of public headers that need to be installed.  Private header files
# should not be included in this list.
include_HEADERS = \
    ias_sensor_model.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ -Icalval

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

