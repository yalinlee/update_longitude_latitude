################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ias_lib/misc/geo/ias_geo_analyze_gcp_records.c \
../ias_lib/misc/geo/ias_geo_check_start_end_date.c \
../ias_lib/misc/geo/ias_geo_compute_earth2orbit_transform.c \
../ias_lib/misc/geo/ias_geo_compute_forward_mappings.c \
../ias_lib/misc/geo/ias_geo_compute_getmjdcoords.c \
../ias_lib/misc/geo/ias_geo_compute_inverse_mappings.c \
../ias_lib/misc/geo/ias_geo_compute_map_edge.c \
../ias_lib/misc/geo/ias_geo_compute_orientation_matrices.c \
../ias_lib/misc/geo/ias_geo_compute_potential.c \
../ias_lib/misc/geo/ias_geo_compute_proj2ls_poly.c \
../ias_lib/misc/geo/ias_geo_compute_proj2proj_poly.c \
../ias_lib/misc/geo/ias_geo_convert_cart2sph.c \
../ias_lib/misc/geo/ias_geo_convert_deg2dms.c \
../ias_lib/misc/geo/ias_geo_convert_dms2deg.c \
../ias_lib/misc/geo/ias_geo_convert_geocentric_height_to_geodetic.c \
../ias_lib/misc/geo/ias_geo_convert_geod2cart.c \
../ias_lib/misc/geo/ias_geo_convert_rpy_to_matrix.c \
../ias_lib/misc/geo/ias_geo_convert_sensor_los_to_spacecraft.c \
../ias_lib/misc/geo/ias_geo_convert_sph2cart.c \
../ias_lib/misc/geo/ias_geo_convert_utc2times.c \
../ias_lib/misc/geo/ias_geo_correct_for_center_of_mass.c \
../ias_lib/misc/geo/ias_geo_correct_for_light_travel_time.c \
../ias_lib/misc/geo/ias_geo_correct_for_velocity_aberration.c \
../ias_lib/misc/geo/ias_geo_create_transformation_matrix.c \
../ias_lib/misc/geo/ias_geo_ecef2eci.c \
../ias_lib/misc/geo/ias_geo_eci2ecef.c \
../ias_lib/misc/geo/ias_geo_extract_window.c \
../ias_lib/misc/geo/ias_geo_find_ariesha.c \
../ias_lib/misc/geo/ias_geo_find_deg.c \
../ias_lib/misc/geo/ias_geo_find_earth_radius.c \
../ias_lib/misc/geo/ias_geo_find_min.c \
../ias_lib/misc/geo/ias_geo_find_mjdcoords.c \
../ias_lib/misc/geo/ias_geo_find_sec.c \
../ias_lib/misc/geo/ias_geo_find_target_position.c \
../ias_lib/misc/geo/ias_geo_get_units.c \
../ias_lib/misc/geo/ias_geo_handle_180.c \
../ias_lib/misc/geo/ias_geo_lagrange_interpolate.c \
../ias_lib/misc/geo/ias_geo_novas_wrapper.c \
../ias_lib/misc/geo/ias_geo_projection_transformation.c \
../ias_lib/misc/geo/ias_geo_report_proj_err.c \
../ias_lib/misc/geo/ias_geo_transform_ecef2j2k.c \
../ias_lib/misc/geo/ias_geo_transform_j2k2tod.c \
../ias_lib/misc/geo/ias_geo_transform_nutation.c \
../ias_lib/misc/geo/ias_geo_transform_polar_motion.c \
../ias_lib/misc/geo/ias_geo_transform_precession.c \
../ias_lib/misc/geo/ias_geo_transform_sidereal.c \
../ias_lib/misc/geo/ias_geo_transform_tod2j2k.c 

OBJS += \
./ias_lib/misc/geo/ias_geo_analyze_gcp_records.o \
./ias_lib/misc/geo/ias_geo_check_start_end_date.o \
./ias_lib/misc/geo/ias_geo_compute_earth2orbit_transform.o \
./ias_lib/misc/geo/ias_geo_compute_forward_mappings.o \
./ias_lib/misc/geo/ias_geo_compute_getmjdcoords.o \
./ias_lib/misc/geo/ias_geo_compute_inverse_mappings.o \
./ias_lib/misc/geo/ias_geo_compute_map_edge.o \
./ias_lib/misc/geo/ias_geo_compute_orientation_matrices.o \
./ias_lib/misc/geo/ias_geo_compute_potential.o \
./ias_lib/misc/geo/ias_geo_compute_proj2ls_poly.o \
./ias_lib/misc/geo/ias_geo_compute_proj2proj_poly.o \
./ias_lib/misc/geo/ias_geo_convert_cart2sph.o \
./ias_lib/misc/geo/ias_geo_convert_deg2dms.o \
./ias_lib/misc/geo/ias_geo_convert_dms2deg.o \
./ias_lib/misc/geo/ias_geo_convert_geocentric_height_to_geodetic.o \
./ias_lib/misc/geo/ias_geo_convert_geod2cart.o \
./ias_lib/misc/geo/ias_geo_convert_rpy_to_matrix.o \
./ias_lib/misc/geo/ias_geo_convert_sensor_los_to_spacecraft.o \
./ias_lib/misc/geo/ias_geo_convert_sph2cart.o \
./ias_lib/misc/geo/ias_geo_convert_utc2times.o \
./ias_lib/misc/geo/ias_geo_correct_for_center_of_mass.o \
./ias_lib/misc/geo/ias_geo_correct_for_light_travel_time.o \
./ias_lib/misc/geo/ias_geo_correct_for_velocity_aberration.o \
./ias_lib/misc/geo/ias_geo_create_transformation_matrix.o \
./ias_lib/misc/geo/ias_geo_ecef2eci.o \
./ias_lib/misc/geo/ias_geo_eci2ecef.o \
./ias_lib/misc/geo/ias_geo_extract_window.o \
./ias_lib/misc/geo/ias_geo_find_ariesha.o \
./ias_lib/misc/geo/ias_geo_find_deg.o \
./ias_lib/misc/geo/ias_geo_find_earth_radius.o \
./ias_lib/misc/geo/ias_geo_find_min.o \
./ias_lib/misc/geo/ias_geo_find_mjdcoords.o \
./ias_lib/misc/geo/ias_geo_find_sec.o \
./ias_lib/misc/geo/ias_geo_find_target_position.o \
./ias_lib/misc/geo/ias_geo_get_units.o \
./ias_lib/misc/geo/ias_geo_handle_180.o \
./ias_lib/misc/geo/ias_geo_lagrange_interpolate.o \
./ias_lib/misc/geo/ias_geo_novas_wrapper.o \
./ias_lib/misc/geo/ias_geo_projection_transformation.o \
./ias_lib/misc/geo/ias_geo_report_proj_err.o \
./ias_lib/misc/geo/ias_geo_transform_ecef2j2k.o \
./ias_lib/misc/geo/ias_geo_transform_j2k2tod.o \
./ias_lib/misc/geo/ias_geo_transform_nutation.o \
./ias_lib/misc/geo/ias_geo_transform_polar_motion.o \
./ias_lib/misc/geo/ias_geo_transform_precession.o \
./ias_lib/misc/geo/ias_geo_transform_sidereal.o \
./ias_lib/misc/geo/ias_geo_transform_tod2j2k.o 

C_DEPS += \
./ias_lib/misc/geo/ias_geo_analyze_gcp_records.d \
./ias_lib/misc/geo/ias_geo_check_start_end_date.d \
./ias_lib/misc/geo/ias_geo_compute_earth2orbit_transform.d \
./ias_lib/misc/geo/ias_geo_compute_forward_mappings.d \
./ias_lib/misc/geo/ias_geo_compute_getmjdcoords.d \
./ias_lib/misc/geo/ias_geo_compute_inverse_mappings.d \
./ias_lib/misc/geo/ias_geo_compute_map_edge.d \
./ias_lib/misc/geo/ias_geo_compute_orientation_matrices.d \
./ias_lib/misc/geo/ias_geo_compute_potential.d \
./ias_lib/misc/geo/ias_geo_compute_proj2ls_poly.d \
./ias_lib/misc/geo/ias_geo_compute_proj2proj_poly.d \
./ias_lib/misc/geo/ias_geo_convert_cart2sph.d \
./ias_lib/misc/geo/ias_geo_convert_deg2dms.d \
./ias_lib/misc/geo/ias_geo_convert_dms2deg.d \
./ias_lib/misc/geo/ias_geo_convert_geocentric_height_to_geodetic.d \
./ias_lib/misc/geo/ias_geo_convert_geod2cart.d \
./ias_lib/misc/geo/ias_geo_convert_rpy_to_matrix.d \
./ias_lib/misc/geo/ias_geo_convert_sensor_los_to_spacecraft.d \
./ias_lib/misc/geo/ias_geo_convert_sph2cart.d \
./ias_lib/misc/geo/ias_geo_convert_utc2times.d \
./ias_lib/misc/geo/ias_geo_correct_for_center_of_mass.d \
./ias_lib/misc/geo/ias_geo_correct_for_light_travel_time.d \
./ias_lib/misc/geo/ias_geo_correct_for_velocity_aberration.d \
./ias_lib/misc/geo/ias_geo_create_transformation_matrix.d \
./ias_lib/misc/geo/ias_geo_ecef2eci.d \
./ias_lib/misc/geo/ias_geo_eci2ecef.d \
./ias_lib/misc/geo/ias_geo_extract_window.d \
./ias_lib/misc/geo/ias_geo_find_ariesha.d \
./ias_lib/misc/geo/ias_geo_find_deg.d \
./ias_lib/misc/geo/ias_geo_find_earth_radius.d \
./ias_lib/misc/geo/ias_geo_find_min.d \
./ias_lib/misc/geo/ias_geo_find_mjdcoords.d \
./ias_lib/misc/geo/ias_geo_find_sec.d \
./ias_lib/misc/geo/ias_geo_find_target_position.d \
./ias_lib/misc/geo/ias_geo_get_units.d \
./ias_lib/misc/geo/ias_geo_handle_180.d \
./ias_lib/misc/geo/ias_geo_lagrange_interpolate.d \
./ias_lib/misc/geo/ias_geo_novas_wrapper.d \
./ias_lib/misc/geo/ias_geo_projection_transformation.d \
./ias_lib/misc/geo/ias_geo_report_proj_err.d \
./ias_lib/misc/geo/ias_geo_transform_ecef2j2k.d \
./ias_lib/misc/geo/ias_geo_transform_j2k2tod.d \
./ias_lib/misc/geo/ias_geo_transform_nutation.d \
./ias_lib/misc/geo/ias_geo_transform_polar_motion.d \
./ias_lib/misc/geo/ias_geo_transform_precession.d \
./ias_lib/misc/geo/ias_geo_transform_sidereal.d \
./ias_lib/misc/geo/ias_geo_transform_tod2j2k.d 


# Each subdirectory must supply rules for building sources it contributes
ias_lib/misc/geo/%.o: ../ias_lib/misc/geo/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I/DATA/DPAS5_COTS/odl -I/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc -I/DATA/DPAS5_COTS/hdf5/include -I/DATA/DPAS5_COTS/novas3.1/include -I/DATA/DPAS5_COTS/gctp3/include -I/DATA/DPAS5_COTS/remez/include -I/DATA/DPAS5_COTS/gsl/include/gsl -I/DATA/DPAS5_COTS/tiff/include -I/usr/local/include -I/DATA/DPAS5_COTS/unixodbc/include -I"/home/cqw/workplace_1/Get_Geodetic/MQ" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/ancillary" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/ancillary_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_database" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/cpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/gcp" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/geometric_grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L0R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1G" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/parameter_file_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/rlut" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/sensor" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/spacecraft" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/database_access" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/geo" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/math" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/miscellaneous" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/odl" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/pixel_mask" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/satellite_attributes" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/threading" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/perllib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/setup" -I"/home/cqw/workplace_1/Get_Geodetic" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


