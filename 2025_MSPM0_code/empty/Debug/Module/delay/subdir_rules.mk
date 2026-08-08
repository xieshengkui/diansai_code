################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Module/delay/%.o: ../Module/delay/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/TI/CSS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"D:/Desktop/电赛资料汇总/2025年电赛E题代码/代码1/empty/Module/OLED" -I"D:/Desktop/电赛资料汇总/2025年电赛E题代码/代码1/empty/Module/command" -I"D:/Desktop/电赛资料汇总/2025年电赛E题代码/代码1/empty/Module/stepper" -I"D:/Desktop/电赛资料汇总/2025年电赛E题代码/代码1/empty/Module/headfile" -I"D:/Desktop/电赛资料汇总/2025年电赛E题代码/代码1/empty/Module/delay" -I"D:/Desktop/电赛资料汇总/2025年电赛E题代码/代码1/empty" -I"D:/Desktop/电赛资料汇总/2025年电赛E题代码/代码1/empty/Debug" -I"C:/TI/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"Module/delay/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


