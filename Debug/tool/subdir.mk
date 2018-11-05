################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../tool/buf_factory.c \
../tool/cJSON.c \
../tool/key_file.c \
../tool/list.c \
../tool/mailbox.c \
../tool/thread.c 

OBJS += \
./tool/buf_factory.o \
./tool/cJSON.o \
./tool/key_file.o \
./tool/list.o \
./tool/mailbox.o \
./tool/thread.o 

C_DEPS += \
./tool/buf_factory.d \
./tool/cJSON.d \
./tool/key_file.d \
./tool/list.d \
./tool/mailbox.d \
./tool/thread.d 


# Each subdirectory must supply rules for building sources it contributes
tool/%.o: ../tool/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	mipsel-openwrt-linux-uclibc-gcc -I"/home/work/.workspace/multi_port_convertor" -I"/home/work/.workspace/multi_port_convertor/app" -I"/home/work/.workspace/multi_port_convertor/net" -I"/home/work/.workspace/multi_port_convertor/tool" -I"/home/work/.workspace/multi_port_convertor/uart" -I"/home/work/.workspace/multi_port_convertor/usb" -I/home/work/openwrt_widora-master/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include -I/home/work/openwrt_widora-master/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include/glib-2.0 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


