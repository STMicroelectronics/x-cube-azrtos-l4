/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_dfu_media.h
  * @author  MCD Application Team
  * @brief   USBX Device DFU interface header file
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
#ifndef __UX_DEVICE_DFU_MEDIA_H__
#define __UX_DEVICE_DFU_MEDIA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ux_device_class_dfu.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct
{
  ULONG wblock_num;
  ULONG wlength;
  UCHAR *data_ptr;
} ux_dfu_downloadInfotypeDef;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define ADDR_FLASH_PAGE_0     ((uint32_t)0x08000000) /* Base @ of Page 0, 4 Kbytes */
#define ADDR_FLASH_PAGE_1     ((uint32_t)0x08001000) /* Base @ of Page 2, 4 Kbytes */
#define ADDR_FLASH_PAGE_2     ((uint32_t)0x08002000) /* Base @ of Page 4, 4 Kbytes */
#define ADDR_FLASH_PAGE_3     ((uint32_t)0x08003000) /* Base @ of Page 6, 4 Kbytes */
#define ADDR_FLASH_PAGE_4     ((uint32_t)0x08004000) /* Base @ of Page 8, 4 Kbytes */
#define ADDR_FLASH_PAGE_12    ((uint32_t)0x0800C000) /* Base @ of Page 24, 4 Kbytes */
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/* Special Commands with Download Request */
#define DFU_CMD_GETCOMMANDS             0x00U
#define DFU_CMD_SETADDRESSPOINTER       0x21U
#define DFU_CMD_ERASE                   0x41U
#define DFU_CMD_READ_UNPROTECT          0x92U
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
 void DFU_Init(void *dfu);
 void DFU_DeInit(void *dfu);
 UINT DFU_GetStatus(void *dfu,ULONG *media_status);
 UINT DFU_Notify(VOID *dfu, ULONG notification);
 UINT DFU_Read(VOID *dfu, ULONG block_number, UCHAR * data_pointer, ULONG length, ULONG *media_status);
 UINT DFU_Write(VOID *dfu, ULONG block_number, UCHAR * data_pointer, ULONG length, ULONG *media_status);
 UINT DFU_Leave(VOID *dfu, UX_SLAVE_TRANSFER *transfer);
 UINT DFU_Device_ConnectionCallback(ULONG Device_State);
 void usbx_dfu_download_thread_entry(ULONG arg);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif  /* __UX_DEVICE_DFU_MEDIA_H__ */
