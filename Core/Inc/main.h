/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_exti.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_utils.h"
#include "stm32g0xx_ll_pwr.h"
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_spi.h"
#include "stm32g0xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_PWR_Pin LL_GPIO_PIN_11
#define LED_PWR_GPIO_Port GPIOC
#define LED_ERR_Pin LL_GPIO_PIN_12
#define LED_ERR_GPIO_Port GPIOC
#define BUZER_Pin LL_GPIO_PIN_3
#define BUZER_GPIO_Port GPIOC
#define BTN_SERVICE_Pin LL_GPIO_PIN_7
#define BTN_SERVICE_GPIO_Port GPIOC
#define SPI2_CS_Pin LL_GPIO_PIN_9
#define SPI2_CS_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define TOGGLE_DO(g,p)      LL_GPIO_TogglePin(g,p)
#define RESET_DO(g,p)       LL_GPIO_ResetOutputPin(g,p)
#define SET_DO(g,p)         LL_GPIO_SetOutputPin(g,p)

#define LED_TOGGLE(g,p)     TOGGLE_DO(g,p)
#define LED_OFF(g,p)        RESET_DO(g,p)
#define LED_ON(g,p)         SET_DO(g,p)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
