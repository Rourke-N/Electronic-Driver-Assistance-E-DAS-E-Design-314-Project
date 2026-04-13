################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Accelerometer.c \
../Core/Src/DistanceSensor.c \
../Core/Src/GPS.c \
../Core/Src/General.c \
../Core/Src/Keypad.c \
../Core/Src/LightSensor.c \
../Core/Src/TempSensor.c \
../Core/Src/main.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c 

OBJS += \
./Core/Src/Accelerometer.o \
./Core/Src/DistanceSensor.o \
./Core/Src/GPS.o \
./Core/Src/General.o \
./Core/Src/Keypad.o \
./Core/Src/LightSensor.o \
./Core/Src/TempSensor.o \
./Core/Src/main.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o 

C_DEPS += \
./Core/Src/Accelerometer.d \
./Core/Src/DistanceSensor.d \
./Core/Src/GPS.d \
./Core/Src/General.d \
./Core/Src/Keypad.d \
./Core/Src/LightSensor.d \
./Core/Src/TempSensor.d \
./Core/Src/main.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/Accelerometer.cyclo ./Core/Src/Accelerometer.d ./Core/Src/Accelerometer.o ./Core/Src/Accelerometer.su ./Core/Src/DistanceSensor.cyclo ./Core/Src/DistanceSensor.d ./Core/Src/DistanceSensor.o ./Core/Src/DistanceSensor.su ./Core/Src/GPS.cyclo ./Core/Src/GPS.d ./Core/Src/GPS.o ./Core/Src/GPS.su ./Core/Src/General.cyclo ./Core/Src/General.d ./Core/Src/General.o ./Core/Src/General.su ./Core/Src/Keypad.cyclo ./Core/Src/Keypad.d ./Core/Src/Keypad.o ./Core/Src/Keypad.su ./Core/Src/LightSensor.cyclo ./Core/Src/LightSensor.d ./Core/Src/LightSensor.o ./Core/Src/LightSensor.su ./Core/Src/TempSensor.cyclo ./Core/Src/TempSensor.d ./Core/Src/TempSensor.o ./Core/Src/TempSensor.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su

.PHONY: clean-Core-2f-Src

