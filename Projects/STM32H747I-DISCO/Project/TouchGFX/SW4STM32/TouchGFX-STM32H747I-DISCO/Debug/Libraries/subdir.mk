################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/Audio.c \
../Libraries/Config.c \
../Libraries/Debug.c \
../Libraries/Lid.c \
../Libraries/MicrosecondTimer.c \
../Libraries/MillisecondTimer.c \
../Libraries/RealTimeClock.c \
../Libraries/Rgb.c \
../Libraries/Storage.c \
../Libraries/UvLight.c 

CPP_SRCS += \
../Libraries/Unit.cpp 

OBJS += \
./Libraries/Audio.o \
./Libraries/Config.o \
./Libraries/Debug.o \
./Libraries/Lid.o \
./Libraries/MicrosecondTimer.o \
./Libraries/MillisecondTimer.o \
./Libraries/RealTimeClock.o \
./Libraries/Rgb.o \
./Libraries/Storage.o \
./Libraries/Unit.o \
./Libraries/UvLight.o 

C_DEPS += \
./Libraries/Audio.d \
./Libraries/Config.d \
./Libraries/Debug.d \
./Libraries/Lid.d \
./Libraries/MicrosecondTimer.d \
./Libraries/MillisecondTimer.d \
./Libraries/RealTimeClock.d \
./Libraries/Rgb.d \
./Libraries/Storage.d \
./Libraries/UvLight.d 

CPP_DEPS += \
./Libraries/Unit.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/%.o: ../Libraries/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -DUSE_HAL_DRIVER -DSTM32H747xx -DUSE_STM32H747I_DISCO -DBSP_USE_CMSIS_OS -DCORE_CM7 '-DUSE_BPP=24' -DST -DUSE_FLOATING_POINT -DUSE_USB_HS -DUSE_PWR_LDO_SUPPLY__N -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/Gui/generated/images/include" -I../../../Gui/generated/fonts/include -I../../../Gui/generated/texts/include -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/Gui/gui/include" -I../../../Gui/gui/include/gui/audio_player_screen -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/ST/TouchGFX/touchgfx/framework/include" -I"/Users/ppronk/CLionProjects/UVSmartD25/Drivers/STM32H7xx_HAL_Driver/Inc" -I"/Users/ppronk/CLionProjects/UVSmartD25/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"/Users/ppronk/CLionProjects/UVSmartD25/Drivers/BSP/STM32H747I-Discovery" -I"/Users/ppronk/CLionProjects/UVSmartD25/Utilities/JPEG" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/Third_Party/FreeRTOS/Source/include" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/ST/STM32_USB_Host_Library/Core/Inc" -I../../../../../../../Middlewares/Third_Party/FatFs/src -I../../../../../../../Middlewares/Third_Party/FatFs/src/drivers -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/Third_Party/LibJPEG/include" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/Config" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/Core/Inc" -I"/Users/ppronk/CLionProjects/UVSmartD25/Drivers/CMSIS/Include" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/Gui/target" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/ST/STM32_Audio/Addons/PDM/Inc" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/SW4STM32/TouchGFX-STM32H747I-DISCO/Libraries" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/SW4STM32/TouchGFX-STM32H747I-DISCO/Middleware/FatFS/src" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/SW4STM32/TouchGFX-STM32H747I-DISCO/Middleware/MinIni"  -Os -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Libraries/%.o: ../Libraries/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU G++ Compiler'
	@echo $(PWD)
	arm-none-eabi-g++ -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -DUSE_HAL_DRIVER -DSTM32H747xx -DUSE_STM32H747I_DISCO -DBSP_USE_CMSIS_OS -DCORE_CM7 '-DUSE_BPP=24' -DST -DUSE_FLOATING_POINT -DUSE_USB_HS -DUSE_PWR_LDO_SUPPLY__N -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/Gui/generated/images/include" -I../../../Gui/generated/fonts/include -I../../../Gui/generated/texts/include -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/Gui/gui/include" -I../../../Gui/gui/include/gui/audio_player_screen -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/ST/TouchGFX/touchgfx/framework/include" -I"/Users/ppronk/CLionProjects/UVSmartD25/Drivers/STM32H7xx_HAL_Driver/Inc" -I"/Users/ppronk/CLionProjects/UVSmartD25/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"/Users/ppronk/CLionProjects/UVSmartD25/Drivers/BSP/STM32H747I-Discovery" -I"/Users/ppronk/CLionProjects/UVSmartD25/Utilities/JPEG" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/Third_Party/FreeRTOS/Source/include" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/ST/STM32_USB_Host_Library/Core/Inc" -I../../../../../../../Middlewares/Third_Party/FatFs/src -I../../../../../../../Middlewares/Third_Party/FatFs/src/drivers -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/Third_Party/LibJPEG/include" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/Config" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/Core/Inc" -I"/Users/ppronk/CLionProjects/UVSmartD25/Drivers/CMSIS/Include" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/Gui/target" -I"/Users/ppronk/CLionProjects/UVSmartD25/Middlewares/ST/STM32_Audio/Addons/PDM/Inc" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/SW4STM32/TouchGFX-STM32H747I-DISCO/Libraries" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/SW4STM32/TouchGFX-STM32H747I-DISCO/Middleware/FatFS/src" -I"/Users/ppronk/CLionProjects/UVSmartD25/Projects/STM32H747I-DISCO/Project/TouchGFX/SW4STM32/TouchGFX-STM32H747I-DISCO/Middleware/MinIni"  -Os -Wall -fmessage-length=0 -ffunction-sections -c -fno-exceptions -fno-rtti -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


