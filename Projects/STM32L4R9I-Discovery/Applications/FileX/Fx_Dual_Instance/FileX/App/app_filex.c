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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* Buffer for FileX FX_MEDIA sector cache. this should be 32-Bytes
aligned to avoid cache maintenance issues */
uint32_t media_memory_sd[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)];
uint32_t media_memory_nor[LX_STM32_OSPI_SECTOR_SIZE / sizeof(uint32_t)];

/* Define FileX global data structures.  */
FX_MEDIA        sdio_disk;
FX_MEDIA        nor_flash_disk;
FX_FILE         fx_file_one;
FX_FILE         fx_file_two;
/* Define ThreadX global data structures.  */
TX_THREAD       fx_thread_one;
TX_THREAD       fx_thread_two;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
VOID fx_thread_one_entry(ULONG thread_input);
VOID fx_thread_two_entry(ULONG thread_input);
VOID Tx_Error_Handler(int th_id);
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

  /* Initialize FileX.  */
  fx_system_initialize();

  /* USER CODE END MX_FileX_Init */
  return ret;
}

/* USER CODE BEGIN 1 */
void fx_thread_one_entry(ULONG thread_input)
{

  UINT status;
  ULONG bytes_read;
  CHAR read_buffer[32];
  CHAR data[] = "This is FileX working on STM32";

  printf("** Thread One Started ** \n");
  printf("[L4-uSD Card]: FileX uSD write Thread Start.\n");

  BSP_IO_ConfigPin(SD_DETECT_PIN, IO_MODE_INPUT_PU);

  if(!SD_IS_DETECT)
  {
    printf("[L4-uSD Card]: !!! This thread won't start processing till an SD card is inserted !!!\n");
  }

  /* This thread won't start till SD card is inserted */
  while(!SD_IS_DETECT)
  {
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
  }

  /* Open the SD disk driver.  */
  status =  fx_media_open(&sdio_disk, "STM32_SDIO_DISK", fx_stm32_sd_driver, 0,(VOID *) media_memory_sd, sizeof(media_memory_sd));

  /* Check the media open status.  */
  if (status != FX_SUCCESS)
  {
    Tx_Error_Handler(1);
  }

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
      Tx_Error_Handler(1);
    }
  }

  /* Open the test file.  */
  status =  fx_file_open(&sdio_disk, &fx_file_one, "STM32.TXT", FX_OPEN_FOR_WRITE);

  /* Check the file open status.  */
  if (status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    Tx_Error_Handler(1);
  }

  /* Seek to the beginning of the test file.  */
  status =  fx_file_seek(&fx_file_one, 0);

  /* Check the file seek status.  */
  if (status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    Tx_Error_Handler(1);
  }

  /* Write a string to the test file.  */
  status =  fx_file_write(&fx_file_one, data, sizeof(data));

  /* Check the file write status.  */
  if (status != FX_SUCCESS)
  {
    /* Error writing to a file, call error handler.  */
    Tx_Error_Handler(1);
  }

  /* Close the test file.  */
  status =  fx_file_close(&fx_file_one);

  /* Check the file close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Tx_Error_Handler(1);
  }

  status = fx_media_flush(&sdio_disk);

  /* Check the media flush  status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Tx_Error_Handler(1);
  }

  /* Open the test file.  */
  status =  fx_file_open(&sdio_disk, &fx_file_one, "STM32.TXT", FX_OPEN_FOR_READ);

  /* Check the file open status.  */
  if (status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    Tx_Error_Handler(1);
  }

  /* Seek to the beginning of the test file.  */
  status =  fx_file_seek(&fx_file_one, 0);

  /* Check the file seek status.  */
  if (status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    Tx_Error_Handler(1);
  }

  /* Read the first 28 bytes of the test file.  */
  status =  fx_file_read(&fx_file_one, read_buffer, sizeof(data), &bytes_read);

  /* Check the file read status.  */
  if ((status != FX_SUCCESS) || (bytes_read != sizeof(data)))
  {
    /* Error reading file, call error handler.  */
    Tx_Error_Handler(1);
  }

  /* Close the test file.  */
  status =  fx_file_close(&fx_file_one);

  /* Check the file close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Tx_Error_Handler(1);
  }

  /* Close the media.  */
  status =  fx_media_close(&sdio_disk);

  /* Check the media close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the media, call error handler.  */
    Tx_Error_Handler(1);
  }

  printf("[L4-uSD Card]: Thread One succefully exucuted. \n");

  while(1)
  {
    /* Do nothing wait for the other thread */
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
  }
}

VOID fx_thread_two_entry(ULONG thread_input)
{

  UINT status;
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
  status =  fx_media_format(&nor_flash_disk,
                            fx_stm32_levelx_nor_driver,   // Driver entry
                            (VOID*)LX_NOR_OSPI_DRIVER_ID, // Device info pointer
                            (UCHAR*)media_memory_nor,         // Media buffer pointer
                            sizeof(media_memory_nor),         // Media buffer size
                            "NOR_FLASH_DISK",             // Volume Name
                            1,                            // Number of FATs
                            32,                           // Directory Entries
                            0,                            // Hidden sectors
                            LX_STM32_OSPI_FLASH_SIZE/512, // Total sectors
                            512,                          // Sector size
                            8,                            // Sectors per cluster
                            1,                            // Heads
                            1);                           // Sectors per track

  /* Check if the format status */
  if (status != FX_SUCCESS)
  {
    Tx_Error_Handler(2);
  }

  /* Open the OctoSPI NOR Flash disk driver.  */
  status =  fx_media_open(&nor_flash_disk, "FX_LX_NOR_DISK", fx_stm32_levelx_nor_driver,(VOID*)LX_NOR_OSPI_DRIVER_ID , media_memory_nor, sizeof(media_memory_nor));

  /* Check the media open status.  */
  if (status != FX_SUCCESS)
  {
    Tx_Error_Handler(2);
  }

  /* Get the available usable space */
  status =  fx_media_space_available(&nor_flash_disk, &available_space_pre);

  /* Check the get available state request status.  */
  if (status != FX_SUCCESS)
  {
    Tx_Error_Handler(2);
  }

  printf("[L4-NOR Flash]: User available NOR Flash disk space size before file is written: %lu bytes.\n", available_space_pre);

  /* Create a file called STM32.TXT in the root directory.  */
  status =  fx_file_create(&nor_flash_disk, "STM32.TXT");

  /* Check the create status.  */
  if (status != FX_SUCCESS)
  {
    /* Check for an already created status. This is expected on the
    second pass of this loop!  */
    if (status != FX_ALREADY_CREATED)
    {
      /* Create error, call error handler.  */
      Tx_Error_Handler(2);
    }
  }

  /* Open the test file.  */
  status =  fx_file_open(&nor_flash_disk, &fx_file_two, "STM32.TXT", FX_OPEN_FOR_WRITE);

  /* Check the file open status.  */
  if (status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    Tx_Error_Handler(2);
  }

  /* Seek to the beginning of the test file.  */
  status =  fx_file_seek(&fx_file_two, 0);

  /* Check the file seek status.  */
  if (status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    Tx_Error_Handler(2);
  }

  status =  fx_file_write(&fx_file_two, data, sizeof(data));

  /* Check the file write status.  */
  if (status != FX_SUCCESS)
  {
    /* Error writing to a file, call error handler.  */
    Tx_Error_Handler(2);
  }

  /* Close the test file.  */
  status =  fx_file_close(&fx_file_two);

  /* Check the file close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Tx_Error_Handler(2);
  }

  status = fx_media_flush(&nor_flash_disk);

  /* Check the media flush  status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Tx_Error_Handler(2);
  }

  /* Open the test file.  */
  status =  fx_file_open(&nor_flash_disk, &fx_file_two, "STM32.TXT", FX_OPEN_FOR_READ);

  /* Check the file open status.  */
  if (status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    Tx_Error_Handler(2);
  }

  /* Seek to the beginning of the test file.  */
  status =  fx_file_seek(&fx_file_two, 0);

  /* Check the file seek status.  */
  if (status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    Tx_Error_Handler(2);
  }

  /* Read the first 31 bytes of the test file.  */
  status =  fx_file_read(&fx_file_two, read_buffer, sizeof(data), &bytes_read);

  /* Check the file read status.  */
  if ((status != FX_SUCCESS) || (bytes_read != sizeof(data)))
  {
    /* Error reading file, call error handler.  */
    Tx_Error_Handler(2);
  }

  /* Close the test file.  */
  status =  fx_file_close(&fx_file_two);

  /* Check the file close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Tx_Error_Handler(2);
  }

  /* Get the available usable space, after the file has been created */
  status =  fx_media_space_available(&nor_flash_disk, &available_space_post);

  /* Check the get available state request status.  */
  if (status != FX_SUCCESS)
  {
    Tx_Error_Handler(2);
  }

  printf("[L4-NOR Flash]: User available NOR Flash disk space size after file is written: %lu bytes.\n", available_space_post);
  printf("[L4-NOR Flash]: The test file occupied a total of %lu cluster(s) (%u per cluster).\n",
         (available_space_pre - available_space_post) / (nor_flash_disk.fx_media_bytes_per_sector * nor_flash_disk.fx_media_sectors_per_cluster),
         nor_flash_disk.fx_media_bytes_per_sector * nor_flash_disk.fx_media_sectors_per_cluster);

  /* Close the media.  */
  status =  fx_media_close(&nor_flash_disk);

  /* Check the media close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the media, call error handler.  */
    Tx_Error_Handler(2);
  }

  printf("[L4-NOR Flash]: Thread Two succefully exucuted. \n\n");

  while(1)
  {
    BSP_LED_Toggle(LED_GREEN);
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
  }
}

VOID Tx_Error_Handler(int th_id)
{
  /* Error occurred in one of the threads: */
  /* Suspend the other thread and call the Error_Handler */

  tx_thread_terminate((th_id == 1)? &fx_thread_two : &fx_thread_one);
  printf("Error occurred at thread #%i\n", th_id);
  Error_Handler();
}

/* USER CODE END 1 */
