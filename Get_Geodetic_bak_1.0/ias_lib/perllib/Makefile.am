EXTRA_LIBS = Logger.pm ias_const.pm ias_wo_const.pm ias_db.pm \
             ias_processing_system.pm ias_disk_utils.pm ias_date_utils.pm \
             ias_MessageIds.pm ias_create_work_order.pm \
             ias_StartProcess.pm ias_cpf_services.pm ias_time.pm \
             ias_system_call.pm ias_wo_utilities.pm ias_cpf_const.pm \
             ias_task.pm ias_odl.pm ias_math_utilities.pm ias_mail_call.pm \
             ias_put_timing_event.pm ias_UpdateWOState.pm

install-exec-hook:
	mkdir -p @IASLIB_PERLLIB@
	cp ${EXTRA_LIBS} @IASLIB_PERLLIB@

uninstall-hook:
	for f in $(EXTRA_LIBS); do \
            rm -f @IASLIB_PERLLIB@/$$f; \
        done

# Auto generate the iasQtWoConst.h file from the perl master file.
iasQtWoConst.h: ias_wo_const.pm
	./create_wo_const_header_file.sh ias_wo_const.pm iasQtWoConst.h

# List of public headers that need to be installed.  Private header files
# should not be included in this list.
include_HEADERS = iasQtWoConst.h

headers: install-includeHEADERS

clean:
	rm -f iasQtWoConst.h

