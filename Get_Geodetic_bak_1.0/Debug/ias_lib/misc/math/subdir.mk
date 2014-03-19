################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ias_lib/misc/math/ias_math_add_seconds_to_year_doy_sod.c \
../ias_lib/misc/math/ias_math_check_pixels_in_range.c \
../ias_lib/misc/math/ias_math_compute_3dvec_cross.c \
../ias_lib/misc/math/ias_math_compute_3dvec_dot.c \
../ias_lib/misc/math/ias_math_compute_full_julian_date.c \
../ias_lib/misc/math/ias_math_compute_grey_cross.c \
../ias_lib/misc/math/ias_math_compute_grey_cross_same_size.c \
../ias_lib/misc/math/ias_math_compute_mean.c \
../ias_lib/misc/math/ias_math_compute_rmse.c \
../ias_lib/misc/math/ias_math_compute_stdev.c \
../ias_lib/misc/math/ias_math_compute_t_confidence.c \
../ias_lib/misc/math/ias_math_compute_unit_vector.c \
../ias_lib/misc/math/ias_math_compute_vector_length.c \
../ias_lib/misc/math/ias_math_conjugate_quaternion.c \
../ias_lib/misc/math/ias_math_constants.c \
../ias_lib/misc/math/ias_math_convert_doy_to_month_day.c \
../ias_lib/misc/math/ias_math_convert_euler_to_quaternion.c \
../ias_lib/misc/math/ias_math_convert_j2000_time.c \
../ias_lib/misc/math/ias_math_convert_month_day_to_doy.c \
../ias_lib/misc/math/ias_math_convert_quaternion_to_euler.c \
../ias_lib/misc/math/ias_math_correlate_fine.c \
../ias_lib/misc/math/ias_math_correlate_grey.c \
../ias_lib/misc/math/ias_math_cubic_convolution.c \
../ias_lib/misc/math/ias_math_eval_legendre.c \
../ias_lib/misc/math/ias_math_eval_poly.c \
../ias_lib/misc/math/ias_math_eval_poly_xy.c \
../ias_lib/misc/math/ias_math_evaluate_grey.c \
../ias_lib/misc/math/ias_math_find_line_segment_intersection.c \
../ias_lib/misc/math/ias_math_find_median_unsigned.c \
../ias_lib/misc/math/ias_math_fit_registration.c \
../ias_lib/misc/math/ias_math_get_time_difference.c \
../ias_lib/misc/math/ias_math_heapsort_double_array.c \
../ias_lib/misc/math/ias_math_insertion_sort_integer_array.c \
../ias_lib/misc/math/ias_math_interpolate_lagrange.c \
../ias_lib/misc/math/ias_math_interpolate_lagrange_3dvec.c \
../ias_lib/misc/math/ias_math_invert_3x3_matrix.c \
../ias_lib/misc/math/ias_math_is_leap_year.c \
../ias_lib/misc/math/ias_math_kalman.c \
../ias_lib/misc/math/ias_math_matrix.c \
../ias_lib/misc/math/ias_math_matrix_QRfactorization.c \
../ias_lib/misc/math/ias_math_matrix_QRsolve.c \
../ias_lib/misc/math/ias_math_multiply_3x3_matrix.c \
../ias_lib/misc/math/ias_math_normalize_grey_cross.c \
../ias_lib/misc/math/ias_math_normalize_grey_cross_same_size.c \
../ias_lib/misc/math/ias_math_parallel_correlator.c \
../ias_lib/misc/math/ias_math_quaternion.c \
../ias_lib/misc/math/ias_math_rotate_around_axis.c \
../ias_lib/misc/math/ias_math_smooth.c \
../ias_lib/misc/math/ias_math_solve_linear_equation.c \
../ias_lib/misc/math/ias_math_transform_3dvec.c \
../ias_lib/misc/math/ias_math_transpose_3x3_matrix.c 

OBJS += \
./ias_lib/misc/math/ias_math_add_seconds_to_year_doy_sod.o \
./ias_lib/misc/math/ias_math_check_pixels_in_range.o \
./ias_lib/misc/math/ias_math_compute_3dvec_cross.o \
./ias_lib/misc/math/ias_math_compute_3dvec_dot.o \
./ias_lib/misc/math/ias_math_compute_full_julian_date.o \
./ias_lib/misc/math/ias_math_compute_grey_cross.o \
./ias_lib/misc/math/ias_math_compute_grey_cross_same_size.o \
./ias_lib/misc/math/ias_math_compute_mean.o \
./ias_lib/misc/math/ias_math_compute_rmse.o \
./ias_lib/misc/math/ias_math_compute_stdev.o \
./ias_lib/misc/math/ias_math_compute_t_confidence.o \
./ias_lib/misc/math/ias_math_compute_unit_vector.o \
./ias_lib/misc/math/ias_math_compute_vector_length.o \
./ias_lib/misc/math/ias_math_conjugate_quaternion.o \
./ias_lib/misc/math/ias_math_constants.o \
./ias_lib/misc/math/ias_math_convert_doy_to_month_day.o \
./ias_lib/misc/math/ias_math_convert_euler_to_quaternion.o \
./ias_lib/misc/math/ias_math_convert_j2000_time.o \
./ias_lib/misc/math/ias_math_convert_month_day_to_doy.o \
./ias_lib/misc/math/ias_math_convert_quaternion_to_euler.o \
./ias_lib/misc/math/ias_math_correlate_fine.o \
./ias_lib/misc/math/ias_math_correlate_grey.o \
./ias_lib/misc/math/ias_math_cubic_convolution.o \
./ias_lib/misc/math/ias_math_eval_legendre.o \
./ias_lib/misc/math/ias_math_eval_poly.o \
./ias_lib/misc/math/ias_math_eval_poly_xy.o \
./ias_lib/misc/math/ias_math_evaluate_grey.o \
./ias_lib/misc/math/ias_math_find_line_segment_intersection.o \
./ias_lib/misc/math/ias_math_find_median_unsigned.o \
./ias_lib/misc/math/ias_math_fit_registration.o \
./ias_lib/misc/math/ias_math_get_time_difference.o \
./ias_lib/misc/math/ias_math_heapsort_double_array.o \
./ias_lib/misc/math/ias_math_insertion_sort_integer_array.o \
./ias_lib/misc/math/ias_math_interpolate_lagrange.o \
./ias_lib/misc/math/ias_math_interpolate_lagrange_3dvec.o \
./ias_lib/misc/math/ias_math_invert_3x3_matrix.o \
./ias_lib/misc/math/ias_math_is_leap_year.o \
./ias_lib/misc/math/ias_math_kalman.o \
./ias_lib/misc/math/ias_math_matrix.o \
./ias_lib/misc/math/ias_math_matrix_QRfactorization.o \
./ias_lib/misc/math/ias_math_matrix_QRsolve.o \
./ias_lib/misc/math/ias_math_multiply_3x3_matrix.o \
./ias_lib/misc/math/ias_math_normalize_grey_cross.o \
./ias_lib/misc/math/ias_math_normalize_grey_cross_same_size.o \
./ias_lib/misc/math/ias_math_parallel_correlator.o \
./ias_lib/misc/math/ias_math_quaternion.o \
./ias_lib/misc/math/ias_math_rotate_around_axis.o \
./ias_lib/misc/math/ias_math_smooth.o \
./ias_lib/misc/math/ias_math_solve_linear_equation.o \
./ias_lib/misc/math/ias_math_transform_3dvec.o \
./ias_lib/misc/math/ias_math_transpose_3x3_matrix.o 

C_DEPS += \
./ias_lib/misc/math/ias_math_add_seconds_to_year_doy_sod.d \
./ias_lib/misc/math/ias_math_check_pixels_in_range.d \
./ias_lib/misc/math/ias_math_compute_3dvec_cross.d \
./ias_lib/misc/math/ias_math_compute_3dvec_dot.d \
./ias_lib/misc/math/ias_math_compute_full_julian_date.d \
./ias_lib/misc/math/ias_math_compute_grey_cross.d \
./ias_lib/misc/math/ias_math_compute_grey_cross_same_size.d \
./ias_lib/misc/math/ias_math_compute_mean.d \
./ias_lib/misc/math/ias_math_compute_rmse.d \
./ias_lib/misc/math/ias_math_compute_stdev.d \
./ias_lib/misc/math/ias_math_compute_t_confidence.d \
./ias_lib/misc/math/ias_math_compute_unit_vector.d \
./ias_lib/misc/math/ias_math_compute_vector_length.d \
./ias_lib/misc/math/ias_math_conjugate_quaternion.d \
./ias_lib/misc/math/ias_math_constants.d \
./ias_lib/misc/math/ias_math_convert_doy_to_month_day.d \
./ias_lib/misc/math/ias_math_convert_euler_to_quaternion.d \
./ias_lib/misc/math/ias_math_convert_j2000_time.d \
./ias_lib/misc/math/ias_math_convert_month_day_to_doy.d \
./ias_lib/misc/math/ias_math_convert_quaternion_to_euler.d \
./ias_lib/misc/math/ias_math_correlate_fine.d \
./ias_lib/misc/math/ias_math_correlate_grey.d \
./ias_lib/misc/math/ias_math_cubic_convolution.d \
./ias_lib/misc/math/ias_math_eval_legendre.d \
./ias_lib/misc/math/ias_math_eval_poly.d \
./ias_lib/misc/math/ias_math_eval_poly_xy.d \
./ias_lib/misc/math/ias_math_evaluate_grey.d \
./ias_lib/misc/math/ias_math_find_line_segment_intersection.d \
./ias_lib/misc/math/ias_math_find_median_unsigned.d \
./ias_lib/misc/math/ias_math_fit_registration.d \
./ias_lib/misc/math/ias_math_get_time_difference.d \
./ias_lib/misc/math/ias_math_heapsort_double_array.d \
./ias_lib/misc/math/ias_math_insertion_sort_integer_array.d \
./ias_lib/misc/math/ias_math_interpolate_lagrange.d \
./ias_lib/misc/math/ias_math_interpolate_lagrange_3dvec.d \
./ias_lib/misc/math/ias_math_invert_3x3_matrix.d \
./ias_lib/misc/math/ias_math_is_leap_year.d \
./ias_lib/misc/math/ias_math_kalman.d \
./ias_lib/misc/math/ias_math_matrix.d \
./ias_lib/misc/math/ias_math_matrix_QRfactorization.d \
./ias_lib/misc/math/ias_math_matrix_QRsolve.d \
./ias_lib/misc/math/ias_math_multiply_3x3_matrix.d \
./ias_lib/misc/math/ias_math_normalize_grey_cross.d \
./ias_lib/misc/math/ias_math_normalize_grey_cross_same_size.d \
./ias_lib/misc/math/ias_math_parallel_correlator.d \
./ias_lib/misc/math/ias_math_quaternion.d \
./ias_lib/misc/math/ias_math_rotate_around_axis.d \
./ias_lib/misc/math/ias_math_smooth.d \
./ias_lib/misc/math/ias_math_solve_linear_equation.d \
./ias_lib/misc/math/ias_math_transform_3dvec.d \
./ias_lib/misc/math/ias_math_transpose_3x3_matrix.d 


# Each subdirectory must supply rules for building sources it contributes
ias_lib/misc/math/%.o: ../ias_lib/misc/math/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I/DATA/DPAS5_COTS/odl -I/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc -I/DATA/DPAS5_COTS/hdf5/include -I/DATA/DPAS5_COTS/novas3.1/include -I/DATA/DPAS5_COTS/gctp3/include -I/DATA/DPAS5_COTS/remez/include -I/DATA/DPAS5_COTS/gsl/include/gsl -I/DATA/DPAS5_COTS/tiff/include -I/usr/local/include -I/DATA/DPAS5_COTS/unixodbc/include -I"/home/cqw/workplace_1/Get_Geodetic/MQ" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/ancillary" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/ancillary_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_database" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/cpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/gcp" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/geometric_grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L0R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1G" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/parameter_file_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/rlut" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/sensor" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/spacecraft" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/database_access" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/geo" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/math" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/miscellaneous" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/odl" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/pixel_mask" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/satellite_attributes" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/threading" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/perllib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/setup" -I"/home/cqw/workplace_1/Get_Geodetic" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


