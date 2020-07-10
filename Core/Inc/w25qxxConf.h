#ifndef _W25QXXCONFIG_H
#define _W25QXXCONFIG_H


#include "main.h"
#include "gpio.h"
#include "spi.h"

#define _W25QXX_USE_FREERTOS          0
#define _W25QXX_DEBUG                 0

#if (_W25QXX_USE_FREERTOS==1)
#define W25qxx_Delay(delay)     osDelay(delay)
#include "cmsis_os.h"
#elif defined(USE_FULL_LL_DRIVER)
#define     W25qxx_Delay(delay)     LL_mDelay(delay)
#else
#define     W25qxx_Delay(delay)     HAL_Delay(delay)
#endif /* W25qxx_Delay */

#ifdef USE_FULL_LL_DRIVER
#define     _W25QXX_SPI             SPI2
#define     W25qxx_Set_CS()         LL_GPIO_SetOutputPin(SPI2_CS_GPIO_Port, SPI2_CS_Pin)
#define     W25qxx_Reset_CS()       LL_GPIO_ResetOutputPin(SPI2_CS_GPIO_Port ,SPI2_CS_Pin)
#elif USE_HAL_DRIVER
#define     _W25QXX_SPI             hspi2
#define     W25qxx_Set_CS()         HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET)
#define     W25qxx_Reset_CS()       HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET)
#endif /* USE_FULL_LL_DRIVER | USE_HAL_DRIVER */

#endif
