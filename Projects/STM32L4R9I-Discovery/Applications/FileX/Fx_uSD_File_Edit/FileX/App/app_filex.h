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
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32l4r9i_discovery.h"
#include "stm32l4r9i_discovery_io.h"
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
int32_t SD_IsDetected(uint32_t Instance);
uint8_t SD_ITConfig(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SD_DETECT_PIN_BSP                GPIO_PIN_5

#define SD_PRESENT                       1UL
#define SD_NOT_PRESENT                   0UL
#define DEFAULT_STACK_SIZE               (2 * 1024)
#define DEFAULT_QUEUE_LENGTH             16
/* fx_sd_thread priority */
#define DEFAULT_THREAD_PRIO              10
/* fx_sd_thread preemption priority */
#define DEFAULT_PREEMPTION_THRESHOLD     DEFAULT_THREAD_PRIO
/* Message content*/
typedef enum {
CARD_STATUS_CHANGED             = 99,
CARD_STATUS_DISCONNECTED        = 88,
CARD_STATUS_CONNECTED           = 77
} SD_ConnectionStateTypeDef;
/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
#ifdef __cplusplus
}
#endif
#endif /* __APP_FILEX_H__ */
