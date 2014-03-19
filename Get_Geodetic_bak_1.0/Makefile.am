#subdirectories to include
SUBDIRS = . tests

bin_PROGRAMS = correct_los_model

INCLUDES = @IAS_INCLUDES@ @GCTP_INCLUDES@

LDADD = @IAS_LIB@ @IAS_DB_LIBS@ @GCTP_LIBS@ @ODL_LIBS@ @HDF5_LIBS@ \
    @NOVAS_LIBS@

correct_los_model_SOURCES = \
    accum_normal_equation.c \
    add_position.c \
    calc_alignment.c \
    calc_correction.c \
    calc_line_of_sight.c \
    calc_partial_coeffs_with_attitude_and_ephemeris.c \
    calc_partial_coeffs_with_attitude.c \
    calc_partial_coeffs_with_ephemeris.c \
    compute_max_likelihood_estimate.c \
    compute_min_quadratic_estimate.c \
    compute_residual_square_sum.c \
    correct_los_model.c \
    finish_processing.c \
    determine_worst_outlier.c \
    determine_precision_success.c \
    get_correction.c \
    get_gcp_characterization_data.c \
    get_ground_residuals.c \
    get_position_and_reference_time.c \
    get_proj_info.c \
    get_lat_long.c \
    initialize_gcp_struct.c \
    initialize_precision.c \
    observation_residual.c \
    process_one_gcp.c \
    put_solution.c \
    read_parameters.c \
    reset_observe_angle.c \
    setup_partial_coefficients.c \
    solve_weighted_least_square.c \
    store_data_in_db.c \
    update_ephemeris.c \
    update_look_angle.c \
    update_observe_angle.c \
    update_omf.c \
    write_residuals.c
