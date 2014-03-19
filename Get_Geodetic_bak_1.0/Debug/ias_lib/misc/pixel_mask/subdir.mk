################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ias_lib/misc/pixel_mask/ias_pm_add_pixels.c \
../ias_lib/misc/pixel_mask/ias_pm_close_pixel_mask.c \
../ias_lib/misc/pixel_mask/ias_pm_create.c \
../ias_lib/misc/pixel_mask/ias_pm_destroy.c \
../ias_lib/misc/pixel_mask/ias_pm_destroy_iterator.c \
../ias_lib/misc/pixel_mask/ias_pm_get_band_number.c \
../ias_lib/misc/pixel_mask/ias_pm_get_file_band_sca_list.c \
../ias_lib/misc/pixel_mask/ias_pm_get_image.c \
../ias_lib/misc/pixel_mask/ias_pm_get_image_subset.c \
../ias_lib/misc/pixel_mask/ias_pm_get_iterator.c \
../ias_lib/misc/pixel_mask/ias_pm_get_mask_at.c \
../ias_lib/misc/pixel_mask/ias_pm_get_mask_index.c \
../ias_lib/misc/pixel_mask/ias_pm_get_next_span.c \
../ias_lib/misc/pixel_mask/ias_pm_get_number_of_detectors.c \
../ias_lib/misc/pixel_mask/ias_pm_get_number_of_pixels.c \
../ias_lib/misc/pixel_mask/ias_pm_get_sca_number.c \
../ias_lib/misc/pixel_mask/ias_pm_is_detector_flagged.c \
../ias_lib/misc/pixel_mask/ias_pm_open_pixel_mask.c \
../ias_lib/misc/pixel_mask/ias_pm_read_array_from_file.c \
../ias_lib/misc/pixel_mask/ias_pm_read_mask_data_from_file.c \
../ias_lib/misc/pixel_mask/ias_pm_read_single_mask_from_file.c \
../ias_lib/misc/pixel_mask/ias_pm_write_array_to_file.c \
../ias_lib/misc/pixel_mask/ias_pm_write_mask_data_to_file.c \
../ias_lib/misc/pixel_mask/ias_pm_write_single_mask_to_file.c 

OBJS += \
./ias_lib/misc/pixel_mask/ias_pm_add_pixels.o \
./ias_lib/misc/pixel_mask/ias_pm_close_pixel_mask.o \
./ias_lib/misc/pixel_mask/ias_pm_create.o \
./ias_lib/misc/pixel_mask/ias_pm_destroy.o \
./ias_lib/misc/pixel_mask/ias_pm_destroy_iterator.o \
./ias_lib/misc/pixel_mask/ias_pm_get_band_number.o \
./ias_lib/misc/pixel_mask/ias_pm_get_file_band_sca_list.o \
./ias_lib/misc/pixel_mask/ias_pm_get_image.o \
./ias_lib/misc/pixel_mask/ias_pm_get_image_subset.o \
./ias_lib/misc/pixel_mask/ias_pm_get_iterator.o \
./ias_lib/misc/pixel_mask/ias_pm_get_mask_at.o \
./ias_lib/misc/pixel_mask/ias_pm_get_mask_index.o \
./ias_lib/misc/pixel_mask/ias_pm_get_next_span.o \
./ias_lib/misc/pixel_mask/ias_pm_get_number_of_detectors.o \
./ias_lib/misc/pixel_mask/ias_pm_get_number_of_pixels.o \
./ias_lib/misc/pixel_mask/ias_pm_get_sca_number.o \
./ias_lib/misc/pixel_mask/ias_pm_is_detector_flagged.o \
./ias_lib/misc/pixel_mask/ias_pm_open_pixel_mask.o \
./ias_lib/misc/pixel_mask/ias_pm_read_array_from_file.o \
./ias_lib/misc/pixel_mask/ias_pm_read_mask_data_from_file.o \
./ias_lib/misc/pixel_mask/ias_pm_read_single_mask_from_file.o \
./ias_lib/misc/pixel_mask/ias_pm_write_array_to_file.o \
./ias_lib/misc/pixel_mask/ias_pm_write_mask_data_to_file.o \
./ias_lib/misc/pixel_mask/ias_pm_write_single_mask_to_file.o 

C_DEPS += \
./ias_lib/misc/pixel_mask/ias_pm_add_pixels.d \
./ias_lib/misc/pixel_mask/ias_pm_close_pixel_mask.d \
./ias_lib/misc/pixel_mask/ias_pm_create.d \
./ias_lib/misc/pixel_mask/ias_pm_destroy.d \
./ias_lib/misc/pixel_mask/ias_pm_destroy_iterator.d \
./ias_lib/misc/pixel_mask/ias_pm_get_band_number.d \
./ias_lib/misc/pixel_mask/ias_pm_get_file_band_sca_list.d \
./ias_lib/misc/pixel_mask/ias_pm_get_image.d \
./ias_lib/misc/pixel_mask/ias_pm_get_image_subset.d \
./ias_lib/misc/pixel_mask/ias_pm_get_iterator.d \
./ias_lib/misc/pixel_mask/ias_pm_get_mask_at.d \
./ias_lib/misc/pixel_mask/ias_pm_get_mask_index.d \
./ias_lib/misc/pixel_mask/ias_pm_get_next_span.d \
./ias_lib/misc/pixel_mask/ias_pm_get_number_of_detectors.d \
./ias_lib/misc/pixel_mask/ias_pm_get_number_of_pixels.d \
./ias_lib/misc/pixel_mask/ias_pm_get_sca_number.d \
./ias_lib/misc/pixel_mask/ias_pm_is_detector_flagged.d \
./ias_lib/misc/pixel_mask/ias_pm_open_pixel_mask.d \
./ias_lib/misc/pixel_mask/ias_pm_read_array_from_file.d \
./ias_lib/misc/pixel_mask/ias_pm_read_mask_data_from_file.d \
./ias_lib/misc/pixel_mask/ias_pm_read_single_mask_from_file.d \
./ias_lib/misc/pixel_mask/ias_pm_write_array_to_file.d \
./ias_lib/misc/pixel_mask/ias_pm_write_mask_data_to_file.d \
./ias_lib/misc/pixel_mask/ias_pm_write_single_mask_to_file.d 


# Each subdirectory must supply rules for building sources it contributes
ias_lib/misc/pixel_mask/%.o: ../ias_lib/misc/pixel_mask/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I/DATA/DPAS5_COTS/odl -I/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc -I/DATA/DPAS5_COTS/hdf5/include -I/DATA/DPAS5_COTS/novas3.1/include -I/DATA/DPAS5_COTS/gctp3/include -I/DATA/DPAS5_COTS/remez/include -I/DATA/DPAS5_COTS/gsl/include/gsl -I/DATA/DPAS5_COTS/tiff/include -I/usr/local/include -I/DATA/DPAS5_COTS/unixodbc/include -I"/home/cqw/workplace_1/Get_Geodetic/MQ" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/ancillary" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/ancillary_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_database" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/cpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/gcp" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/geometric_grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L0R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1G" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/parameter_file_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/rlut" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/sensor" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/spacecraft" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/database_access" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/geo" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/math" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/miscellaneous" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/odl" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/pixel_mask" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/satellite_attributes" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/threading" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/perllib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/setup" -I"/home/cqw/workplace_1/Get_Geodetic" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


