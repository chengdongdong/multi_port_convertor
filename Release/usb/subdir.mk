################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../usb/udisk.c \
../usb/usb.c \
../usb/usb_communicat.c 

OBJS += \
./usb/udisk.o \
./usb/usb.o \
./usb/usb_communicat.o 

C_DEPS += \
./usb/udisk.d \
./usb/usb.d \
./usb/usb_communicat.d 


# Each subdirectory must supply rules for building sources it contributes
usb/%.o: ../usb/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	mipsel-openwrt-linux-gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


