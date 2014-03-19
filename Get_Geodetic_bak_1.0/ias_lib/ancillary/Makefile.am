# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libancillary.la

# define the source files included in the library
libancillary_la_SOURCES = \
    ias_ancillary_preprocess.c \
    ias_ancillary_build_ephemeris.c \
    ias_ancillary_extract_valid_imu_data_window.c \
    ias_ancillary_extract_valid_quaternion_window.c \
    ias_ancillary_compute_spacecraft_attitude.c \
    ias_ancillary_smooth_ephemeris.c \
    ias_ancillary_convert_imu_to_attitude.c \
    ias_ancillary_convert_imu_to_acs.c \
    ias_ancillary_correct_ephemeris_time.c \
    ias_ancillary_correct_imu_orbital_motion.c \
    ias_ancillary_kalman_smooth_ephemeris.c \
    ias_ancillary_kalman_smooth_imu.c \
    ias_ancillary_preprocess_attitude.c \
    ias_ancillary_preprocess_ephemeris.c \
    ias_ancillary_identify_quaternion_outliers.c \
    ias_ancillary_get_position_and_velocity_at_time.c \
    ias_ancillary_get_quaternion_at_time.c \
    ias_ancillary_get_start_stop_frame_times.c

# headers to install
include_HEADERS = ias_ancillary.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @HDF5_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

