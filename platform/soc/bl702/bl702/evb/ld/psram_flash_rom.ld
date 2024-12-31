OUTPUT_ARCH( "riscv" )

ENTRY( bl702_start )

__EM_SIZE = DEFINED(ble_controller_init) ? (DEFINED(em_16k_init) ? 16K : 8K) : 0K;
__CACHE_SIZE = DEFINED(__CACHE_SIZE) ? __CACHE_SIZE : 16K;
__stack_size = DEFINED(__stack_size) ? __stack_size : 2K;
__sha_ocram_size = DEFINED(__sha_ocram_size) ? __sha_ocram_size : 128;

MEMORY
{
  flash  (rxai!w) : ORIGIN = 0x23000000, LENGTH = (2M)
  tcm_ocram (wxa) : ORIGIN = 0x42010000 + __CACHE_SIZE, LENGTH = (64K - __CACHE_SIZE + 64K - __EM_SIZE - __stack_size - __sha_ocram_size)
  sys_stack (wxa) : ORIGIN = 0x42030000 - __EM_SIZE - __stack_size - __sha_ocram_size, LENGTH = __stack_size
  sha_ocram (wxa) : ORIGIN = 0x42030000 - __EM_SIZE - __sha_ocram_size, LENGTH = __sha_ocram_size
  hbnram    (wxa) : ORIGIN = 0x40010000, LENGTH = (4K)
  psram     (wxa) : ORIGIN = 0x26000000, LENGTH = (2M)
}

SECTIONS
{
  BOOT2_PT_ADDR = 0x4202DC00;

  .init :
  {
    KEEP (*(SORT_NONE(.init)))
  } >flash

  .text :
  {
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    *(.text .text.*)
    *(.gnu.linkonce.t.*)
  } >flash

  .rodata :
  {
    *(.rdata)
    *(.rodata .rodata.*)
    *(.sdata2.*)
    *(.gcc_except_table.*)

    /* camera driver */
    . = ALIGN(4);
    _bl_driver_camera_start = .;
    KEEP(*(.camera_desc))
    *(.camera_desc)
    _bl_driver_camera_end = .;

    /* static cli cmds */
    . = ALIGN(4);
    _bl_static_cli_cmds_start = .;
    KEEP(*(.static_cli_cmds))
    *(.static_cli_cmds)
    _bl_static_cli_cmds_end = .;

    /* static blog code1 */
    . = ALIGN(4);
    _bl_static_blogcomponent_code_start = .;
    KEEP(SORT(*)(.static_blogcomponent_code*))
    *(.static_blogcomponent_code*)
    _bl_static_blogcomponent_code_end = .;

    /* static blog code2 */
    . = ALIGN(4);
    _bl_static_blogfile_code_start = .;
    KEEP(SORT(*)(.static_blogfile_code*))
    *(.static_blogfile_code*)
    _bl_static_blogfile_code_end = .;

    /* static blog code3 */
    . = ALIGN(4);
    _bl_static_blogpri_code_start = .;
    KEEP(SORT(*)(.static_blogpri_code*))
    *(.static_blogpri_code*)
    _bl_static_blogpri_code_end = .;

    /* section information for components's version */
    . = ALIGN(4);
    _version_info_section_start = .;
    KEEP(*(.version.*))
    _version_info_section_end = .;

    *(.gnu.linkonce.r.*)
  } >flash

  .init_array :
  {
    __init_array_start = .;
    KEEP(*(.init_array))
    __init_array_end = .;
  } >flash

  .bleromro :
  {
    KEEP(*(.bleromro))
  } >flash

  .bleromrw (NOLOAD) :
  {
    KEEP(*(.bleromrw))
  } >tcm_ocram

  .rtosromrw (NOLOAD) : ALIGN(16)
  {
    /*always put freertos under global_pointer with the following order. No change!*/
    . = . + 0x20; /* reserve (0x800-0x7E0) bytes, make sure scope of __global_pointer start from rtosromrw, DO NOT cover bleromrw */
    PROVIDE( __global_pointer_head$ =  . );
    PROVIDE( __global_pointer$ = . + 0x7E0 );
    . = . + 0x4A0;
  } >tcm_ocram

  .hbn : ALIGN(4)
  {
    PROVIDE( _hbn_load = LOADADDR(.hbn) );
    PROVIDE( _hbn_run = ADDR(.hbn) );
    PROVIDE( _hbn_run_end = ADDR(.hbn) + SIZEOF(.hbn) );

    /* put bl702_rf_private.o in hbnram to avoid ocram conflict during rf calibration */
    *libbl702_rf.a:bl702_rf_private.o(.retention .retention_noinit)
    *libbl702_rf.a:bl702_rf_private.o(.sdata.* .srodata.* .sbss.* .bss.* COMMON)
    PROVIDE ( _rf_data_end = . );

    /* tcm code that should be retentive after hbn wakeup */
    *libbl702_std.a:bl702_romapi.o(.tcm_code.* .sclock_rlt_code.*)
    *libbl702_std.a:bl702_aon.o(.tcm_code.* .sclock_rlt_code.*)
    *libbl702_std.a:bl702_glb.o(.tcm_code.* .sclock_rlt_code.*)
    *libbl702_std.a:bl702_hbn.o(.tcm_code.* .sclock_rlt_code.*)
    PROVIDE ( _hbn_restore_tcm_run_end = . );

    /* data that should be retentive after hbn wakeup */
    *liblmac154.a:*(.sdata.* .srodata.* .sbss.* .bss.* COMMON)
    *libbl702_std.a:*(.sdata.* .srodata.* .sbss.* .bss.* COMMON)
    *libhosal.a:bl_gpio.o(.sdata.* .srodata.* .sbss.* .bss.* COMMON)
    *libhosal.a:hosal_gpio.o(.sdata.* .srodata.* .sbss.* .bss.* COMMON)
    *libhosal.a:bl_uart.o(.sdata.* .srodata.* .sbss.* .bss.* COMMON)
    *libhosal.a:hosal_uart.o(.sdata.* .srodata.* .sbss.* .bss.* COMMON)
    PROVIDE ( _hbn_restore_data_run_end = . );

    *(.hbn_code.*)
    *(.hbn_data)
    *(.retention)
  } >hbnram AT >flash

  .hbn_noinit (NOLOAD) :
  {
    *(.hbn_noinit)
    *(.retention_noinit)
  } >hbnram

  .tcmdata (NOLOAD) :
  {
    *(.tcm_data)
  } >tcm_ocram

  .tcmcode : ALIGN(4)
  {
    PROVIDE( _tcm_load = LOADADDR(.tcmcode) );
    PROVIDE( _tcm_run = ADDR(.tcmcode) );
    PROVIDE( _tcm_run_end = ADDR(.tcmcode) + SIZEOF(.tcmcode) );

    *(.tcm_code.*)
    *(.tcm_const.*)
    *(.sclock_rlt_code.*)
    *(.sclock_rlt_const.*)
  } >tcm_ocram AT >flash

  .data : ALIGN(4)
  {
    PROVIDE( _data_load = LOADADDR(.data) );
    PROVIDE( _data_run = ADDR(.data) );
    PROVIDE( _data_run_end = ADDR(.data) + SIZEOF(.data) );

    *(.pds_code.*)
    *(.data .data.*)
    *(.gnu.linkonce.d.*)

    . = ALIGN(8);
    *(.sdata .sdata.*)
    *(.gnu.linkonce.s.*)

    . = ALIGN(8);
    *(.srodata.cst16)
    *(.srodata.cst8)
    *(.srodata.cst4)
    *(.srodata.cst2)
    *(.srodata .srodata.*)
    
    . = ALIGN(8);
    *(._k_queue.static.*)
    *(._k_sem.static.*)
    *(._k_mutex.static.*)
    _bt_gatt_service_static_list_start = .;
    KEEP(*(SORT_BY_NAME("._bt_gatt_service_static.static.*")))
    _bt_gatt_service_static_list_end = .;
    _bt_l2cap_fixed_chan_list_start = .;
    KEEP(*(SORT_BY_NAME("._bt_l2cap_fixed_chan.static.*")))
    _bt_l2cap_fixed_chan_list_end = .;
  } >tcm_ocram AT >flash

  .boot2 (NOLOAD) :
  {
    PROVIDE ( __boot2_pt_addr_start = . );
    *(.bss.boot2_partition_table)
    PROVIDE ( __boot2_pt_addr_end   = . );

    PROVIDE ( __boot2_flashCfg_start = . );
    *(.bss.boot2_flashCfg)
    PROVIDE ( __boot2_flashCfg_end = . );
  } >tcm_ocram

  .bss (NOLOAD) :
  {
    PROVIDE( __bss_start = ADDR(.bss) );
    PROVIDE( __bss_end = ADDR(.bss) + SIZEOF(.bss) );

    *(.sbss*)
    *(.gnu.linkonce.sb.*)
    *(.bss .bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
  } >tcm_ocram

  . = ALIGN(8);
  PROVIDE( _heap_start = . );
  PROVIDE( _heap_size = ORIGIN(tcm_ocram) + LENGTH(tcm_ocram) - _heap_start );

  PROVIDE( _heap2_start = 0 );
  PROVIDE( _heap2_size = 0 );

  .stack (NOLOAD) : ALIGN(8)
  {
    PROVIDE( _sp_base = . );
    . = . + __stack_size;
    PROVIDE( _sp_main = . );
    __freertos_irq_stack_top = .;
  } >sys_stack

  .sha_ocram (NOLOAD) :
  {
    *(.sha_ocram*)
  } >sha_ocram

  .psram (NOLOAD) :
  {
    . = ALIGN(16);
    PROVIDE( _psram_start = . );
    *(.wifi_ram*)
    . = ALIGN(16);
    PROVIDE( _psram_end = . );
  } >psram

  . = ALIGN(8);
  PROVIDE( _heap3_start = . );
  PROVIDE( _heap3_size = ORIGIN(psram) + LENGTH(psram) - _heap3_start );


  /* camera driver */
  PROVIDE( _ld_bl_driver_camera_start = _bl_driver_camera_start );
  PROVIDE( _ld_bl_driver_camera_end   = _bl_driver_camera_end );

  /* cli */
  PROVIDE( _ld_bl_static_cli_cmds_start = _bl_static_cli_cmds_start );
  PROVIDE( _ld_bl_static_cli_cmds_end   = _bl_static_cli_cmds_end );

  /* blog */
  PROVIDE( _ld_bl_static_blogcomponent_code_start = _bl_static_blogcomponent_code_start );
  PROVIDE( _ld_bl_static_blogcomponent_code_end   = _bl_static_blogcomponent_code_end );
  PROVIDE( _ld_bl_static_blogfile_code_start = _bl_static_blogfile_code_start );
  PROVIDE( _ld_bl_static_blogfile_code_end   = _bl_static_blogfile_code_end );
  PROVIDE( _ld_bl_static_blogpri_code_start = _bl_static_blogpri_code_start );
  PROVIDE( _ld_bl_static_blogpri_code_end   = _bl_static_blogpri_code_end );

  /* ram information */
  PROVIDE( _ld_ram_size0 = LENGTH(flash) );
  PROVIDE( _ld_ram_addr0 = ORIGIN(flash) );
  PROVIDE( _ld_ram_size1 = LENGTH(tcm_ocram) );
  PROVIDE( _ld_ram_addr1 = ORIGIN(tcm_ocram) );
  PROVIDE( _ld_ram_size2 = LENGTH(hbnram) );
  PROVIDE( _ld_ram_addr2 = ORIGIN(hbnram) );
  PROVIDE( _ld_ram_size3 = SIZEOF(.stack) );
  PROVIDE( _ld_ram_addr3 = ADDR(.stack) );
  PROVIDE( _ld_ram_sha_size = LENGTH(sha_ocram) );
  PROVIDE( _ld_ram_sha_addr = ORIGIN(sha_ocram) );
  PROVIDE( _ld_psram_size = LENGTH(psram) );
  PROVIDE( _ld_psram_addr = ORIGIN(psram) );

  /* BOOT2 parameter */
  PROVIDE( __boot2_pt_addr_src = BOOT2_PT_ADDR );

  /* EM size */
  PROVIDE( __LD_CONFIG_EM_SEL = __EM_SIZE );

  /* BLE_CORE_REG size */
  PROVIDE( __LD_BLE_CORE_REG_SIZE = __EM_SIZE ? 0xA4 : 0 );

  /* rom FreeRTOS */
  PROVIDE(xEventGroupCreateStatic = 0x0000000021020000);
  PROVIDE(xEventGroupCreate = 0x000000002102005e);
  PROVIDE(xEventGroupWaitBits = 0x0000000021020086);
  PROVIDE(xEventGroupClearBits = 0x000000002102017e);
  PROVIDE(vEventGroupClearBitsCallback = 0x00000000210201c8);
  PROVIDE(xEventGroupClearBitsFromISR = 0x00000000210201ca);
  PROVIDE(xEventGroupGetBitsFromISR = 0x00000000210201dc);
  PROVIDE(xEventGroupSetBits = 0x00000000210201e0);
  PROVIDE(xEventGroupSync = 0x0000000021020290);
  PROVIDE(vEventGroupSetBitsCallback = 0x00000000210203bc);
  PROVIDE(vEventGroupDelete = 0x00000000210203be);
  PROVIDE(xEventGroupSetBitsFromISR = 0x0000000021020426);
  PROVIDE(uxEventGroupGetNumber = 0x0000000021020438);
  PROVIDE(vEventGroupSetNumber = 0x0000000021020442);
  PROVIDE(vListInitialise = 0x0000000021020446);
  PROVIDE(vListInitialiseItem = 0x000000002102045a);
  PROVIDE(vListInsertEnd = 0x0000000021020460);
  PROVIDE(vListInsert = 0x0000000021020476);
  PROVIDE(uxListRemove = 0x00000000210204a6);
  PROVIDE(xQueueGenericReset = 0x0000000021020632);
  PROVIDE(xQueueGenericCreateStatic = 0x00000000210206c2);
  PROVIDE(xQueueGenericCreate = 0x0000000021020744);
  PROVIDE(xQueueCreateCountingSemaphoreStatic = 0x00000000210207c2);
  PROVIDE(xQueueCreateCountingSemaphore = 0x0000000021020800);
  PROVIDE(xQueueGenericSend = 0x0000000021020834);
  PROVIDE(xQueueCreateMutexStatic = 0x0000000021020994);
  PROVIDE(xQueueGiveMutexRecursive = 0x00000000210209c8);
  PROVIDE(xQueueCreateMutex = 0x0000000021020a1c);
  PROVIDE(xQueueGenericSendFromISR = 0x0000000021020a4c);
  PROVIDE(xQueueGiveFromISR = 0x0000000021020b0e);
  PROVIDE(xQueueReceive = 0x0000000021020b8a);
  PROVIDE(xQueueSemaphoreTake = 0x0000000021020ce8);
  PROVIDE(xQueueTakeMutexRecursive = 0x0000000021020e70);
  PROVIDE(xQueuePeek = 0x0000000021020ec0);
  PROVIDE(xQueueReceiveFromISR = 0x0000000021021010);
  PROVIDE(xQueuePeekFromISR = 0x00000000210210e8);
  PROVIDE(uxQueueMessagesWaiting = 0x0000000021021168);
  PROVIDE(uxQueueSpacesAvailable = 0x000000002102118e);
  PROVIDE(uxQueueMessagesWaitingFromISR = 0x00000000210211bc);
  PROVIDE(vQueueDelete = 0x00000000210211d8);
  PROVIDE(uxQueueGetQueueNumber = 0x000000002102125e);
  PROVIDE(vQueueSetQueueNumber = 0x0000000021021262);
  PROVIDE(ucQueueGetQueueType = 0x0000000021021266);
  PROVIDE(xQueueIsQueueEmptyFromISR = 0x000000002102126c);
  PROVIDE(xQueueIsQueueFullFromISR = 0x0000000021021290);
  PROVIDE(vQueueAddToRegistry = 0x00000000210212bc);
  PROVIDE(pcQueueGetName = 0x00000000210212e0);
  PROVIDE(vQueueUnregisterQueue = 0x0000000021021306);
  PROVIDE(vQueueWaitForMessageRestricted = 0x0000000021021330);
  PROVIDE(xStreamBufferGenericCreate = 0x0000000021021554);
  PROVIDE(xStreamBufferGenericCreateStatic = 0x00000000210215c0);
  PROVIDE(vStreamBufferDelete = 0x0000000021021656);
  PROVIDE(xStreamBufferReset = 0x000000002102169e);
  PROVIDE(xStreamBufferSetTriggerLevel = 0x00000000210216fa);
  PROVIDE(xStreamBufferSpacesAvailable = 0x0000000021021728);
  PROVIDE(xStreamBufferBytesAvailable = 0x000000002102176a);
  PROVIDE(xStreamBufferSend = 0x00000000210217a8);
  PROVIDE(xStreamBufferSendFromISR = 0x00000000210218e8);
  PROVIDE(xStreamBufferReceive = 0x00000000210219ae);
  PROVIDE(xStreamBufferNextMessageLengthBytes = 0x0000000021021ac2);
  PROVIDE(xStreamBufferReceiveFromISR = 0x0000000021021b2c);
  PROVIDE(xStreamBufferIsEmpty = 0x0000000021021be2);
  PROVIDE(xStreamBufferIsFull = 0x0000000021021c0e);
  PROVIDE(xStreamBufferSendCompletedFromISR = 0x0000000021021c4c);
  PROVIDE(xStreamBufferReceiveCompletedFromISR = 0x0000000021021c84);
  PROVIDE(uxStreamBufferGetStreamBufferNumber = 0x0000000021021cbc);
  PROVIDE(vStreamBufferSetStreamBufferNumber = 0x0000000021021cc0);
  PROVIDE(ucStreamBufferGetStreamBufferType = 0x0000000021021cc4);
  PROVIDE(xTaskCreateStatic = 0x000000002102204e);
  PROVIDE(xTaskCreate = 0x00000000210220e4);
  PROVIDE(vTaskDelete = 0x0000000021022160);
  PROVIDE(eTaskGetState = 0x0000000021022262);
  PROVIDE(uxTaskPriorityGet = 0x000000002102231a);
  PROVIDE(uxTaskPriorityGetFromISR = 0x000000002102235c);
  PROVIDE(vTaskPrioritySet = 0x000000002102236a);
  PROVIDE(vTaskResume = 0x0000000021022482);
  PROVIDE(xTaskResumeFromISR = 0x000000002102252a);
  PROVIDE(vTaskStartScheduler = 0x00000000210225ba);
  PROVIDE(vTaskEndScheduler = 0x000000002102262a);
  PROVIDE(vTaskSuspendAll = 0x0000000021022636);
  PROVIDE(xTaskGetTickCount = 0x0000000021022642);
  PROVIDE(xTaskGetTickCount2 = 0x0000000021022648);
  PROVIDE(xTaskGetTickCountFromISR = 0x0000000021022658);
  PROVIDE(uxTaskGetNumberOfTasks = 0x000000002102265e);
  PROVIDE(pcTaskGetName = 0x0000000021022664);
  PROVIDE(xTaskGetIdleTaskHandle = 0x000000002102268a);
  PROVIDE(vTaskStepTick = 0x00000000210226b0);
  PROVIDE(xTaskIncrementTick = 0x00000000210226e0);
  PROVIDE(xTaskResumeAll = 0x0000000021022846);
  PROVIDE(vTaskDelayUntil = 0x0000000021022968);
  PROVIDE(vTaskDelay = 0x00000000210229dc);
  PROVIDE(vTaskSwitchContext = 0x0000000021022af2);
  PROVIDE(vTaskSuspend = 0x0000000021022ba8);
  PROVIDE(vTaskPlaceOnEventList = 0x0000000021022ce4);
  PROVIDE(vTaskPlaceOnUnorderedEventList = 0x0000000021022d12);
  PROVIDE(vTaskPlaceOnEventListRestricted = 0x0000000021022d62);
  PROVIDE(xTaskRemoveFromEventList = 0x0000000021022d9a);
  PROVIDE(vTaskRemoveFromUnorderedEventList = 0x0000000021022e30);
  PROVIDE(vTaskSetTimeOutState = 0x0000000021022eb4);
  PROVIDE(vTaskInternalSetTimeOutState = 0x0000000021022f06);
  PROVIDE(xTaskCheckForTimeOut = 0x0000000021022f14);
  PROVIDE(vTaskMissedYield = 0x0000000021022fb4);
  PROVIDE(uxTaskGetTaskNumber = 0x0000000021022fbc);
  PROVIDE(vTaskSetTaskNumber = 0x0000000021022fc6);
  PROVIDE(eTaskConfirmSleepModeStatus = 0x0000000021022fcc);
  PROVIDE(vTaskGetInfo = 0x0000000021022ff0);
  PROVIDE(uxTaskGetSystemState = 0x0000000021023144);
  PROVIDE(uxTaskGetStackHighWaterMark = 0x000000002102323c);
  PROVIDE(xTaskGetCurrentTaskHandle = 0x000000002102327e);
  PROVIDE(xTaskGetSchedulerState = 0x0000000021023284);
  PROVIDE(xTaskPriorityInherit = 0x0000000021023298);
  PROVIDE(xTaskPriorityDisinherit = 0x000000002102336a);
  PROVIDE(vTaskPriorityDisinheritAfterTimeout = 0x0000000021023424);
  PROVIDE(vTaskEnterCritical = 0x0000000021023510);
  PROVIDE(vTaskExitCritical = 0x0000000021023520);
  PROVIDE(vTaskList = 0x0000000021023534);
  PROVIDE(uxTaskResetEventItemValue = 0x0000000021023648);
  PROVIDE(pvTaskIncrementMutexHeldCount = 0x0000000021023660);
  PROVIDE(ulTaskNotifyTake = 0x0000000021023674);
  PROVIDE(xTaskNotifyWait = 0x0000000021023712);
  PROVIDE(xTaskGenericNotify = 0x00000000210237ea);
  PROVIDE(xTaskGenericNotifyFromISR = 0x000000002102391e);
  PROVIDE(vTaskNotifyGiveFromISR = 0x0000000021023a28);
  PROVIDE(xTaskNotifyStateClear = 0x0000000021023ad4);
  PROVIDE(xTimerCreateTimerTask = 0x0000000021023bde);
  PROVIDE(xTimerCreate = 0x0000000021023c36);
  PROVIDE(xTimerCreateStatic = 0x0000000021023ca6);
  PROVIDE(xTimerGenericCommand = 0x0000000021023d2c);
  PROVIDE(xTimerGetTimerDaemonTaskHandle = 0x0000000021024028);
  PROVIDE(xTimerGetPeriod = 0x000000002102404e);
  PROVIDE(vTimerSetReloadMode = 0x000000002102406a);
  PROVIDE(xTimerGetExpiryTime = 0x00000000210240b2);
  PROVIDE(pcTimerGetName = 0x00000000210240ce);
  PROVIDE(xTimerIsTimerActive = 0x00000000210240ea);
  PROVIDE(pvTimerGetTimerID = 0x0000000021024114);
  PROVIDE(vTimerSetTimerID = 0x000000002102413a);
  PROVIDE(xTimerPendFunctionCallFromISR = 0x0000000021024162);
  PROVIDE(xTimerPendFunctionCall = 0x0000000021024186);
  PROVIDE(uxTimerGetTimerNumber = 0x00000000210241d2);
  PROVIDE(vTimerSetTimerNumber = 0x00000000210241d6);
  PROVIDE(vPortSetupTimerInterrupt = 0x00000000210241da);
  PROVIDE(xPortStartScheduler = 0x000000002102421e);
  PROVIDE(vPortEndScheduler = 0x000000002102425a);
  PROVIDE(freertos_risc_v_trap_handler = 0x0000000021024300);
  PROVIDE(pvPortMalloc = 0x0000000021024762);
  PROVIDE(vPortFree = 0x000000002102483a);
  PROVIDE(xPortGetFreeHeapSize = 0x00000000210248ce);
  PROVIDE(xPortGetMinimumEverFreeHeapSize = 0x00000000210248d4);
  PROVIDE(vPortDefineHeapRegions = 0x00000000210248da);
  PROVIDE(pullMachineTimerCompareRegister = 0x0000000021024a50);
  PROVIDE(pTrapNetCounter = __global_pointer$ -2016);
  PROVIDE(pullNextTime = __global_pointer$ -2012);
  PROVIDE(uxTimerIncrementsForOneTick = __global_pointer$ -2008);
  PROVIDE(pxCurrentTCB = __global_pointer$ -2004);
  PROVIDE(TrapNetCounter = __global_pointer$ -1924);
  PROVIDE(ullNextTime = __global_pointer$ -1920);

  PROVIDE(vApplicationMallocFailedHook = __global_pointer$ -876);
  PROVIDE(vAssertCalled = __global_pointer$ -864);
}
