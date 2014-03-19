################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ias_lib/los_model/ias_los_model_allocate.c \
../ias_lib/los_model/ias_los_model_apply_precision_params.c \
../ias_lib/los_model/ias_los_model_build_jitter_table.c \
../ias_lib/los_model/ias_los_model_calc_scene_corners.c \
../ias_lib/los_model/ias_los_model_get_moon_position_at_location.c \
../ias_lib/los_model/ias_los_model_get_satellite_state_vector_at_location.c \
../ias_lib/los_model/ias_los_model_initialize.c \
../ias_lib/los_model/ias_los_model_input_line_samp_to_geodetic.c \
../ias_lib/los_model/ias_los_model_lunar_projection.c \
../ias_lib/los_model/ias_los_model_set_cpf.c \
../ias_lib/los_model/ias_los_model_set_l0r.c 

OBJS += \
./ias_lib/los_model/ias_los_model_allocate.o \
./ias_lib/los_model/ias_los_model_apply_precision_params.o \
./ias_lib/los_model/ias_los_model_build_jitter_table.o \
./ias_lib/los_model/ias_los_model_calc_scene_corners.o \
./ias_lib/los_model/ias_los_model_get_moon_position_at_location.o \
./ias_lib/los_model/ias_los_model_get_satellite_state_vector_at_location.o \
./ias_lib/los_model/ias_los_model_initialize.o \
./ias_lib/los_model/ias_los_model_input_line_samp_to_geodetic.o \
./ias_lib/los_model/ias_los_model_lunar_projection.o \
./ias_lib/los_model/ias_los_model_set_cpf.o \
./ias_lib/los_model/ias_los_model_set_l0r.o 

C_DEPS += \
./ias_lib/los_model/ias_los_model_allocate.d \
./ias_lib/los_model/ias_los_model_apply_precision_params.d \
./ias_lib/los_model/ias_los_model_build_jitter_table.d \
./ias_lib/los_model/ias_los_model_calc_scene_corners.d \
./ias_lib/los_model/ias_los_model_get_moon_position_at_location.d \
./ias_lib/los_model/ias_los_model_get_satellite_state_vector_at_location.d \
./ias_lib/los_model/ias_los_model_initialize.d \
./ias_lib/los_model/ias_los_model_input_line_samp_to_geodetic.d \
./ias_lib/los_model/ias_los_model_lunar_projection.d \
./ias_lib/los_model/ias_los_model_set_cpf.d \
./ias_lib/los_model/ias_los_model_set_l0r.d 


# Each subdirectory must supply rules for building sources it contributes
ias_lib/los_model/%.o: ../ias_lib/los_model/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I/DATA/DPAS5_COTS/odl -I/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc -I/DATA/DPAS5_COTS/hdf5/include -I/DATA/DPAS5_COTS/novas3.1/include -I/DATA/DPAS5_COTS/gctp3/include -I/DATA/DPAS5_COTS/remez/include -I/DATA/DPAS5_COTS/gsl/include/gsl -I/DATA/DPAS5_COTS/tiff/include -I/usr/local/include -I/DATA/DPAS5_COTS/unixodbc/include -I"/home/cqw/workplace_1/Get_Geodetic/MQ" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/ancillary" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/ancillary_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_database" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/cpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/gcp" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/geometric_grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L0R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1G" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/parameter_file_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/rlut" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/sensor" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/spacecraft" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/database_access" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/geo" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/math" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/miscellaneous" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/odl" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/pixel_mask" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/satellite_attributes" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/threading" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/perllib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/setup" -I"/home/cqw/workplace_1/Get_Geodetic" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


