# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libodl.la

# define the source files included in the library
libodl_la_SOURCES = \
        ias_odl_add_field.c \
        ias_odl_replace_field.c \
        ias_odl_add_or_replace_field.c \
        ias_odl_add_field_list.c \
        ias_odl_add_group.c \
        ias_odl_create_tree.c \
        ias_odl_find_keyword.c \
        ias_odl_find_object_description.c \
        ias_odl_free_tree.c \
        ias_odl_get_field.c \
        ias_odl_get_field_list.c \
        ias_odl_get_group.c \
        ias_odl_get_group_names.c \
        ias_odl_parse_file.c \
        ias_odl_parse_label_string.c \
        ias_odl_read_tree.c \
        ias_odl_write_tree.c \
        ias_odl_remove_character.c

# headers to install
include_HEADERS = ias_odl.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @ODL_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

