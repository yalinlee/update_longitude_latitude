################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ias_lib/io/rlut/ias_rlut2.c \
../ias_lib/io/rlut/ias_rlut_build_file_attributes_table_description.c \
../ias_lib/io/rlut/ias_rlut_build_linearization_params_table_description.c \
../ias_lib/io/rlut/ias_rlut_open_close_file.c \
../ias_lib/io/rlut/ias_rlut_read_file_attributes.c \
../ias_lib/io/rlut/ias_rlut_read_linearization_params.c \
../ias_lib/io/rlut/ias_rlut_read_tirs_secondary_linearization_params.c \
../ias_lib/io/rlut/ias_rlut_write_file_attributes.c \
../ias_lib/io/rlut/ias_rlut_write_linearization_params.c \
../ias_lib/io/rlut/ias_rlut_write_tirs_secondary_linearization_params.c 

OBJS += \
./ias_lib/io/rlut/ias_rlut2.o \
./ias_lib/io/rlut/ias_rlut_build_file_attributes_table_description.o \
./ias_lib/io/rlut/ias_rlut_build_linearization_params_table_description.o \
./ias_lib/io/rlut/ias_rlut_open_close_file.o \
./ias_lib/io/rlut/ias_rlut_read_file_attributes.o \
./ias_lib/io/rlut/ias_rlut_read_linearization_params.o \
./ias_lib/io/rlut/ias_rlut_read_tirs_secondary_linearization_params.o \
./ias_lib/io/rlut/ias_rlut_write_file_attributes.o \
./ias_lib/io/rlut/ias_rlut_write_linearization_params.o \
./ias_lib/io/rlut/ias_rlut_write_tirs_secondary_linearization_params.o 

C_DEPS += \
./ias_lib/io/rlut/ias_rlut2.d \
./ias_lib/io/rlut/ias_rlut_build_file_attributes_table_description.d \
./ias_lib/io/rlut/ias_rlut_build_linearization_params_table_description.d \
./ias_lib/io/rlut/ias_rlut_open_close_file.d \
./ias_lib/io/rlut/ias_rlut_read_file_attributes.d \
./ias_lib/io/rlut/ias_rlut_read_linearization_params.d \
./ias_lib/io/rlut/ias_rlut_read_tirs_secondary_linearization_params.d \
./ias_lib/io/rlut/ias_rlut_write_file_attributes.d \
./ias_lib/io/rlut/ias_rlut_write_linearization_params.d \
./ias_lib/io/rlut/ias_rlut_write_tirs_secondary_linearization_params.d 


# Each subdirectory must supply rules for building sources it contributes
ias_lib/io/rlut/%.o: ../ias_lib/io/rlut/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I/DATA/DPAS5_COTS/odl -I/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc -I/DATA/DPAS5_COTS/hdf5/include -I/DATA/DPAS5_COTS/novas3.1/include -I/DATA/DPAS5_COTS/gctp3/include -I/DATA/DPAS5_COTS/remez/include -I/DATA/DPAS5_COTS/gsl/include/gsl -I/DATA/DPAS5_COTS/tiff/include -I/usr/local/include -I/DATA/DPAS5_COTS/unixodbc/include -I"/home/cqw/workplace_1/Get_Geodetic/MQ" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/ancillary" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/ancillary_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_database" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/cpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/gcp" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/geometric_grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L0R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1G" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/parameter_file_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/rlut" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/sensor" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/spacecraft" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/database_access" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/geo" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/math" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/miscellaneous" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/odl" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/pixel_mask" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/satellite_attributes" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/threading" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/perllib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/setup" -I"/home/cqw/workplace_1/Get_Geodetic" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


