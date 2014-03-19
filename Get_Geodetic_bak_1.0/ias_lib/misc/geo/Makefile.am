# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libgeo.la

# define the source files included in the library
libgeo_la_SOURCES = \
    ias_geo_analyze_gcp_records.c \
    ias_geo_check_start_end_date.c \
    ias_geo_compute_earth2orbit_transform.c \
    ias_geo_compute_forward_mappings.c \
    ias_geo_compute_getmjdcoords.c \
    ias_geo_compute_inverse_mappings.c \
    ias_geo_compute_map_edge.c \
    ias_geo_compute_orientation_matrices.c \
    ias_geo_compute_potential.c \
    ias_geo_compute_proj2ls_poly.c \
    ias_geo_compute_proj2proj_poly.c \
    ias_geo_novas_wrapper.c \
    ias_geo_convert_cart2sph.c \
    ias_geo_convert_deg2dms.c \
    ias_geo_convert_dms2deg.c \
    ias_geo_convert_geocentric_height_to_geodetic.c \
    ias_geo_convert_geod2cart.c \
    ias_geo_convert_sensor_los_to_spacecraft.c \
    ias_geo_convert_sph2cart.c \
    ias_geo_convert_rpy_to_matrix.c \
    ias_geo_convert_utc2times.c \
    ias_geo_correct_for_center_of_mass.c \
    ias_geo_correct_for_light_travel_time.c \
    ias_geo_correct_for_velocity_aberration.c \
    ias_geo_create_transformation_matrix.c \
    ias_geo_ecef2eci.c \
    ias_geo_eci2ecef.c \
    ias_geo_extract_window.c \
    ias_geo_find_ariesha.c \
    ias_geo_find_deg.c \
    ias_geo_find_earth_radius.c \
    ias_geo_find_min.c \
    ias_geo_find_mjdcoords.c \
    ias_geo_find_sec.c \
    ias_geo_find_target_position.c \
    ias_geo_get_units.c \
    ias_geo_handle_180.c \
    ias_geo_lagrange_interpolate.c \
    ias_geo_projection_transformation.c \
    ias_geo_report_proj_err.c \
    ias_geo_transform_nutation.c \
    ias_geo_transform_polar_motion.c \
    ias_geo_transform_precession.c \
    ias_geo_transform_ecef2j2k.c \
    ias_geo_transform_sidereal.c \
    ias_geo_transform_tod2j2k.c \
    ias_geo_transform_j2k2tod.c

# headers to install
include_HEADERS = ias_geo.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @GCTP_INCLUDES@ @NOVAS_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

