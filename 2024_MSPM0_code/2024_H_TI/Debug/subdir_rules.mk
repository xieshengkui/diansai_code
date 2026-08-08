################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/TI/CSS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/command" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/turn" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/buzzer" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/question_task" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/track" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/position" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/encoder" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/headfile" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/motor" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/ICM45686" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Debug" -I"C:/TI/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-443152680: ../empty.syscfg
	@echo 'SysConfig - building file: "$<"'
	"C:/TI/sysconfig_1.26.2/sysconfig_cli.bat" -s "C:/TI/mspm0_sdk_2_10_00_04/.metadata/product.json" --script "D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/empty.syscfg" -o "." --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-443152680 ../empty.syscfg
device.opt: build-443152680
device.cmd.genlibs: build-443152680
ti_msp_dl_config.c: build-443152680
ti_msp_dl_config.h: build-443152680
Event.dot: build-443152680

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/TI/CSS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/command" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/turn" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/buzzer" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/question_task" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/track" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/position" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/encoder" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/headfile" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/motor" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/ICM45686" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Debug" -I"C:/TI/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: C:/TI/mspm0_sdk_2_10_00_04/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/TI/CSS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/command" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/turn" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/buzzer" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/question_task" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/track" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/position" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/encoder" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/headfile" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/motor" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Module/ICM45686" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI" -I"D:/Desktop/电赛资料汇总/2024年电赛H题代码/2024年电赛H题MSPM0代码/2024_H_TI/Debug" -I"C:/TI/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


