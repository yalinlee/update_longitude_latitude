# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libiasmath.la

# define the source files included in the library
libiasmath_la_SOURCES = \
    ias_math_add_seconds_to_year_doy_sod.c \
    ias_math_check_pixels_in_range.c \
    ias_math_compute_3dvec_cross.c \
    ias_math_compute_3dvec_dot.c \
    ias_math_compute_full_julian_date.c \
    ias_math_compute_grey_cross.c \
    ias_math_compute_grey_cross_same_size.c \
    ias_math_compute_rmse.c \
    ias_math_compute_stdev.c \
    ias_math_compute_unit_vector.c \
    ias_math_constants.c \
    ias_math_convert_doy_to_month_day.c \
    ias_math_convert_j2000_time.c \
    ias_math_compute_mean.c \
    ias_math_compute_t_confidence.c \
    ias_math_compute_vector_length.c \
    ias_math_quaternion.c \
    ias_math_conjugate_quaternion.c \
    ias_math_convert_month_day_to_doy.c \
    ias_math_correlate_fine.c \
    ias_math_correlate_grey.c \
    ias_math_cubic_convolution.c \
    ias_math_evaluate_grey.c \
    ias_math_eval_legendre.c \
    ias_math_eval_poly.c \
    ias_math_eval_poly_xy.c \
    ias_math_find_line_segment_intersection.c \
    ias_math_find_median_unsigned.c \
    ias_math_fit_registration.c \
    ias_math_get_time_difference.c \
    ias_math_heapsort_double_array.c \
    ias_math_insertion_sort_integer_array.c \
    ias_math_kalman.c \
    ias_math_interpolate_lagrange.c \
    ias_math_interpolate_lagrange_3dvec.c \
    ias_math_invert_3x3_matrix.c \
    ias_math_is_leap_year.c \
    ias_math_matrix.c \
    ias_math_matrix_QRfactorization.c \
    ias_math_matrix_QRsolve.c \
    ias_math_multiply_3x3_matrix.c \
    ias_math_normalize_grey_cross.c \
    ias_math_normalize_grey_cross_same_size.c \
    ias_math_parallel_correlator.c \
    ias_math_rotate_around_axis.c \
    ias_math_smooth.c \
    ias_math_solve_linear_equation.c \
    ias_math_transform_3dvec.c \
    ias_math_transpose_3x3_matrix.c \
    ias_math_convert_quaternion_to_euler.c \
    ias_math_convert_euler_to_quaternion.c


# headers to install
include_HEADERS = ias_math.h ias_math_parallel_corr.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

