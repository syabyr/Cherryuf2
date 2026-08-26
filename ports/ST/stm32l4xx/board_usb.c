/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Zhaqian
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "board_api.h"

//--------------------------------------------------------------------+
// CherryUSB LLD
//--------------------------------------------------------------------+
static struct usbd_interface intf0;

#if defined(USB_OTG_FS)

// DWC2 FIFO allocation for MSC (total 304 words of the 1.25KB DFIFO):
// RX = (5 * control eps + 8) + (512 / 4 + 1) + (2 * out eps) + 1 rounded up,
// TX0 = EP0 IN 64 bytes, TX1 = MSC IN 512 bytes
uint16_t usbd_get_dwc2_rxfifo_conf(uint8_t busid) {
    (void)busid;
    return 160;
}

uint16_t usbd_get_dwc2_txfifo_conf(uint8_t busid, uint8_t fifoid) {
    (void)busid;
    if (fifoid == 0) {
        return 64 / 4;
    }
    if (fifoid == 1) {
        return 512 / 4;
    }
    return 0;
}

__attribute__((weak)) void board_uf2boot_init(void) {
    // dwc2 driver relies on HAL_Delay (via usbd_dwc2_delay_ms) during init,
    // start SysTick so the HAL timebase ticks
    SysTick_Config(SystemCoreClock / 1000U);

    usbd_desc_register(BOOTUF2_BUS_ID_FS, &bootuf2_descriptor);
    usbd_add_interface(BOOTUF2_BUS_ID_FS, usbd_msc_init_intf(BOOTUF2_BUS_ID_FS, &intf0, BOOTUF2_OUT_EP, BOOTUF2_IN_EP));
    usbd_initialize(BOOTUF2_BUS_ID_FS, USB_DEVICE_SPEED_FS, USB_OTG_FS_PERIPH_BASE, usbd_event_handler);
}

__attribute__((weak)) void usb_dc_low_level_init(uint8_t busid) {
    (void)busid;
    HAL_PWREx_EnableVddUSB();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin              = (GPIO_PIN_11 | GPIO_PIN_12);
    GPIO_InitStruct.Mode             = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull             = GPIO_NOPULL;
    GPIO_InitStruct.Speed            = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate        = GPIO_AF10_OTG_FS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
}

//--------------------------------------------------------------------+
// IRQ Handler
//--------------------------------------------------------------------+
void OTG_FS_IRQHandler(void) {
    extern void USBD_IRQHandler(uint8_t busid);
    USBD_IRQHandler(BOOTUF2_BUS_ID_FS);
}

#else

__attribute__((weak)) void board_uf2boot_init(void) {
    usbd_desc_register(BOOTUF2_BUS_ID_FS, &bootuf2_descriptor);
    usbd_add_interface(BOOTUF2_BUS_ID_FS, usbd_msc_init_intf(BOOTUF2_BUS_ID_FS, &intf0, BOOTUF2_OUT_EP, BOOTUF2_IN_EP));
    usbd_initialize(BOOTUF2_BUS_ID_FS, USB_DEVICE_SPEED_FS, USB_BASE, usbd_event_handler);
}

__attribute__((weak)) void usb_dc_low_level_init(uint8_t busid) {
    (void)busid;
    HAL_PWREx_EnableVddUSB();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin              = (GPIO_PIN_11 | GPIO_PIN_12);
    GPIO_InitStruct.Mode             = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull             = GPIO_NOPULL;
    GPIO_InitStruct.Speed            = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate        = GPIO_AF10_USB_FS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    __HAL_RCC_USB_CLK_ENABLE();
    HAL_NVIC_SetPriority(USB_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_IRQn);
}

//--------------------------------------------------------------------+
// IRQ Handler
//--------------------------------------------------------------------+
void USB_IRQHandler(void) {
    extern void USBD_IRQHandler(uint8_t busid);
    USBD_IRQHandler(BOOTUF2_BUS_ID_FS);
}

#endif
