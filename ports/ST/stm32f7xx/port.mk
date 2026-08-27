UF2_FAMILY_ID ?= 0x53b1f5d8
CROSS_COMPILE = arm-none-eabi-

ST_DRIVERS ?= sdk/ST/stm32f7xx_library/stm32f7xx_hal_driver
ST_CMSIS ?= sdk/ST/stm32f7xx_library/cmsis_device_f7
CMSIS_5 ?= sdk/CMSIS_5

# Port Compiler Flags
CFLAGS += \
  -flto \
  -mthumb \
  -mabi=aapcs \
  -mcpu=cortex-m7 \
  -mfloat-abi=hard \
  -mfpu=fpv5-d16 \
  -nostdlib -nostartfiles \
  -DUSE_HAL_DRIVER

# suppress warning caused by vendor mcu driver
CFLAGS += -Wno-error=cast-align -Wno-error=unused-parameter

# default linker file
LD_FILES ?= $(PORT_DIR)/ld/STM32F746ZG_FLASH.ld

# Port source (use PORT_DIR instead of CURRENT_PATH: macOS realpath does not
# support --relative-to, which leaves CURRENT_PATH empty)
SRC_C += \
	$(addprefix $(PORT_DIR)/, $(wildcard *.c)) \
	$(ST_CMSIS)/Source/Templates/system_stm32f7xx.c \
	$(ST_DRIVERS)/Src/stm32f7xx_hal.c \
	$(ST_DRIVERS)/Src/stm32f7xx_hal_cortex.c \
	$(ST_DRIVERS)/Src/stm32f7xx_hal_rcc.c \
	$(ST_DRIVERS)/Src/stm32f7xx_hal_pwr.c \
	$(ST_DRIVERS)/Src/stm32f7xx_hal_pwr_ex.c \
	$(ST_DRIVERS)/Src/stm32f7xx_hal_gpio.c \
	$(ST_DRIVERS)/Src/stm32f7xx_hal_flash.c \
	$(ST_DRIVERS)/Src/stm32f7xx_hal_flash_ex.c

# Port include
INC += \
	$(TOP)/$(CMSIS_5)/CMSIS/Core/Include \
	$(TOP)/$(ST_CMSIS)/Include \
	$(TOP)/$(ST_DRIVERS)/Inc

# Port source for USB
SRC_C += \
	$(CHERRYUSB_DIR)/port/dwc2/usb_dc_dwc2.c \
	$(CHERRYUSB_DIR)/port/dwc2/usb_glue_st.c

# Port include for USB port
INC += $(TOP)/$(CHERRYUSB_DIR)/port/dwc2
