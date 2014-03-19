# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libcpf_file_io.la

# define the source files included in the library
libcpf_file_io_la_SOURCES = \
    ias_cpf_cache.c \
    ias_cpf_convert_3digit_month_to_number.c \
    ias_cpf_free.c \
    ias_cpf_parse_ancil_qa_thresholds.c \
    ias_cpf_parse_ancil_conv.c \
    ias_cpf_parse_attitude_params.c \
    ias_cpf_parse_b2b_assessment.c \
    ias_cpf_parse_diffuser_rad.c \
    ias_cpf_parse_earth_const.c \
    ias_cpf_parse_file_attributes.c \
    ias_cpf_parse_focal_plane_cal.c \
    ias_cpf_parse_gcp_correlation.c \
    ias_cpf_parse_geo_system.c \
    ias_cpf_parse_histogram_characterization.c \
    ias_cpf_parse_i2i_assessment.c \
    ias_cpf_parse_impulse_noise.c \
    ias_cpf_parse_lamp_radiance.c \
    ias_cpf_parse_los_model_correction.c \
    ias_cpf_parse_lunar_irradiance.c \
    ias_cpf_parse_nonuniformity.c \
    ias_cpf_parse_oli_abs_gains.c \
    ias_cpf_parse_oli_avg_bias.c \
    ias_cpf_parse_oli_det_offsets.c \
    ias_cpf_parse_oli_det_noise.c \
    ias_cpf_parse_oli_det_status.c \
    ias_cpf_parse_oli_focal_plane.c \
    ias_cpf_parse_oli_parameters.c \
    ias_cpf_parse_oli_radiance_rescale.c \
    ias_cpf_parse_orbit_parameters.c \
    ias_cpf_parse_reflect_conv.c \
    ias_cpf_parse_relative_gains.c \
    ias_cpf_parse_saturation_level.c \
    ias_cpf_parse_sca_parameters.c \
    ias_cpf_parse_oli_temp_sens.c \
    ias_cpf_parse_tirs_temp_sens.c \
    ias_cpf_parse_tirs_abs_gains.c \
    ias_cpf_parse_tirs_abs_gains_blind.c \
    ias_cpf_parse_tirs_align_cal.c \
    ias_cpf_parse_tirs_det_offsets.c \
    ias_cpf_parse_tirs_det_noise.c \
    ias_cpf_parse_tirs_det_status.c \
    ias_cpf_parse_tirs_det_status_blind.c \
    ias_cpf_parse_tirs_detector_response.c \
    ias_cpf_parse_tirs_detector_response_blind.c \
    ias_cpf_parse_tirs_focal_plane.c \
    ias_cpf_parse_tirs_parameters.c \
    ias_cpf_parse_tirs_radiance_rescale.c \
    ias_cpf_parse_tirs_thermal_constants.c \
    ias_cpf_parse_tirs_rel_gains_blind.c \
    ias_cpf_parse_ut1_times.c \
    ias_cpf_parse_cloud_cover_assessment.c \
    ias_cpf_read.c

# headers to install
include_HEADERS = ias_cpf.h

# include headers from the IAS include directory
#INCLUDES = @IAS_INCLUDES@ @ODL_INCLUDES@
INCLUDES = @IAS_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

