################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OBJS += $(OUT_PATH)/main.o 
OBJS +=$(OUT_PATH)/main_light.o 
OBJS +=$(OUT_PATH)/vendor_att_light.o 
OBJS +=$(OUT_PATH)/vendor_light.o 

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/%.o: ./%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"