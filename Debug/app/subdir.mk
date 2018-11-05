################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app/main.c \
../app/system.c 

OBJS += \
./app/main.o \
./app/system.o 

C_DEPS += \
./app/main.d \
./app/system.d 


# Each subdirectory must supply rules for building sources it contributes
app/%.o: ../app/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	mipsel-openwrt-linux-uclibc-gcc -I"/home/work/.workspace/multi_port_convertor" -I"/home/work/.workspace/multi_port_convertor/app" -I"/home/work/.workspace/multi_port_convertor/net" -I"/home/work/.workspace/multi_port_convertor/tool" -I"/home/work/.workspace/multi_port_convertor/uart" -I"/home/work/.workspace/multi_port_convertor/usb" -I/home/work/openwrt_widora-master/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include -I/home/work/openwrt_widora-master/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include/glib-2.0 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


