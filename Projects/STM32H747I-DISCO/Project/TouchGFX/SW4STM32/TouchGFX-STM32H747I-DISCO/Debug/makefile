################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middleware/USB_Host/Core/subdir.mk
-include Middleware/USB_Host/Class/MSC/subdir.mk
-include Middleware/TouchGFX/Target/subdir.mk
-include Middleware/TouchGFX/OS/subdir.mk
-include Middleware/MinIni/subdir.mk
-include Middleware/Libjpeg/subdir.mk
-include Middleware/FreeRTOS/Portable/subdir.mk
-include Middleware/FreeRTOS/MemMgr/subdir.mk
-include Middleware/FreeRTOS/CMSIS_RTOS/subdir.mk
-include Middleware/FreeRTOS/subdir.mk
-include Middleware/FatFS/src/subdir.mk
-include Libraries/subdir.mk
-include Drivers/STM32H7xx_HAL_Driver/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include Drivers/BSP/Components/subdir.mk
-include Drivers/BSP/subdir.mk
-include Application/User/TouchGFX/images/subdir.mk
-include Application/User/TouchGFX/gui/subdir.mk
-include Application/User/Core/subdir.mk
-include Application/SW4STM32/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: UvSmart_CM7.elf

# Tool invocations
UvSmart_CM7.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU G++ Linker'
	arm-none-eabi-g++ -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -L"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/ST/TouchGFX/touchgfx/lib/core/cortex_m7/gcc"  -specs=nosys.specs -specs=nano.specs -T"../stm32h747xx_flash_CM7.ld" -Wl,-Map=output.map -Wl,--gc-sections -fno-exceptions -fno-rtti -o "UvSmart_CM7.elf" @"objects.list" $(USER_OBJS) $(LIBS) -lm
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

post-build:
	-@echo 'Generating hex and Printing size information:'
	arm-none-eabi-objcopy -O binary "UvSmart_CM7.elf" "UvSmart_CM7.bin"
	arm-none-eabi-size "UvSmart_CM7.elf"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
