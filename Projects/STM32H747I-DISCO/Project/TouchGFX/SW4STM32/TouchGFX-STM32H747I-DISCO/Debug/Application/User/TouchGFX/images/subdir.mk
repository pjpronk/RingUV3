################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
D:/Software/UvSmart/Projects/STM32H747I-DISCO/Project/TouchGFX/Gui/generated/images/src/BitmapDatabase.cpp 

OBJS += \
./Application/User/TouchGFX/images/BitmapDatabase.o 

CPP_DEPS += \
./Application/User/TouchGFX/images/BitmapDatabase.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/TouchGFX/images/BitmapDatabase.o: D:/Software/UvSmart/Projects/STM32H747I-DISCO/Project/TouchGFX/Gui/generated/images/src/BitmapDatabase.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU G++ Compiler'
	@echo $(PWD)
	arm-none-eabi-g++ -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -DUSE_HAL_DRIVER -DSTM32H747xx -DUSE_STM32H747I_DISCO -DBSP_USE_CMSIS_OS -DCORE_CM7 '-DUSE_BPP=24' -DST -DUSE_FLOATING_POINT -DUSE_USB_HS -DUSE_PWR_LDO_SUPPLY__N -I"D:/Software/UvSmart/Projects/STM32H747I-DISCO/Project/TouchGFX/Gui/generated/images/include" -I../../../Gui/generated/fonts/include -I../../../Gui/generated/texts/include -I"D:/Software/UvSmart/Projects/STM32H747I-DISCO/Project/TouchGFX/Gui/gui/include" -I../../../Gui/gui/include/gui/audio_player_screen -I"D:/Software/UvSmart/Middlewares/ST/TouchGFX/touchgfx/framework/include" -I"D:/Software/UvSmart/Drivers/STM32H7xx_HAL_Driver/Inc" -I"D:/Software/UvSmart/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"D:/Software/UvSmart/Drivers/BSP/STM32H747I-Discovery" -I"D:/Software/UvSmart/Utilities/JPEG" -I"D:/Software/UvSmart/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"D:/Software/UvSmart/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I"D:/Software/UvSmart/Middlewares/Third_Party/FreeRTOS/Source/include" -I"D:/Software/UvSmart/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc" -I"D:/Software/UvSmart/Middlewares/ST/STM32_USB_Host_Library/Core/Inc" -I../../../../../../../Middlewares/Third_Party/FatFs/src -I../../../../../../../Middlewares/Third_Party/FatFs/src/drivers -I"D:/Software/UvSmart/Middlewares/Third_Party/LibJPEG/include" -I"D:/Software/UvSmart/Projects/STM32H747I-DISCO/Project/TouchGFX/Config" -I"D:/Software/UvSmart/Projects/STM32H747I-DISCO/Project/TouchGFX/Core/Inc" -I"D:/Software/UvSmart/Drivers/CMSIS/Include" -I"D:/Software/UvSmart/Projects/STM32H747I-DISCO/Project/TouchGFX" -I"D:/Software/UvSmart/Projects/STM32H747I-DISCO/Project/TouchGFX/Gui/target" -I"D:/Software/UvSmart/Middlewares/ST/STM32_Audio/Addons/PDM/Inc" -I"D:/Software/UvSmart/Projects/STM32H747I-DISCO/Project/TouchGFX/SW4STM32/TouchGFX-STM32H747I-DISCO/Libraries" -I"D:/Software/UvSmart/Projects/STM32H747I-DISCO/Project/TouchGFX/SW4STM32/TouchGFX-STM32H747I-DISCO/Middleware/FatFS/src" -I"D:/Software/UvSmart/Projects/STM32H747I-DISCO/Project/TouchGFX/SW4STM32/TouchGFX-STM32H747I-DISCO/Middleware/MinIni"  -Os -Wall -fmessage-length=0 -ffunction-sections -c -fno-exceptions -fno-rtti -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


