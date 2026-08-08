################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
ICM45686/%.o: ../ICM45686/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/TI/CSS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"D:/Desktop/yuntai/empty/command" -I"D:/Desktop/yuntai/empty/ICM45686" -I"D:/Desktop/yuntai/empty/OLED" -I"D:/Desktop/yuntai/empty/delay" -I"D:/Desktop/yuntai/empty/headfile" -I"D:/Desktop/yuntai/empty/stepper" -I"D:/Desktop/yuntai/empty" -I"D:/Desktop/yuntai/empty/Debug" -I"C:/TI/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"ICM45686/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


