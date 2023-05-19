
## <b>Ux_Device_VIDEO application description</b>

This application provides an example of Azure RTOS USBX stack usage on NUCLEO-L4R5ZI board,
it shows how to develop USB Device VIDEO based application.

The application is designed to emulate an USB VIDEO device, the code provides all required device descriptors framework
and associated Class descriptor report to build a compliant USB VIDEO device.

At the beginning ThreadX call the entry function tx_application_define(), at this stage, all USBx resources
are initialized, the VIDEO Class driver is registered and the application creates 1 threads with the same priorities :

  - app_ux_device_thread_entry (Prio : 10; PreemptionPrio : 10) used to initialize USB_OTG HAL PCD driver and start the device.

#### <b>Expected success behavior</b>

When plugged to PC host, the NUCLEO-L4R5ZI must be properly enumerated as an USB VIDEO device.
During the enumeration phase, device provides host with the requested descriptors (Device, configuration, string).
Those descriptors are used by host driver to identify the device capabilities.
Once the NUCLEO-L4R5ZI USB device successfully completed the enumeration phase.

 1- Open a camera application (Camera, ContaCam or VLC media player) then open the capture device and play the video on the PC host.

 2- To test other stream change number of included file "ux_device_video.c":

    - stream1 FORMAT : MJPEG and Width = 320, Height = 236.
    - stream2 FORMAT : UNCOMPRESSED and Width = 176, Height = 144 and the descriptor shall be updated.

#### <b>Error behaviors</b>

Host PC shows that USB device does not operate as designed (VIDEO Device enumeration failed).

#### <b>Assumptions if any</b>

User is familiar with USB 2.0 "Universal Serial BUS" Specification and VIDEO class Specification.

#### <b>Known limitations</b>

None.

### <b>Notes</b>

#### <b>ThreadX usage hints</b>

 - ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.
 - ThreadX is configured with 100 ticks/sec by default, this should be taken into account when using delays or timeouts at application. It is always possible to reconfigure it in the "tx_user.h", the "TX_TIMER_TICKS_PER_SECOND" define,but this should be reflected in "tx_initialize_low_level.S" file too.
 - ThreadX is disabling all interrupts during kernel start-up to avoid any unexpected behavior, therefore all system related calls (HAL, BSP) should be done either at the beginning of the application or inside the thread entry functions.
 - ThreadX offers the "tx_application_define()" function, that is automatically called by the tx_kernel_enter() API.
   It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...)  but it should not in any way contain a system API call (HAL or BSP).
 - Using dynamic memory allocation requires to apply some changes to the linker file.
   ThreadX needs to pass a pointer to the first free memory location in RAM to the tx_application_define() function,
   using the "first_unused_memory" argument.
   This require changes in the linker files to expose this memory location.
    + For EWARM add the following section into the .icf file:
     ```
     place in RAM_region    { last section FREE_MEM };
     ```
    + For MDK-ARM:
    ```
    either define the RW_IRAM1 region in the ".sct" file
    or modify the line below in "tx_initialize_low_level.S to match the memory region being used
        LDR r1, =|Image$$RW_IRAM1$$ZI$$Limit|
    ```
    + For STM32CubeIDE add the following section into the .ld file:
    ```
    ._threadx_heap :
      {
         . = ALIGN(8);
         __RAM_segment_used_end__ = .;
         . = . + 64K;
         . = ALIGN(8);
       } >RAM_D1 AT> RAM_D1
    ```

       The simplest way to provide memory for ThreadX is to define a new section, see ._threadx_heap above.
       In the example above the ThreadX heap size is set to 64KBytes.
       The ._threadx_heap must be located between the .bss and the ._user_heap_stack sections in the linker script.
       Caution: Make sure that ThreadX does not need more than the provided heap memory (64KBytes in this example).
       Read more in STM32CubeIDE User Guide, chapter: "Linker script".

    + The "tx_initialize_low_level.S" should be also modified to enable the "USE_DYNAMIC_MEMORY_ALLOCATION" flag.

### <b>Keywords</b>

RTOS, ThreadX, USBXDevice, USB_OTG, Full Speed, Video

### <b>Hardware and Software environment</b>

  - This example runs on STM32L4R5xx devices.
  - This example has been tested with STMicroelectronics NUCLEO-L4R5ZI boards Revision MB1312 A-01 and can be easily tailored
    to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application
