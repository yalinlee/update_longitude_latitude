################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ias_lib/ancillary/ias_ancillary_build_ephemeris.c \
../ias_lib/ancillary/ias_ancillary_compute_spacecraft_attitude.c \
../ias_lib/ancillary/ias_ancillary_convert_imu_to_acs.c \
../ias_lib/ancillary/ias_ancillary_convert_imu_to_attitude.c \
../ias_lib/ancillary/ias_ancillary_correct_ephemeris_time.c \
../ias_lib/ancillary/ias_ancillary_correct_imu_orbital_motion.c \
../ias_lib/ancillary/ias_ancillary_extract_valid_imu_data_window.c \
../ias_lib/ancillary/ias_ancillary_extract_valid_quaternion_window.c \
../ias_lib/ancillary/ias_ancillary_get_position_and_velocity_at_time.c \
../ias_lib/ancillary/ias_ancillary_get_quaternion_at_time.c \
../ias_lib/ancillary/ias_ancillary_get_start_stop_frame_times.c \
../ias_lib/ancillary/ias_ancillary_identify_quaternion_outliers.c \
../ias_lib/ancillary/ias_ancillary_kalman_smooth_ephemeris.c \
../ias_lib/ancillary/ias_ancillary_kalman_smooth_imu.c \
../ias_lib/ancillary/ias_ancillary_preprocess.c \
../ias_lib/ancillary/ias_ancillary_preprocess_attitude.c \
../ias_lib/ancillary/ias_ancillary_preprocess_ephemeris.c \
../ias_lib/ancillary/ias_ancillary_smooth_ephemeris.c 

OBJS += \
./ias_lib/ancillary/ias_ancillary_build_ephemeris.o \
./ias_lib/ancillary/ias_ancillary_compute_spacecraft_attitude.o \
./ias_lib/ancillary/ias_ancillary_convert_imu_to_acs.o \
./ias_lib/ancillary/ias_ancillary_convert_imu_to_attitude.o \
./ias_lib/ancillary/ias_ancillary_correct_ephemeris_time.o \
./ias_lib/ancillary/ias_ancillary_correct_imu_orbital_motion.o \
./ias_lib/ancillary/ias_ancillary_extract_valid_imu_data_window.o \
./ias_lib/ancillary/ias_ancillary_extract_valid_quaternion_window.o \
./ias_lib/ancillary/ias_ancillary_get_position_and_velocity_at_time.o \
./ias_lib/ancillary/ias_ancillary_get_quaternion_at_time.o \
./ias_lib/ancillary/ias_ancillary_get_start_stop_frame_times.o \
./ias_lib/ancillary/ias_ancillary_identify_quaternion_outliers.o \
./ias_lib/ancillary/ias_ancillary_kalman_smooth_ephemeris.o \
./ias_lib/ancillary/ias_ancillary_kalman_smooth_imu.o \
./ias_lib/ancillary/ias_ancillary_preprocess.o \
./ias_lib/ancillary/ias_ancillary_preprocess_attitude.o \
./ias_lib/ancillary/ias_ancillary_preprocess_ephemeris.o \
./ias_lib/ancillary/ias_ancillary_smooth_ephemeris.o 

C_DEPS += \
./ias_lib/ancillary/ias_ancillary_build_ephemeris.d \
./ias_lib/ancillary/ias_ancillary_compute_spacecraft_attitude.d \
./ias_lib/ancillary/ias_ancillary_convert_imu_to_acs.d \
./ias_lib/ancillary/ias_ancillary_convert_imu_to_attitude.d \
./ias_lib/ancillary/ias_ancillary_correct_ephemeris_time.d \
./ias_lib/ancillary/ias_ancillary_correct_imu_orbital_motion.d \
./ias_lib/ancillary/ias_ancillary_extract_valid_imu_data_window.d \
./ias_lib/ancillary/ias_ancillary_extract_valid_quaternion_window.d \
./ias_lib/ancillary/ias_ancillary_get_position_and_velocity_at_time.d \
./ias_lib/ancillary/ias_ancillary_get_quaternion_at_time.d \
./ias_lib/ancillary/ias_ancillary_get_start_stop_frame_times.d \
./ias_lib/ancillary/ias_ancillary_identify_quaternion_outliers.d \
./ias_lib/ancillary/ias_ancillary_kalman_smooth_ephemeris.d \
./ias_lib/ancillary/ias_ancillary_kalman_smooth_imu.d \
./ias_lib/ancillary/ias_ancillary_preprocess.d \
./ias_lib/ancillary/ias_ancillary_preprocess_attitude.d \
./ias_lib/ancillary/ias_ancillary_preprocess_ephemeris.d \
./ias_lib/ancillary/ias_ancillary_smooth_ephemeris.d 


# Each subdirectory must supply rules for building sources it contributes
ias_lib/ancillary/%.o: ../ias_lib/ancillary/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I/DATA/DPAS5_COTS/odl -I/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc -I/DATA/DPAS5_COTS/hdf5/include -I/DATA/DPAS5_COTS/novas3.1/include -I/DATA/DPAS5_COTS/gctp3/include -I/DATA/DPAS5_COTS/remez/include -I/DATA/DPAS5_COTS/gsl/include/gsl -I/DATA/DPAS5_COTS/tiff/include -I/usr/local/include -I/DATA/DPAS5_COTS/unixodbc/include -I"/home/cqw/workplace_1/Get_Geodetic/MQ" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/ancillary" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/ancillary_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_database" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/cpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/gcp" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/geometric_grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L0R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1G" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/parameter_file_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/rlut" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/sensor" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/spacecraft" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/database_access" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/geo" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/math" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/miscellaneous" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/odl" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/pixel_mask" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/satellite_attributes" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/threading" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/perllib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/setup" -I"/home/cqw/workplace_1/Get_Geodetic" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


