
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
* @file    app_filex.h
* @author  MCD Application Team
* @brief   FileX applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_FILEX_H__
#define __APP_FILEX_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"
#include "fx_stm32_sd_driver.h"
#include "fx_stm32_levelx_nor_driver.h"

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
UINT MX_FileX_Init(VOID *memory_ptr);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */
#define DEFAULT_STACK_SIZE               (2 * 1024)
#define DEFAULT_THREAD_PRIO              10
#define DEFAULT_TIME_SLICE               4
#define DEFAULT_PREEMPTION_THRESHOLD     DEFAULT_THREAD_PRIO

#define SD_NOT_PRESENT                  -1
#define SD_PRESENT                       0

/* fx nor_qspi number of bytes per sector */
#ifndef FX_NOR_OSPI_SECTOR_SIZE
  #define FX_NOR_OSPI_SECTOR_SIZE 512
#endif

/* fx sd volume name */
#ifndef FX_SD_VOLUME_NAME
  #define FX_SD_VOLUME_NAME              "STM32_SDIO_DISK"
#endif

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
#ifdef __cplusplus
}
#endif
#endif /* __APP_FILEX_H__ */
