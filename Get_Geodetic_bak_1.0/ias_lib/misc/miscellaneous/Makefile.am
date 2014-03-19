# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libmiscellaneous.la

# define the source files included in the library
libmiscellaneous_la_SOURCES = \
    ias_misc_2d_array.c \
    ias_misc_check_file_type.c \
    ias_misc_check_year_month_day.c \
    ias_misc_copy_report_header.c \
    ias_misc_convert_data_type.c \
    ias_misc_convert_to_lowercase.c \
    ias_misc_convert_to_uppercase.c \
    ias_misc_free_report_header.c \
    ias_misc_get_char_id.c \
    ias_misc_get_header_info.c \
    ias_misc_get_processing_system.c \
    ias_misc_get_sizeof_data_type.c \
    ias_misc_get_software_version.c \
    ias_misc_get_timestamp.c \
    ias_misc_initialize_gps_report_header.c \
    ias_misc_initialize_report_header.c \
    ias_misc_initialize_rps_report_header.c \
    ias_misc_parse_datetime_string.c \
    ias_misc_read_dem.c \
    ias_misc_read_elevation_at_line_sample.c \
    ias_misc_read_gcp_residuals.c \
    ias_misc_read_single_band_l1g.c \
    ias_misc_set_header_info.c \
    ias_misc_split_filename.c \
    ias_misc_write_envi_header.c \
    ias_misc_write_gcp_residuals.c \
    ias_misc_write_gps_report_header.c \
    ias_misc_write_report_header.c \
    ias_misc_write_rps_report_header.c

# headers to install
include_HEADERS = \
    ias_miscellaneous.h \
    ias_misc_gcp_residuals.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @HDF5_INCLUDES@ @GCTP_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

