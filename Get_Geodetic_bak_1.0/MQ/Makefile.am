
SUFFIXES = .pc

lib_LIBRARIES = libMQ.a

AM_CPPFLAGS = @ODL_INCLUDES@ @HDF_INCLUDES@ \
           @GPS_INCLUDES@

libMQ_a_CFLAGS = -fPIC

libMQ_a_SOURCES = \
		amqp_api.c\
		amqp_connection.c\
		amqp_framing.c\
		amqp_mem.c\
		amqp_sendstring.c\
		amqp_socket.c\
		amqp_table.c\
		utils.c\
		unix/platform_utils.c\
		unix/socket.c
				
.pc.c:
	cp $< $*.c
	chmod 644 $*.c
	$(CPP) $(CFLAGS) -P @PROC_CPPFLAGS@ $(INCLUDES) $*.c -o $*.i
	$(ORACLE_HOME)/bin/proc iname=$*.i @PROFLAGS@
	rm $*.i

EXTRA_DIST = *.pc
