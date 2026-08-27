CFLAGS += \
  -DSTM32F746xx \
  -DHSE_VALUE=8000000U

SRC_S += \
  $(ST_CMSIS)/Source/Templates/gcc/startup_stm32f746xx.s

# 8MHz HSE comes from the ST-LINK MCO (NUCLEO-F746ZG). Bootloader uses the
# first 64KB (4 x 16KB sectors), application starts at 0x08010000 (sector 4)
LD_FILES = $(PORT_DIR)/ld/STM32F746ZG_FLASH.ld

USB_XFER_SPEED = FS
