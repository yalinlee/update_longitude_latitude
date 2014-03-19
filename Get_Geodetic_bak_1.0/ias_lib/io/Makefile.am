# subdirectories to include
SUBDIRS = ancillary_io gcp geometric_grid grid L0R L1R L1G model \
          parameter_file_io cpf_file bpf_database bpf_file \
          rlut

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.
noinst_LTLIBRARIES = libio.la

# include a dummy sources line to silence a complaint
libio_la_SOURCES =

# Each of the intermediate libraries need to be listed here to include them in
# the final library
libio_la_LIBADD = \
    ancillary_io/libancillary_io.la \
    bpf_database/libbpf_database.la \
    bpf_file/libbpf_file.la \
    gcp/libgcp.la \
    geometric_grid/libgeometric_grid.la \
    grid/libgrid.la \
    L0R/libL0R.la \
    L1R/libL1R.la \
    L1G/libL1G.la \
    model/libmodelio.la \
    parameter_file_io/libparameter_file_io.la \
    cpf_file/libcpf_file_io.la \
    rlut/librlut_io.la


# The following recurses into each of the subdirectories to install the public
# header files before the main build is done
headers:
	for d in $(SUBDIRS); do \
        cd $$d && make headers && cd ..; \
	done
