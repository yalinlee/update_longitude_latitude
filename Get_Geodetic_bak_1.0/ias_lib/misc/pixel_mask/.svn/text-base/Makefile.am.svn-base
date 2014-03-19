# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libpixel_mask.la

# define the source files included in the library
libpixel_mask_la_SOURCES = \
    ias_pm_create.c \
    ias_pm_add_pixels.c \
    ias_pm_destroy.c \
    ias_pm_get_mask_at.c \
    ias_pm_get_mask_index.c \
    ias_pm_get_iterator.c \
    ias_pm_destroy_iterator.c \
    ias_pm_get_next_span.c \
    ias_pm_get_image.c \
    ias_pm_get_image_subset.c \
    ias_pm_open_pixel_mask.c \
    ias_pm_close_pixel_mask.c \
    ias_pm_write_mask_data_to_file.c \
    ias_pm_write_array_to_file.c \
    ias_pm_write_single_mask_to_file.c \
    ias_pm_read_array_from_file.c \
    ias_pm_read_single_mask_from_file.c \
    ias_pm_read_mask_data_from_file.c \
    ias_pm_is_detector_flagged.c \
    ias_pm_get_sca_number.c \
    ias_pm_get_band_number.c \
    ias_pm_get_number_of_detectors.c \
    ias_pm_get_number_of_pixels.c \
    ias_pm_get_file_band_sca_list.c

# headers to install
include_HEADERS = ias_pixel_mask.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

