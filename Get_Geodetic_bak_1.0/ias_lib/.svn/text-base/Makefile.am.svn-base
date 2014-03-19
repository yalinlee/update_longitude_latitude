# subdirectories to include
SUBDIRS = ancillary grid io los_model misc perllib rps setup @IAS_QT_LIB_DIR@

# libtool is used to allow combining the different directories into
# intermediate libraries before creating the final IAS library.  Automake does
# appear to be able to do that without using libtool.
lib_LTLIBRARIES = libias.la

# we only want a static library
libias_la_LDFLAGS = -static

# include a dummy sources line to silence a complaint
libias_la_SOURCES = 

# Each of the intermediate libraries need to be listed here to include them in
# the final library
libias_la_LIBADD = \
    ancillary/libancillary.la \
    grid/libgrid.la \
    io/libio.la \
    los_model/liblos_model.la \
    misc/libmisc.la \
    rps/librps.la

# The following recurses into each of the subdirectories to install the public
# header files before the main build is done
headers:
	for d in $(SUBDIRS); do \
        cd $$d && make headers && cd ..; \
	done

# make a target to run a make check and then verify no errors are found
tests:
	make check >& check.log && grep failure check.log

