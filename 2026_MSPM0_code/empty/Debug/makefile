################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := D:/TI/CSS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS

GEN_OPTS__FLAG := @"./device.opt" 
GEN_CMDS__FLAG := -Wl,-l"./device_linker.cmd" 

ORDERED_OBJS += \
"./empty.o" \
"./ti_msp_dl_config.o" \
"./startup_mspm0g350x_ticlang.o" \
"./Module/ICM45686/I2C_communication.o" \
"./Module/ICM45686/IMU.o" \
"./Module/ICM45686/basic_read_reg.o" \
"./Module/ICM45686/inv_imu_driver.o" \
"./Module/ICM45686/inv_imu_transport.o" \
"./Module/K230/K230.o" \
"./Module/OLED/bmp.o" \
"./Module/OLED/oled.o" \
"./Module/OLED/oledfont.o" \
"./Module/bluetooth/bluetooth.o" \
"./Module/command/command.o" \
"./Module/delay/delay.o" \
"./Module/encoder/encoder.o" \
"./Module/key/key.o" \
"./Module/menu/menu.o" \
"./Module/motor/motor.o" \
"./Module/position/position.o" \
"./Module/question_task/question_task.o" \
"./Module/stepper/stepper.o" \
"./Module/track/track.o" \
"./Module/turn/turn.o" \
$(GEN_CMDS__FLAG) \
-Wl,-ldevice.cmd.genlibs \
-Wl,-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include Module/ICM45686/subdir_vars.mk
-include Module/K230/subdir_vars.mk
-include Module/OLED/subdir_vars.mk
-include Module/bluetooth/subdir_vars.mk
-include Module/command/subdir_vars.mk
-include Module/delay/subdir_vars.mk
-include Module/encoder/subdir_vars.mk
-include Module/key/subdir_vars.mk
-include Module/menu/subdir_vars.mk
-include Module/motor/subdir_vars.mk
-include Module/position/subdir_vars.mk
-include Module/question_task/subdir_vars.mk
-include Module/stepper/subdir_vars.mk
-include Module/track/subdir_vars.mk
-include Module/turn/subdir_vars.mk
-include subdir_rules.mk
-include Module/ICM45686/subdir_rules.mk
-include Module/K230/subdir_rules.mk
-include Module/OLED/subdir_rules.mk
-include Module/bluetooth/subdir_rules.mk
-include Module/command/subdir_rules.mk
-include Module/delay/subdir_rules.mk
-include Module/encoder/subdir_rules.mk
-include Module/key/subdir_rules.mk
-include Module/menu/subdir_rules.mk
-include Module/motor/subdir_rules.mk
-include Module/position/subdir_rules.mk
-include Module/question_task/subdir_rules.mk
-include Module/stepper/subdir_rules.mk
-include Module/track/subdir_rules.mk
-include Module/turn/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
empty.out 

EXE_OUTPUTS__QUOTED += \
"empty.out" 


# All Target
all: $(OBJS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "empty.out"

# Tool invocations
empty.out: $(OBJS) $(GEN_CMDS)
	@echo 'Arm Linker - building target: "$@"'
	"D:/TI/CSS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -gdwarf-3 -Wall -Wl,-m"empty.map" -Wl,-i"C:/TI/mspm0_sdk_2_10_00_04/source" -Wl,-i"D:/Desktop/电赛资料汇总/2026年电赛H题代码/empty" -Wl,-i"D:/Desktop/电赛资料汇总/2026年电赛H题代码/empty/Debug/syscfg" -Wl,-i"D:/TI/CSS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/lib" -Wl,--diag_wrap=off -Wl,--display_error_number -Wl,--warn_sections -Wl,--xml_link_info="empty_linkInfo.xml" -Wl,--rom_model -o "empty.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_MISC_FILES__QUOTED)$(GEN_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "empty.o" "ti_msp_dl_config.o" "startup_mspm0g350x_ticlang.o" "Module\ICM45686\I2C_communication.o" "Module\ICM45686\IMU.o" "Module\ICM45686\basic_read_reg.o" "Module\ICM45686\inv_imu_driver.o" "Module\ICM45686\inv_imu_transport.o" "Module\K230\K230.o" "Module\OLED\bmp.o" "Module\OLED\oled.o" "Module\OLED\oledfont.o" "Module\bluetooth\bluetooth.o" "Module\command\command.o" "Module\delay\delay.o" "Module\encoder\encoder.o" "Module\key\key.o" "Module\menu\menu.o" "Module\motor\motor.o" "Module\position\position.o" "Module\question_task\question_task.o" "Module\stepper\stepper.o" "Module\track\track.o" "Module\turn\turn.o" 
	-$(RM) "empty.d" "ti_msp_dl_config.d" "startup_mspm0g350x_ticlang.d" "Module\ICM45686\I2C_communication.d" "Module\ICM45686\IMU.d" "Module\ICM45686\basic_read_reg.d" "Module\ICM45686\inv_imu_driver.d" "Module\ICM45686\inv_imu_transport.d" "Module\K230\K230.d" "Module\OLED\bmp.d" "Module\OLED\oled.d" "Module\OLED\oledfont.d" "Module\bluetooth\bluetooth.d" "Module\command\command.d" "Module\delay\delay.d" "Module\encoder\encoder.d" "Module\key\key.d" "Module\menu\menu.d" "Module\motor\motor.d" "Module\position\position.d" "Module\question_task\question_task.d" "Module\stepper\stepper.d" "Module\track\track.d" "Module\turn\turn.d" 
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

