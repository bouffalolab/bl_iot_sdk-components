#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <vfs.h>
#include <device/vfs_uart.h>
#include <aos/kernel.h>
#include <aos/yloop.h>
#include <event_device.h>
#include <cli.h>

#include <bl702l_clock.h>
#include <bl_sys.h>
#include <bl_chip.h>
#include <bl_irq.h>
#include <bl_rtc.h>
#include <bl_sec.h>
#include <hal_boot2.h>
#include <hal_board.h>
#include <hal_hwtimer.h>
#include <hal_tcal.h>
#include <hosal_uart.h>
#include <hosal_dma.h>

#include <libfdt.h>
#include <utils_log.h>
#include <blog.h>
#ifdef EASYFLASH_ENABLE
#include <easyflash.h>
#endif
#ifdef SYS_LOOPRT_ENABLE
#include <looprt.h>
#include <loopset.h>
#endif
#ifdef SYS_USER_VFS_ROMFS_ENABLE
#include <bl_romfs.h>
#endif

#ifdef CFG_USE_ROM_CODE
#include <rom_freertos_ext.h>
#include <rom_hal_ext.h>
#include <rom_lmac154_ext.h>
#endif

HOSAL_UART_DEV_DECL(uart_stdio, 0, 14, 15, 2000000);

extern uint8_t _heap_start;
extern uint8_t _heap_size; // @suppress("Type cannot be resolved")
extern uint8_t _heap2_start;
extern uint8_t _heap2_size; // @suppress("Type cannot be resolved")
static HeapRegion_t xHeapRegions[] =
{
    { &_heap_start,  (unsigned int) &_heap_size },
    { &_heap2_start, (unsigned int) &_heap2_size },
    { NULL, 0 }, /* Terminates the array. */
    { NULL, 0 } /* Terminates the array. */
};

#if defined(CFG_USE_PSRAM)
extern uint8_t _heap3_start;
extern uint8_t _heap3_size; // @suppress("Type cannot be resolved")
static HeapRegion_t xHeapRegionsPsram[] =
{
    { &_heap3_start, (unsigned int) &_heap3_size },
    { NULL, 0 }, /* Terminates the array. */
    { NULL, 0 } /* Terminates the array. */
};
#endif

#ifndef CFG_USE_ROM_CODE
void __attribute__((weak)) vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
#else
void __attribute__((weak)) user_vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
#endif
{
    puts("Stack Overflow checked\r\n");
    if(pcTaskName){
        printf("Stack name %s\r\n", pcTaskName);
    }
    while (1) {
        /*empty here*/
    }
}

#ifndef CFG_USE_ROM_CODE
void __attribute__((weak)) vApplicationMallocFailedHook(void)
#else
void __attribute__((weak)) user_vApplicationMallocFailedHook(void)
#endif
{
    printf("Memory Allocate Failed. Current left size is %d bytes\r\n",
        xPortGetFreeHeapSize()
    );
#if defined(CFG_USE_PSRAM)
    printf("Current psram left size is %d bytes\r\n",
        xPortGetFreeHeapSizePsram()
    );
#endif
    while (1) {
        /*empty here*/
    }
}

#ifndef CFG_USE_ROM_CODE
void __attribute__((weak)) vApplicationIdleHook(void)
{
    __asm volatile(
            "   wfi     "
    );
    /*empty*/
}

#if ( configUSE_TICKLESS_IDLE != 0 )
void __attribute__((weak)) vApplicationSleep( TickType_t xExpectedIdleTime )
{
    /*empty*/
}
#endif

#if ( configUSE_TICK_HOOK != 0 )
void __attribute__((weak)) vApplicationTickHook( void )
{
    /*empty*/
}
#endif

void __attribute__((weak)) vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void __attribute__((weak)) vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif

#ifndef CFG_USE_ROM_CODE
void user_vAssertCalled(void) __attribute__ ((weak, alias ("vAssertCalled")));
void __attribute__((weak)) vAssertCalled(void)
#else
void __attribute__((weak)) user_vAssertCalled(void)
#endif
{
#if 0
    taskDISABLE_INTERRUPTS();
    abort();
#else
    taskDISABLE_INTERRUPTS();
    printf("vAssertCalled, ra = %p, taskname %s\r\n", 
        (void *)__builtin_return_address(0), pcTaskGetName(NULL));
    while (1) {
        /*empty here*/
    }
#endif
}

void __attribute__((weak)) _cli_init(int fd_console)
{
    /*empty*/
}

static void app_main_entry(void *pvParameters)
{
    extern int main();
    main();
    vTaskDelete(NULL);
}

static void aos_loop_proc(void *pvParameters)
{
#ifdef SYS_LOOPRT_ENABLE
    static StackType_t proc_stack_looprt[512];
    static StaticTask_t proc_task_looprt;

    /*Init bloop stuff*/
    looprt_start(proc_stack_looprt, 512, &proc_task_looprt);
    loopset_led_hook_on_looprt();
#endif
#ifdef EASYFLASH_ENABLE
    easyflash_init();
#endif

#ifdef SYS_VFS_ENABLE
    vfs_init();
    vfs_device_init();
#endif

#ifdef SYS_VFS_UART_ENABLE
    uint32_t fdt = 0, offset = 0;
    const char *uart_node[] = {
        "uart@4000A000",
    };

    if (0 == hal_board_get_dts_addr("uart", &fdt, &offset)) {
        vfs_uart_init(fdt, offset, uart_node, 1);
    }
#endif

#ifdef SYS_USER_VFS_ROMFS_ENABLE
    romfs_register();
#endif

#ifdef SYS_AOS_LOOP_ENABLE
    aos_loop_init();
#endif

#ifdef SYS_AOS_CLI_ENABLE
    int fd_console;
    fd_console = aos_open("/dev/ttyS0", 0);
    if (fd_console >= 0) {
        printf("Init CLI with event Driven\r\n");
        aos_cli_init(0);
        aos_poll_read_fd(fd_console, aos_cli_event_cb_read_get(), (void*)0x12345678);
        _cli_init(fd_console);
    }
#elif defined (CFG_OPENTHREAD_CLI_EN)
    extern void ot_cli_init(void);
    ot_cli_init();
#endif

#if defined(CFG_TCAL_ENABLE)
    hal_hwtimer_init();
    hal_tcal_init();
#endif

    xTaskCreate(app_main_entry,
            (char*)"main",
            SYS_APP_TASK_STACK_SIZE / sizeof(StackType_t),
            NULL,
            SYS_APP_TASK_PRIORITY,
            NULL);

#ifdef SYS_AOS_LOOP_ENABLE
    aos_loop_run();
#endif
    puts("------------------------------------------\r\n");
    puts("+++++++++Critical Exit From AOS LOOP entry++++++++++\r\n");
    puts("******************************************\r\n");
    vTaskDelete(NULL);
}

void __attribute__((weak)) _dump_lib_info(void)
{
    /*empty*/
}

static void _dump_boot_info(void)
{
    char chip_feature[40];
#if 0
    const char *banner;

    puts("Booting BL702 Chip...\r\n");

    /*Display Banner*/
    if (0 == bl_chip_banner(&banner)) {
        puts(banner);
    }
    puts("\r\n");
#endif
    /*Chip Feature list*/
    puts("\r\n");
    puts("------------------------------------------------------------\r\n");
    puts("RISC-V Core Feature:");
    bl_chip_info(chip_feature);
    puts(chip_feature);
    puts("\r\n");

    puts("Build Version: ");
    puts(BL_SDK_VER); // @suppress("Symbol is not resolved")
    puts("\r\n");

    puts("Std BSP Driver Version: ");
    puts(BL_SDK_STDDRV_VER); // @suppress("Symbol is not resolved")
    puts("\r\n");

    puts("Std BSP Common Version: ");
    puts(BL_SDK_STDCOM_VER); // @suppress("Symbol is not resolved")
    puts("\r\n");

    puts("RF Version: ");
    puts(BL_SDK_RF_VER); // @suppress("Symbol is not resolved")
    puts("\r\n");

    _dump_lib_info();

    puts("Build Date: ");
    puts(__DATE__);
    puts("\r\n");
    puts("Build Time: ");
    puts(__TIME__);
    puts("\r\n");
    puts("------------------------------------------------------------\r\n");

}

void setup_heap()
{
    bl_sys_em_config();

#ifdef CFG_USE_ROM_CODE
    // Initialize rom data
    extern uint8_t _rom_data_run;
    extern uint8_t _rom_data_load;
    extern uint8_t _rom_data_size;
    memcpy((void *)&_rom_data_run, (void *)&_rom_data_load, (size_t)&_rom_data_size);
#endif

    // Invoked during system boot via start.S
    vPortDefineHeapRegions(xHeapRegions);


#if defined(CFG_USE_PSRAM)
    extern uint8_t _psram_start;
    extern uint8_t _psram_end;
    extern void bl_psram_init(void);

    bl_psram_init();
    memset(&_psram_start, 0, &_psram_end - &_psram_start);
    vPortDefineHeapRegionsPsram(xHeapRegionsPsram);
#endif /*CFG_USE_PSRAM*/
}

static void system_early_init(void)
{
#ifdef SYS_BLOG_ENABLE
    blog_init();
#endif

    bl_irq_init();
    bl_rtc_init();
    bl_sec_init();
    hal_boot2_init();

#ifdef SYS_DMA_ENABLE
    hosal_dma_init();
#endif

    /* To be added... */
    /* board config is set after system is init*/
    hal_board_cfg(0);
}

void bl702_main()
{
    TaskHandle_t aos_loop_proc_task;

    bl_sys_early_init();
    
#ifdef CFG_USE_ROM_CODE
    rom_freertos_init(256, 400);
    rom_hal_init();
    rom_lmac154_hook_init();
#endif

#if defined(GPIO_SIM_PRINT)
    extern int bl_gpio_uart_init(uint8_t tx_pin, uint32_t baudrate);
    bl_gpio_uart_init(8, 1000000);
#endif

    /*Init UART In the first place*/
    hosal_uart_init(&uart_stdio);
    puts("Starting bl702 now....\r\n");

    _dump_boot_info();

    printf("Reset Info %d\r\n", bl_sys_rstinfo_get());
    printf("CPU %luHz\r\n", SystemCoreClockGet());

    printf("Heap %u@%p, %u@%p\r\n",
            (unsigned int)&_heap_size, &_heap_start,
            (unsigned int)&_heap2_size, &_heap2_start
    );

#if defined(CFG_USE_PSRAM)
    printf("PSRAM Heap %u@%p\r\n",(unsigned int)&_heap3_size, &_heap3_start);
#endif

    system_early_init();

    puts("[OS] Starting aos_loop_proc task...\r\n");
    xTaskCreate(aos_loop_proc, (char*)"event_loop", 1024, NULL, 15, &aos_loop_proc_task);

    puts("[OS] Starting OS Scheduler...\r\n");
    vTaskStartScheduler();
}
