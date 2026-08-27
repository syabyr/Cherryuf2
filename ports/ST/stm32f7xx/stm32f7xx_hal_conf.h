/**
  ******************************************************************************
  * @file    stm32f7xx_hal_conf.h
  * @brief   HAL configuration file for the CherryUF2 stm32f7xx port.
  *          Based on stm32f7xx_hal_conf_template.h, only the modules used
  *          by the bootloader are enabled.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F7xx_HAL_CONF_H
#define __STM32F7xx_HAL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* ########################## Module Selection ############################## */
/**
  * @brief This is the list of modules to be used in the HAL driver
  */
#define HAL_MODULE_ENABLED
// #define HAL_ADC_MODULE_ENABLED
// #define HAL_CAN_MODULE_ENABLED
// #define HAL_CAN_LEGACY_MODULE_ENABLED
// #define HAL_CEC_MODULE_ENABLED
// #define HAL_CRC_MODULE_ENABLED
// #define HAL_CRYP_MODULE_ENABLED
// #define HAL_DAC_MODULE_ENABLED
// #define HAL_DCMI_MODULE_ENABLED
// #define HAL_DMA_MODULE_ENABLED
// #define HAL_DMA2D_MODULE_ENABLED
// #define HAL_ETH_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
// #define HAL_NAND_MODULE_ENABLED
// #define HAL_NOR_MODULE_ENABLED
// #define HAL_PCCARD_MODULE_ENABLED
// #define HAL_SRAM_MODULE_ENABLED
// #define HAL_SDRAM_MODULE_ENABLED
// #define HAL_HASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
// #define HAL_I2C_MODULE_ENABLED
// #define HAL_SMBUS_MODULE_ENABLED
// #define HAL_I2S_MODULE_ENABLED
// #define HAL_IWDG_MODULE_ENABLED
// #define HAL_LPTIM_MODULE_ENABLED
// #define HAL_LTDC_MODULE_ENABLED
// #define HAL_DSI_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
// #define HAL_QSPI_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
// #define HAL_RNG_MODULE_ENABLED
// #define HAL_RTC_MODULE_ENABLED
// #define HAL_SAI_MODULE_ENABLED
// #define HAL_SD_MODULE_ENABLED
// #define HAL_SPDIFRX_MODULE_ENABLED
// #define HAL_SPI_MODULE_ENABLED
// #define HAL_TIM_MODULE_ENABLED
// #define HAL_UART_MODULE_ENABLED
// #define HAL_USART_MODULE_ENABLED
// #define HAL_IRDA_MODULE_ENABLED
// #define HAL_SMARTCARD_MODULE_ENABLED
// #define HAL_WWDG_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
// #define HAL_PCD_MODULE_ENABLED
// #define HAL_HCD_MODULE_ENABLED
// #define HAL_DFSDM_MODULE_ENABLED
// #define HAL_DSI_MODULE_ENABLED
// #define HAL_JPEG_MODULE_ENABLED
// #define HAL_MDIOS_MODULE_ENABLED
// #define HAL_SMBUS_MODULE_ENABLED
// #define HAL_MMC_MODULE_ENABLED

/* ########################## HSE/HSI Values adaptation ##################### */
#if !defined  (HSE_VALUE)
  #define HSE_VALUE              25000000U /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    100U      /*!< Time out for HSE start up, in ms */
#endif /* HSE_STARTUP_TIMEOUT */

/**
  * @brief Internal High Speed oscillator (HSI) value.
  */
#if !defined  (HSI_VALUE)
  #define HSI_VALUE              16000000U /*!< Value of the Internal High Speed oscillator in Hz */
#endif /* HSI_VALUE */

/**
  * @brief Internal Low Speed oscillator (LSI) value.
  */
#if !defined  (LSI_VALUE)
 #define LSI_VALUE               32000U    /*!< LSI Typical Value in Hz */
#endif /* LSI_VALUE */

/**
  * @brief External Low Speed oscillator (LSE) value.
  */
#if !defined  (LSE_VALUE)
 #define LSE_VALUE               32768U    /*!< Value of the External Low Speed oscillator in Hz */
#endif /* LSE_VALUE */

#if !defined  (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    5000U     /*!< Time out for LSE start up, in ms */
#endif /* LSE_STARTUP_TIMEOUT */

/**
  * @brief External clock source for I2S peripheral
  */
#if !defined  (EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE    12288000U /*!< Value of the External audio frequency in Hz */
#endif /* EXTERNAL_CLOCK_VALUE */

/* Tip: To avoid modifying this file each time you need to use different HSE,
   ===  you can define the HSE value in your toolchain compiler preprocessor. */

/* ########################### System Configuration ######################### */
/*!< Tick interrupt priority (HAL_Init not used by the bootloader) */
#define TICK_INT_PRIORITY            ((uint32_t)0x0FU)
#define USE_RTOS                     0U
#define PREFETCH_ENABLE              1U
#define ART_ACCLERATOR_ENABLE        1U

/*!< Uncomment the following line if you need to relocate your vector Table
     in Internal SRAM */
/* #define USER_VECT_TAB_ADDRESS */
#if defined(USER_VECT_TAB_ADDRESS)
/*!< Vector Table base address field.
         This value must be a multiple of 0x200. */
#define VECT_TAB_BASE_ADDRESS   SRAM1_BASE
/*!< Vector Table offset field.
         This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET         0x00000000U
#endif /* USER_VECT_TAB_ADDRESS */

/******************************************************************************/

/* ################## SPI peripheral configuration ########################## */

/* CRC feature: This used is required only if the CRC computation is required */
#define USE_SPI_CRC                   0U

/* Includes ------------------------------------------------------------------*/
/**
  * @brief Include module's header file
  */

#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f7xx_hal_rcc.h"
#endif /* HAL_RCC_MODULE_ENABLED */

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f7xx_hal_gpio.h"
#endif /* HAL_GPIO_MODULE_ENABLED */

#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32f7xx_hal_dma.h"
#endif /* HAL_DMA_MODULE_ENABLED */

#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f7xx_hal_cortex.h"
#endif /* HAL_CORTEX_MODULE_ENABLED */

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f7xx_hal_flash.h"
#endif /* HAL_FLASH_MODULE_ENABLED */

#ifdef HAL_PWR_MODULE_ENABLED
  #include "stm32f7xx_hal_pwr.h"
#endif /* HAL_PWR_MODULE_ENABLED */

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t *file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F7xx_HAL_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
