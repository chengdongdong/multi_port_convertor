################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include usb/subdir.mk
-include uart/subdir.mk
-include tool/subdir.mk
-include net/subdir.mk
-include app/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: multi_port_convertor

# Tool invocations
multi_port_convertor: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	mipsel-openwrt-linux-uclibc-gcc -L/home/work/openwrt_widora-master/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib -o "multi_port_convertor" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) multi_port_convertor
	-@echo ' '

post-build:
	-cp multi_port_convertor /home/work/targetfs
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
