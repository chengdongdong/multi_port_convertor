################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../net/http_protocol.c \
../net/net.c \
../net/net_protocol.c \
../net/tcp.c \
../net/udp.c \
../net/wifi.c 

OBJS += \
./net/http_protocol.o \
./net/net.o \
./net/net_protocol.o \
./net/tcp.o \
./net/udp.o \
./net/wifi.o 

C_DEPS += \
./net/http_protocol.d \
./net/net.d \
./net/net_protocol.d \
./net/tcp.d \
./net/udp.d \
./net/wifi.d 


# Each subdirectory must supply rules for building sources it contributes
net/%.o: ../net/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	mipsel-openwrt-linux-uclibc-gcc -I"/home/work/.workspace/multi_port_convertor" -I"/home/work/.workspace/multi_port_convertor/app" -I"/home/work/.workspace/multi_port_convertor/net" -I"/home/work/.workspace/multi_port_convertor/tool" -I"/home/work/.workspace/multi_port_convertor/uart" -I"/home/work/.workspace/multi_port_convertor/usb" -I/home/work/openwrt_widora-master/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include -I/home/work/openwrt_widora-master/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include/glib-2.0 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


