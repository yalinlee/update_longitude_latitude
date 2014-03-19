# subdirectories to include
SUBDIRS = . tests

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libdatabase_access.la

# define the source files included in the library
libdatabase_access_la_SOURCES = \
    ias_db.c \
    ias_db_get_connect_info.c \
    ias_db_table.c \
    ias_db_insert_transaction_using_table.c \
    ias_db_query.c \
    ias_db_delete_records.c \
    ias_db_get_count.c

# headers to install
include_HEADERS = ias_db.h  \
      	ias_db_insert_transaction_using_table.h \
      	ias_db_query.h \
	ias_db_get_connect_info.h \
	ias_db_insert.h \
	ias_db_stored_proc.h \
	ias_db_table.h

# include headers from the IAS include directory
INCLUDES = @IAS_INCLUDES@ @IAS_DB_INCLUDES@

# redirect the headers target to the standard header install target created
# by automake
headers: install-includeHEADERS

