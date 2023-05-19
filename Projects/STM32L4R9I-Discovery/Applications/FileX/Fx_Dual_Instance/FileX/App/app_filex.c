
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_filex.c
  * @author  MCD Application Team
  * @brief   FileX applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "app_filex.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/

/* USER CODE BEGIN PD */

#define FILEX_DEFAULT_STACK_SIZE               (2 * 1024)
#define APP_QUEUE_SIZE                          16

/* fx_sd_thread priority */
#define DEFAULT_THREAD_PRIO                     10

/* Msg content*/
typedef enum {
  CARD_STATUS_CHANGED             = 99,
  CARD_STATUS_DISCONNECTED        = 88,
  CARD_STATUS_CONNECTED           = 77
} SD_ConnectionStateTypeDef;

/* fx_sd_thread preemption priority */
#define DEFAULT_PREEMPTION_THRESHOLD      DEFAULT_THREAD_PRIO

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Buffer for FileX FX_MEDIA sector cache.*/
uint32_t fx_sd_media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)];
uint32_t fx_nor_ospi_media_memory[FX_NOR_OSPI_SECTOR_SIZE / sizeof(uint32_t)];

/* Define FileX global data structures.  */
FX_MEDIA        sdio_disk;
FX_MEDIA        nor_ospi_flash_disk;
FX_FILE         fx_file_one;
FX_FILE         fx_file_two;
/* Define ThreadX global data structures.  */
TX_THREAD       fx_thread_one;
TX_THREAD       fx_thread_two;
/* Define ThreadX global data structures.  */
TX_QUEUE        tx_msg_queue;

extern UART_HandleTypeDef huart2;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
VOID fx_thread_one_entry(ULONG thread_input);
VOID fx_thread_two_entry(ULONG thread_input);
static int32_t SD_IsDetected(uint32_t Instance);
void Error_Handler(void);
/* USER CODE END PFP */

/**
  * @brief  Application FileX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_FileX_Init(VOID *memory_ptr)
{
  UINT ret = FX_SUCCESS;

  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN MX_FileX_MEM_POOL */

  /* USER CODE END MX_FileX_MEM_POOL */
  /* USER CODE BEGIN MX_FileX_Init */

  VOID *pointer;

  /* Allocate memory for the 1st thread's stack */
  ret = tx_byte_allocate(byte_pool, &pointer, DEFAULT_STACK_SIZE, TX_NO_WAIT);

  if (ret != FX_SUCCESS)
  {
    /* Failed at allocating memory */
    Error_Handler();
  }

  /* Create the 1st concurrent thread.  */
  tx_thread_create(&fx_thread_one, "fx_thread_one", fx_thread_one_entry, 0, pointer, DEFAULT_STACK_SIZE, DEFAULT_THREAD_PRIO,
                   DEFAULT_PREEMPTION_THRESHOLD, DEFAULT_TIME_SLICE, TX_AUTO_START);

  /* Allocate Memory for the Queue */
  ret = tx_byte_allocate(byte_pool, (VOID **) &pointer, APP_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);

  /*Check memory allocation for the queue*/
  if (ret != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create message queue*/
  ret = tx_queue_create(&tx_msg_queue, "sd_event_queue", 1, pointer, APP_QUEUE_SIZE * sizeof(ULONG));

  /* Check main thread creation */
  if (ret != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Allocate memory for the 2nd concurrent thread's stack */
  ret = tx_byte_allocate(byte_pool, &pointer, DEFAULT_STACK_SIZE, TX_NO_WAIT);

  if (ret != FX_SUCCESS)
  {
    /* Failed at allocating memory */
    Error_Handler();
  }

  /* Create the 2nd concurrent thread */
  tx_thread_create(&fx_thread_two, "fx_thread_two", fx_thread_two_entry, 0, pointer, DEFAULT_STACK_SIZE, DEFAULT_THREAD_PRIO,
                   DEFAULT_PREEMPTION_THRESHOLD, DEFAULT_TIME_SLICE, TX_AUTO_START);

  /* USER CODE END MX_FileX_Init */

  /* Initialize FileX.  */
  fx_system_initialize();

  /* USER CODE BEGIN MX_FileX_Init 1*/

  /* USER CODE END MX_FileX_Init 1*/

  return ret;
}

/* USER CODE BEGIN 1 */

void fx_thread_one_entry(ULONG thread_input)
{
  UINT sd_status = FX_SUCCESS;
  /* USER CODE BEGIN fx_app_thread_entry 0 */
  UINT status;
  ULONG r_msg;
  ULONG s_msg = CARD_STATUS_CHANGED;
  ULONG last_status = CARD_STATUS_DISCONNECTED;
  ULONG bytes_read;
  CHAR read_buffer[32];
  CHAR data[] = "This is FileX working on STM32";

  printf("** Thread One Started ** \n");
  printf("[L4-uSD Card]: FileX uSD write Thread Start.\n");

 if(SD_IsDetected(FX_STM32_SD_INSTANCE) == SD_PRESENT)
  {
    /* SD card is already inserted, place the info into the queue */
    tx_queue_send(&tx_msg_queue, &s_msg, TX_NO_WAIT);
  }
  else
  {
    /* Indicate that SD card is not inserted from start */
    BSP_LED_On(LED_ORANGE);
  }

  /* Infinite Loop */
  for( ;; )
  {

    /* We wait here for a valid SD card insertion event, if it is not inserted already */
    while(1)
    {

      while(tx_queue_receive(&tx_msg_queue, &r_msg, TX_TIMER_TICKS_PER_SECOND / 2) != TX_SUCCESS)
      {
        /* Toggle GREEN LED to indicate idle state after a successful operation */
        if(last_status == CARD_STATUS_CONNECTED)
        {
          BSP_LED_Toggle(LED_GREEN);
        }
      }

      /* check if we received the correct event message */
      if(r_msg == CARD_STATUS_CHANGED)
      {
        /* reset the status */
        r_msg = 0;

        /* for debouncing purpose we wait a bit till it settles down */
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);

        if(SD_IsDetected(FX_STM32_SD_INSTANCE) == SD_PRESENT)
        {
          /* We have a valid SD insertion event, start processing.. */
          /* Update last known status */
          last_status = CARD_STATUS_CONNECTED;
          BSP_LED_Off(LED_ORANGE);/*LED_ORANGE Off*/
          break;
        }
        else
        {
          /* Update last known status */
          last_status = CARD_STATUS_DISCONNECTED;
          BSP_LED_Off(LED_GREEN);  /*LED_GREEN Off*/
          BSP_LED_On(LED_ORANGE); /*LED_ORANGE On*/
        }
      }
    }

  /* USER CODE END fx_app_thread_entry 0 */

  /* Open the SD disk driver */
  sd_status =  fx_media_open(&sdio_disk, FX_SD_VOLUME_NAME, fx_stm32_sd_driver, (VOID *)FX_NULL, (VOID *) fx_sd_media_memory, sizeof(fx_sd_media_memory));

  /* Check the media open sd_status */
  if (sd_status != FX_SUCCESS)
  {
    /* USER CODE BEGIN SD open error */
    while(1);
    /* USER CODE END SD open error */
  }

  /* USER CODE BEGIN fx_app_thread_entry 1 */
/* Create a file called STM32.TXT in the root directory.  */
    status =  fx_file_create(&sdio_disk, "STM32.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {
      /* Check for an already created status. This is expected on the
      second pass of this loop!  */
      if (status != FX_ALREADY_CREATED)
      {
        /* Create error, call error handler.  */
        Error_Handler();
      }
    }

    /* Open the test file.  */
    status =  fx_file_open(&sdio_disk, &fx_file_one, "STM32.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {
      /* Error opening file, call error handler.  */
      Error_Handler();
    }

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&fx_file_one, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {
      /* Error performing file seek, call error handler.  */
      Error_Handler();
    }

    /* Write a string to the test file.  */
    status =  fx_file_write(&fx_file_one, data, sizeof(data));

    /* Check the file write status.  */
    if (status != FX_SUCCESS)
    {
      /* Error writing to a file, call error handler.  */
      Error_Handler();
    }

    /* Close the test file.  */
    status =  fx_file_close(&fx_file_one);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {
      /* Error closing the file, call error handler.  */
      Error_Handler();
    }

    status = fx_media_flush(&sdio_disk);

    /* Check the media flush  status.  */
    if (status != FX_SUCCESS)
    {
      /* Error closing the file, call error handler.  */
      Error_Handler();
    }

    /* Open the test file.  */
    status =  fx_file_open(&sdio_disk, &fx_file_one, "STM32.TXT", FX_OPEN_FOR_READ);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {
      /* Error opening file, call error handler.  */
      Error_Handler();
    }

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&fx_file_one, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {
      /* Error performing file seek, call error handler.  */
      Error_Handler();
    }

    /* Read the first 28 bytes of the test file.  */
    status =  fx_file_read(&fx_file_one, read_buffer, sizeof(data), &bytes_read);

    /* Check the file read status.  */
    if ((status != FX_SUCCESS) || (bytes_read != sizeof(data)))
    {
      /* Error reading file, call error handler.  */
      Error_Handler();
    }

    /* Close the test file.  */
    status =  fx_file_close(&fx_file_one);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {
      /* Error closing the file, call error handler.  */
      Error_Handler();
    }

    /* Close the media.  */
    status =  fx_media_close(&sdio_disk);

    /* Check the media close status.  */
    if (status != FX_SUCCESS)
    {
      /* Error closing the media, call error handler.  */
      Error_Handler();
    }
    printf("[L4-uSD Card]: Thread One succefully executed. \n");

    while(1)
    {
      /* Do nothing wait for the other thread */
      tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
  }

}

VOID fx_thread_two_entry(ULONG thread_input)
{
  UINT nor_ospi_status = FX_SUCCESS;
  /* USER CODE BEGIN fx_app_thread_entry 0 */
  ULONG available_space_pre;
  ULONG available_space_post;
  ULONG bytes_read;
  CHAR read_buffer[32];
  CHAR data[] = "This is FileX working on STM32";

  printf("** Thread Two Started ** \n");
  printf("[L4-NOR Flash]: FileX/LevelX NOR OctoSPI Thread Start.\n");

  /* Print the absolute size of the NOR chip*/
  printf("[L4-NOR Flash]: Total NOR Flash Chip size is: %lu bytes.\n", (unsigned long)LX_STM32_OSPI_FLASH_SIZE);

  /* Format the NOR flash as FAT */
  nor_ospi_status =  fx_media_format(&nor_ospi_flash_disk,
                            fx_stm32_levelx_nor_driver,   // Driver entry
                            (VOID*)LX_NOR_OSPI_DRIVER_ID, // Device info pointer
                            (UCHAR*)fx_nor_ospi_media_memory,         // Media buffer pointer
                            sizeof(fx_nor_ospi_media_memory),         // Media buffer size
                            "nor_ospi_flash_disk",             // Volume Name
                            1,                            // Number of FATs
                            32,                           // Directory Entries
                            0,                            // Hidden sectors
                            LX_STM32_OSPI_FLASH_SIZE/512, // Total sectors
                            512,                          // Sector size
                            8,                            // Sectors per cluster
                            1,                            // Heads
                            1);                           // Sectors per track

  /* Check if the format status */
  if (nor_ospi_status != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Open the QUAD-SPI NOR Flash disk driver.  */
  nor_ospi_status =  fx_media_open(&nor_ospi_flash_disk, "FX_LX_NOR_DISK", fx_stm32_levelx_nor_driver,(VOID*)LX_NOR_OSPI_DRIVER_ID , fx_nor_ospi_media_memory, sizeof(fx_nor_ospi_media_memory));

  /* Check the media open status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Get the available usable space */
  nor_ospi_status =  fx_media_space_available(&nor_ospi_flash_disk, &available_space_pre);

  /* Check the get available state request status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    Error_Handler();
  }

  printf("[L4-NOR Flash]: User available NOR Flash disk space size before file is written: %lu bytes.\n", available_space_pre);

  /* Create a file called STM32.TXT in the root directory.  */
  nor_ospi_status =  fx_file_create(&nor_ospi_flash_disk, "STM32.TXT");

  /* Check the create status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    /* Check for an already created status. This is expected on the
    second pass of this loop!  */
    if (nor_ospi_status != FX_ALREADY_CREATED)
    {
      /* Create error, call error handler.  */
      Error_Handler();
    }
  }

  /* Open the test file.  */
  nor_ospi_status =  fx_file_open(&nor_ospi_flash_disk, &fx_file_two, "STM32.TXT", FX_OPEN_FOR_WRITE);

  /* Check the file open status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    Error_Handler();
  }

  /* Seek to the beginning of the test file.  */
  nor_ospi_status =  fx_file_seek(&fx_file_two, 0);

  /* Check the file seek status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    Error_Handler();
  }

  nor_ospi_status =  fx_file_write(&fx_file_two, data, sizeof(data));

  /* Check the file write status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    /* Error writing to a file, call error handler.  */
    Error_Handler();
  }

  /* Close the test file.  */
  nor_ospi_status =  fx_file_close(&fx_file_two);

  /* Check the file close status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Error_Handler();
  }

  nor_ospi_status = fx_media_flush(&nor_ospi_flash_disk);

  /* Check the media flush  status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Error_Handler();
  }

  /* Open the test file.  */
  nor_ospi_status =  fx_file_open(&nor_ospi_flash_disk, &fx_file_two, "STM32.TXT", FX_OPEN_FOR_READ);

  /* Check the file open status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    Error_Handler();
  }

  /* Seek to the beginning of the test file.  */
  nor_ospi_status =  fx_file_seek(&fx_file_two, 0);

  /* Check the file seek status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    Error_Handler();
  }

  /* Read the first 31 bytes of the test file.  */
  nor_ospi_status =  fx_file_read(&fx_file_two, read_buffer, sizeof(data), &bytes_read);

  /* Check the file read status.  */
  if ((nor_ospi_status != FX_SUCCESS) || (bytes_read != sizeof(data)))
  {
    /* Error reading file, call error handler.  */
    Error_Handler();
  }

  /* Close the test file.  */
  nor_ospi_status =  fx_file_close(&fx_file_two);

  /* Check the file close status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Error_Handler();
  }

  /* Get the available usable space, after the file has been created */
  nor_ospi_status =  fx_media_space_available(&nor_ospi_flash_disk, &available_space_post);

  /* Check the get available state request status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    Error_Handler();
  }

  printf("[L4-NOR Flash]: User available NOR Flash disk space size after file is written: %lu bytes.\n", available_space_post);
  printf("[L4-NOR Flash]: The test file occupied a total of %lu cluster(s) (%u per cluster).\n",
         (available_space_pre - available_space_post) / (nor_ospi_flash_disk.fx_media_bytes_per_sector * nor_ospi_flash_disk.fx_media_sectors_per_cluster),
         nor_ospi_flash_disk.fx_media_bytes_per_sector * nor_ospi_flash_disk.fx_media_sectors_per_cluster);

  /* Close the media.  */
  nor_ospi_status =  fx_media_close(&nor_ospi_flash_disk);

  /* Check the media close status.  */
  if (nor_ospi_status != FX_SUCCESS)
  {
    /* Error closing the media, call error handler.  */
    Error_Handler();
  }

  printf("[L4-NOR Flash]: Thread Two succefully executed. \n\n");

  while(1)
  {    
    BSP_LED_Toggle(LED_GREEN);
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
  }

}

/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  ULONG s_msg = CARD_STATUS_CHANGED;
  if (GPIO_Pin == SD_DETECT_Pin)
  {
    tx_queue_send(&tx_msg_queue, &s_msg, TX_NO_WAIT);
  }
}

/**
 * @brief  Detects if SD card is correctly plugged in the memory slot or not.
 * @param Instance  SD Instance
 * @retval Returns if SD is detected or not
 */
static int32_t SD_IsDetected(uint32_t Instance)
{
  int32_t ret;

  if(Instance >= 1)
  {
    ret = HAL_ERROR;
  }
  else
  {
    /* Check SD card detect pin */
    BSP_IO_ConfigPin(SD_DETECT_PIN, IO_MODE_INPUT_PU);

    if(IO_PIN_RESET == !BSP_IO_ReadPin(SD_DETECT_PIN))
    {
      ret = SD_NOT_PRESENT;
    }
    else
    {
      ret = SD_PRESENT;
    }
  }

  return(int32_t)ret;

}

/* USER CODE END 1 */
