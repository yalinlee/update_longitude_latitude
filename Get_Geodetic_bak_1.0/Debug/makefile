################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include ias_lib/misc/threading/subdir.mk
-include ias_lib/misc/satellite_attributes/subdir.mk
-include ias_lib/misc/pixel_mask/subdir.mk
-include ias_lib/misc/odl/subdir.mk
-include ias_lib/misc/miscellaneous/subdir.mk
-include ias_lib/misc/math/subdir.mk
-include ias_lib/misc/geo/subdir.mk
-include ias_lib/misc/database_access/subdir.mk
-include ias_lib/misc/subdir.mk
-include ias_lib/los_model/spacecraft/subdir.mk
-include ias_lib/los_model/sensor/subdir.mk
-include ias_lib/los_model/subdir.mk
-include ias_lib/io/rlut/subdir.mk
-include ias_lib/io/parameter_file_io/subdir.mk
-include ias_lib/io/model/subdir.mk
-include ias_lib/io/grid/subdir.mk
-include ias_lib/io/geometric_grid/subdir.mk
-include ias_lib/io/gcp/subdir.mk
-include ias_lib/io/cpf_file/subdir.mk
-include ias_lib/io/bpf_file/subdir.mk
-include ias_lib/io/bpf_database/subdir.mk
-include ias_lib/io/ancillary_io/subdir.mk
-include ias_lib/io/L1R/subdir.mk
-include ias_lib/io/L1G/subdir.mk
-include ias_lib/io/L0R/subdir.mk
-include ias_lib/grid/subdir.mk
-include ias_lib/ancillary/subdir.mk
-include MQ/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Get_Geodetic

# Tool invocations
Get_Geodetic: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L/DATA/DPAS5_COTS/odl -L/DATA/DPAS5_COTS/unixodbc/lib -L/DATA/DPAS5_COTS/gctp3/lib -L/DATA/DPAS5_COTS/remez/lib -L/DATA/DPAS5_COTS/hdf5/lib -L/DATA/DPAS5_COTS/gsl/lib -L/DATA/DPAS5_COTS/novas3.1/lib -L/DATA/DPAS5_COTS/tiff/lib -o "Get_Geodetic" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Get_Geodetic
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
