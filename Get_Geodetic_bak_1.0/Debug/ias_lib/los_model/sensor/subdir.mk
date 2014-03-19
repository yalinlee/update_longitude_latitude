################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ias_lib/los_model/sensor/ias_sensor_adjust_los_for_ssm.c \
../ias_lib/los_model/sensor/ias_sensor_align_ssm_data.c \
../ias_lib/los_model/sensor/ias_sensor_check_ssm_encoder_data.c \
../ias_lib/los_model/sensor/ias_sensor_find_los_vector.c \
../ias_lib/los_model/sensor/ias_sensor_find_time.c \
../ias_lib/los_model/sensor/ias_sensor_get_jitter.c \
../ias_lib/los_model/sensor/ias_sensor_get_maximum_detector_delay.c \
../ias_lib/los_model/sensor/ias_sensor_set_cpf.c \
../ias_lib/los_model/sensor/ias_sensor_set_frame_times.c \
../ias_lib/los_model/sensor/ias_sensor_set_l0r.c \
../ias_lib/los_model/sensor/ias_sensor_set_ssm_from_l0r.c \
../ias_lib/los_model/sensor/ias_sensor_smooth_ssm_data.c 

OBJS += \
./ias_lib/los_model/sensor/ias_sensor_adjust_los_for_ssm.o \
./ias_lib/los_model/sensor/ias_sensor_align_ssm_data.o \
./ias_lib/los_model/sensor/ias_sensor_check_ssm_encoder_data.o \
./ias_lib/los_model/sensor/ias_sensor_find_los_vector.o \
./ias_lib/los_model/sensor/ias_sensor_find_time.o \
./ias_lib/los_model/sensor/ias_sensor_get_jitter.o \
./ias_lib/los_model/sensor/ias_sensor_get_maximum_detector_delay.o \
./ias_lib/los_model/sensor/ias_sensor_set_cpf.o \
./ias_lib/los_model/sensor/ias_sensor_set_frame_times.o \
./ias_lib/los_model/sensor/ias_sensor_set_l0r.o \
./ias_lib/los_model/sensor/ias_sensor_set_ssm_from_l0r.o \
./ias_lib/los_model/sensor/ias_sensor_smooth_ssm_data.o 

C_DEPS += \
./ias_lib/los_model/sensor/ias_sensor_adjust_los_for_ssm.d \
./ias_lib/los_model/sensor/ias_sensor_align_ssm_data.d \
./ias_lib/los_model/sensor/ias_sensor_check_ssm_encoder_data.d \
./ias_lib/los_model/sensor/ias_sensor_find_los_vector.d \
./ias_lib/los_model/sensor/ias_sensor_find_time.d \
./ias_lib/los_model/sensor/ias_sensor_get_jitter.d \
./ias_lib/los_model/sensor/ias_sensor_get_maximum_detector_delay.d \
./ias_lib/los_model/sensor/ias_sensor_set_cpf.d \
./ias_lib/los_model/sensor/ias_sensor_set_frame_times.d \
./ias_lib/los_model/sensor/ias_sensor_set_l0r.d \
./ias_lib/los_model/sensor/ias_sensor_set_ssm_from_l0r.d \
./ias_lib/los_model/sensor/ias_sensor_smooth_ssm_data.d 


# Each subdirectory must supply rules for building sources it contributes
ias_lib/los_model/sensor/%.o: ../ias_lib/los_model/sensor/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I/DATA/DPAS5_COTS/odl -I/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc -I/DATA/DPAS5_COTS/hdf5/include -I/DATA/DPAS5_COTS/novas3.1/include -I/DATA/DPAS5_COTS/gctp3/include -I/DATA/DPAS5_COTS/remez/include -I/DATA/DPAS5_COTS/gsl/include/gsl -I/DATA/DPAS5_COTS/tiff/include -I/usr/local/include -I/DATA/DPAS5_COTS/unixodbc/include -I"/home/cqw/workplace_1/Get_Geodetic/MQ" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/ancillary" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/ancillary_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_database" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/cpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/gcp" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/geometric_grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L0R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1G" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/parameter_file_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/rlut" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/sensor" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/spacecraft" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/database_access" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/geo" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/math" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/miscellaneous" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/odl" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/pixel_mask" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/satellite_attributes" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/threading" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/perllib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/setup" -I"/home/cqw/workplace_1/Get_Geodetic" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


