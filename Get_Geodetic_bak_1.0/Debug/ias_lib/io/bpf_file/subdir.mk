################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ias_lib/io/bpf_file/ias_bpf_cache.c \
../ias_lib/io/bpf_file/ias_bpf_get_model_parameters.c \
../ias_lib/io/bpf_file/ias_bpf_mem.c \
../ias_lib/io/bpf_file/ias_bpf_parse_bias_model.c \
../ias_lib/io/bpf_file/ias_bpf_parse_file_attributes.c \
../ias_lib/io/bpf_file/ias_bpf_parse_orbit_parameters.c \
../ias_lib/io/bpf_file/ias_bpf_read.c \
../ias_lib/io/bpf_file/ias_bpf_set_groups.c \
../ias_lib/io/bpf_file/ias_bpf_write.c 

OBJS += \
./ias_lib/io/bpf_file/ias_bpf_cache.o \
./ias_lib/io/bpf_file/ias_bpf_get_model_parameters.o \
./ias_lib/io/bpf_file/ias_bpf_mem.o \
./ias_lib/io/bpf_file/ias_bpf_parse_bias_model.o \
./ias_lib/io/bpf_file/ias_bpf_parse_file_attributes.o \
./ias_lib/io/bpf_file/ias_bpf_parse_orbit_parameters.o \
./ias_lib/io/bpf_file/ias_bpf_read.o \
./ias_lib/io/bpf_file/ias_bpf_set_groups.o \
./ias_lib/io/bpf_file/ias_bpf_write.o 

C_DEPS += \
./ias_lib/io/bpf_file/ias_bpf_cache.d \
./ias_lib/io/bpf_file/ias_bpf_get_model_parameters.d \
./ias_lib/io/bpf_file/ias_bpf_mem.d \
./ias_lib/io/bpf_file/ias_bpf_parse_bias_model.d \
./ias_lib/io/bpf_file/ias_bpf_parse_file_attributes.d \
./ias_lib/io/bpf_file/ias_bpf_parse_orbit_parameters.d \
./ias_lib/io/bpf_file/ias_bpf_read.d \
./ias_lib/io/bpf_file/ias_bpf_set_groups.d \
./ias_lib/io/bpf_file/ias_bpf_write.d 


# Each subdirectory must supply rules for building sources it contributes
ias_lib/io/bpf_file/%.o: ../ias_lib/io/bpf_file/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I/DATA/DPAS5_COTS/odl -I/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc -I/DATA/DPAS5_COTS/hdf5/include -I/DATA/DPAS5_COTS/novas3.1/include -I/DATA/DPAS5_COTS/gctp3/include -I/DATA/DPAS5_COTS/remez/include -I/DATA/DPAS5_COTS/gsl/include/gsl -I/DATA/DPAS5_COTS/tiff/include -I/usr/local/include -I/DATA/DPAS5_COTS/unixodbc/include -I"/home/cqw/workplace_1/Get_Geodetic/MQ" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/ancillary" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/ancillary_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_database" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/cpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/gcp" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/geometric_grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L0R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1G" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/parameter_file_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/rlut" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/sensor" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/spacecraft" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/database_access" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/geo" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/math" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/miscellaneous" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/odl" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/pixel_mask" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/satellite_attributes" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/threading" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/perllib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/setup" -I"/home/cqw/workplace_1/Get_Geodetic" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


