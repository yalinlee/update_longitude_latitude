# Makefile to install setup files in $IAS_SYS_DIR
setupfile = iaslib_setup

# Set permissions on the installed files
install-data-local: $(setupfile)
	@if [ "x$(IAS_SYS_DIR)" != "x" ] ; then \
        echo mkdir -p $(IAS_SYS_DIR)/setup ; \
        mkdir -p $(IAS_SYS_DIR)/setup ; \
        echo cp $(setupfile) $(IAS_SYS_DIR)/setup ;\
        cp $(setupfile) $(IAS_SYS_DIR)/setup ;\
        echo chmod 640 $(IAS_SYS_DIR)/setup/$(setupfile) ;\
        chmod 640 $(IAS_SYS_DIR)/setup/$(setupfile) ;\
    fi

# target required in ias_lib directories
headers:

