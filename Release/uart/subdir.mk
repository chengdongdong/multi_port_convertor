################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../uart/uart.c \
../uart/uart_communicat.c \
../uart/uart_protocol.c 

OBJS += \
./uart/uart.o \
./uart/uart_communicat.o \
./uart/uart_protocol.o 

C_DEPS += \
./uart/uart.d \
./uart/uart_communicat.d \
./uart/uart_protocol.d 


# Each subdirectory must supply rules for building sources it contributes
uart/%.o: ../uart/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	mipsel-openwrt-linux-gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


