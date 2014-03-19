################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ias_lib/misc/odl/ias_odl_add_field.c \
../ias_lib/misc/odl/ias_odl_add_field_list.c \
../ias_lib/misc/odl/ias_odl_add_group.c \
../ias_lib/misc/odl/ias_odl_add_or_replace_field.c \
../ias_lib/misc/odl/ias_odl_create_tree.c \
../ias_lib/misc/odl/ias_odl_find_keyword.c \
../ias_lib/misc/odl/ias_odl_find_object_description.c \
../ias_lib/misc/odl/ias_odl_free_tree.c \
../ias_lib/misc/odl/ias_odl_get_field.c \
../ias_lib/misc/odl/ias_odl_get_field_list.c \
../ias_lib/misc/odl/ias_odl_get_group.c \
../ias_lib/misc/odl/ias_odl_get_group_names.c \
../ias_lib/misc/odl/ias_odl_parse_file.c \
../ias_lib/misc/odl/ias_odl_parse_label_string.c \
../ias_lib/misc/odl/ias_odl_read_tree.c \
../ias_lib/misc/odl/ias_odl_remove_character.c \
../ias_lib/misc/odl/ias_odl_replace_field.c \
../ias_lib/misc/odl/ias_odl_write_tree.c 

OBJS += \
./ias_lib/misc/odl/ias_odl_add_field.o \
./ias_lib/misc/odl/ias_odl_add_field_list.o \
./ias_lib/misc/odl/ias_odl_add_group.o \
./ias_lib/misc/odl/ias_odl_add_or_replace_field.o \
./ias_lib/misc/odl/ias_odl_create_tree.o \
./ias_lib/misc/odl/ias_odl_find_keyword.o \
./ias_lib/misc/odl/ias_odl_find_object_description.o \
./ias_lib/misc/odl/ias_odl_free_tree.o \
./ias_lib/misc/odl/ias_odl_get_field.o \
./ias_lib/misc/odl/ias_odl_get_field_list.o \
./ias_lib/misc/odl/ias_odl_get_group.o \
./ias_lib/misc/odl/ias_odl_get_group_names.o \
./ias_lib/misc/odl/ias_odl_parse_file.o \
./ias_lib/misc/odl/ias_odl_parse_label_string.o \
./ias_lib/misc/odl/ias_odl_read_tree.o \
./ias_lib/misc/odl/ias_odl_remove_character.o \
./ias_lib/misc/odl/ias_odl_replace_field.o \
./ias_lib/misc/odl/ias_odl_write_tree.o 

C_DEPS += \
./ias_lib/misc/odl/ias_odl_add_field.d \
./ias_lib/misc/odl/ias_odl_add_field_list.d \
./ias_lib/misc/odl/ias_odl_add_group.d \
./ias_lib/misc/odl/ias_odl_add_or_replace_field.d \
./ias_lib/misc/odl/ias_odl_create_tree.d \
./ias_lib/misc/odl/ias_odl_find_keyword.d \
./ias_lib/misc/odl/ias_odl_find_object_description.d \
./ias_lib/misc/odl/ias_odl_free_tree.d \
./ias_lib/misc/odl/ias_odl_get_field.d \
./ias_lib/misc/odl/ias_odl_get_field_list.d \
./ias_lib/misc/odl/ias_odl_get_group.d \
./ias_lib/misc/odl/ias_odl_get_group_names.d \
./ias_lib/misc/odl/ias_odl_parse_file.d \
./ias_lib/misc/odl/ias_odl_parse_label_string.d \
./ias_lib/misc/odl/ias_odl_read_tree.d \
./ias_lib/misc/odl/ias_odl_remove_character.d \
./ias_lib/misc/odl/ias_odl_replace_field.d \
./ias_lib/misc/odl/ias_odl_write_tree.d 


# Each subdirectory must supply rules for building sources it contributes
ias_lib/misc/odl/%.o: ../ias_lib/misc/odl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I/DATA/DPAS5_COTS/odl -I/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc -I/DATA/DPAS5_COTS/hdf5/include -I/DATA/DPAS5_COTS/novas3.1/include -I/DATA/DPAS5_COTS/gctp3/include -I/DATA/DPAS5_COTS/remez/include -I/DATA/DPAS5_COTS/gsl/include/gsl -I/DATA/DPAS5_COTS/tiff/include -I/usr/local/include -I/DATA/DPAS5_COTS/unixodbc/include -I"/home/cqw/workplace_1/Get_Geodetic/MQ" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/ancillary" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/ancillary_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_database" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/bpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/cpf_file" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/gcp" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/geometric_grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/grid" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L0R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1G" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/L1R" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/parameter_file_io" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/io/rlut" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/sensor" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/los_model/spacecraft" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/database_access" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/geo" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/math" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/miscellaneous" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/odl" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/pixel_mask" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/satellite_attributes" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/misc/threading" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/perllib" -I"/home/cqw/workplace_1/Get_Geodetic/ias_lib/setup" -I"/home/cqw/workplace_1/Get_Geodetic" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


