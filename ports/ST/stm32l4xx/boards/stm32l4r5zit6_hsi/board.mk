CFLAGS += \
  -DSTM32L4R5xx

SRC_S += \
  $(ST_CMSIS)/Source/Templates/gcc/startup_stm32l4r5xx.s

# STM32L4+ embeds a Synopsys OTG_FS controller (DWC2 IP) instead of fsdev
USB_IP = dwc2

LD_FILES = $(PORT_DIR)/ld/STM32L4R5ZI_FLASH.ld

USB_XFER_SPEED = FS
