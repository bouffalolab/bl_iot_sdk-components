OUTPUT_ARCH( "riscv" )

ENTRY( bl702_start )

__EM_SIZE = DEFINED(ble_controller_init) ? 8K : 0K;
__CACHE_SIZE = DEFINED(__CACHE_SIZE) ? __CACHE_SIZE : 16K;
__stack_size = DEFINED(__stack_size) ? __stack_size : 2K;
__sha_ocram_size = DEFINED(__sha_ocram_size) ? __sha_ocram_size : 128;

MEMORY
{
  flash  (rxai!w) : ORIGIN = 0x23000000, LENGTH = (2M)
  tcm       (wxa) : ORIGIN = 0x42014000, LENGTH = (32K)
  tcm_rsvd  (wxa) : ORIGIN = 0x4201C000, LENGTH = (16K)
  ocram_1   (wxa) : ORIGIN = 0x42020000, LENGTH = (27K)              /* retention memory, will not be initialized in bootrom */
  ocram_2   (wxa) : ORIGIN = 0x42026C00, LENGTH = (13K)              /* non-retention memory, will be initialized in bootrom */
  ocram_3   (wxa) : ORIGIN = 0x4202A000, LENGTH = (24K - __EM_SIZE)  /* retention memory, will not be initialized in bootrom */
  hbnram    (wxa) : ORIGIN = 0x40010000, LENGTH = (4K)
  bugkill   (rxai!w) : ORIGIN = 0xD0000000, LENGTH = 16M
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

  .bugkiller_command :
  {
    PROVIDE( __bugkiller_command_start = ADDR(.bugkiller_command) );
    PROVIDE( __bugkiller_command_end = ADDR(.bugkiller_command) + SIZEOF(.bugkiller_command) );
    KEEP (*(.static_bugkiller_command))
  } > bugkill

  .bugkiller  :
  {
    KEEP (*(.bugkiller_code))
    KEEP (*bugkiller*.o(.rodata* .text* .data* .sdata* .sbss*))
  } > bugkill

  .rodata :
  {
    *(.rdata)
    *(.rodata .rodata.*)
    *(.sdata2.*)
    *(.gcc_except_table.*)

    . = ALIGN(8);
    *(.srodata.cst16)
    *(.srodata.cst8)
    *(.srodata.cst4)
    *(.srodata.cst2)
    *(.srodata .srodata.*)

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
  } >ocram_1

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
  } >tcm

  .tcmcode : ALIGN(4)
  {
    PROVIDE( _tcm_load = LOADADDR(.tcmcode) );
    PROVIDE( _tcm_run = ADDR(.tcmcode) );
    PROVIDE( _tcm_run_end = ADDR(.tcmcode) + SIZEOF(.tcmcode) );

    /* tcm code that should be restored after pds wakeup */
    *(.tcm_code.*)
    *(.sclock_rlt_code.*)
    PROVIDE ( _pds_restore_tcm_run_end = . );

    *(.tcm_code.*)
    *(.tcm_const.*)
    *(.sclock_rlt_code.*)
    *(.sclock_rlt_const.*)

    *libblestack.a:ble_cli_cmds.o(.sdata.* .srodata.* .sbss.* .bss.* COMMON)
  } >tcm AT >flash

  .data : ALIGN(4)
  {
    PROVIDE( _data_load = LOADADDR(.data) );
    PROVIDE( _data_run = ADDR(.data) );
    PROVIDE( _data_run_end = ADDR(.data) + SIZEOF(.data) );

    *(.pds_code.*)
    *(.data .data.*)
    *(.gnu.linkonce.d.*)

    . = ALIGN(8);
    /*always put freertos under global_pointer with the following order. No change!*/
    PROVIDE( __global_pointer$ = . + 0x7F0 );
    KEEP(*libbl702_freertos.a:*(.sdata.*))
    KEEP(*libbl702_freertos.a:*(.sbss.*))
    KEEP(*libbl702_freertos.a:*(.bss.*))
    KEEP(*libbl702_freertos.a:*(COMMON))
    *(.sdata .sdata.*)
    *(.gnu.linkonce.s.*)

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
  } >ocram_1 AT >flash

  .boot2 (NOLOAD) :
  {
    PROVIDE ( __boot2_pt_addr_start = . );
    *(.bss.boot2_partition_table)
    PROVIDE ( __boot2_pt_addr_end   = . );

    PROVIDE ( __boot2_flashCfg_start = . );
    *(.bss.boot2_flashCfg)
    PROVIDE ( __boot2_flashCfg_end = . );
  } >hbnram

  .bss (NOLOAD) :
  {
    PROVIDE( __bss_start = ADDR(.bss) );
    PROVIDE( __bss_end = ADDR(.bss) + SIZEOF(.bss) );

    *(.sbss*)
    *(.gnu.linkonce.sb.*)
    *(.bss .bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
  } >ocram_1

  . = ALIGN(8);
  PROVIDE( _heap_start = . );
  PROVIDE( _heap_size = ORIGIN(ocram_1) + LENGTH(ocram_1) - _heap_start );

  PROVIDE( _heap2_start = ORIGIN(ocram_3) );
  PROVIDE( _heap2_size = LENGTH(ocram_3) );

  .stack (NOLOAD) : ALIGN(8)
  {
    PROVIDE( _sp_base = . );
    . = . + __stack_size;
    PROVIDE( _sp_main = . );
    __freertos_irq_stack_top = .;
  } >ocram_2

  .sha_ocram (NOLOAD) :
  {
    *(.sha_ocram*)
  } >ocram_2

  PROVIDE( _tcm_rsvd_start = ORIGIN(tcm_rsvd) );
  PROVIDE( _tcm_rsvd_end = ORIGIN(tcm_rsvd) + LENGTH(tcm_rsvd) );

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
  PROVIDE( _ld_ram_size1 = LENGTH(ocram_1) );
  PROVIDE( _ld_ram_addr1 = ORIGIN(ocram_1) );
  PROVIDE( _ld_ram_size2 = LENGTH(ocram_3) );
  PROVIDE( _ld_ram_addr2 = ORIGIN(ocram_3) );
  PROVIDE( _ld_ram_size3 = SIZEOF(.stack) );
  PROVIDE( _ld_ram_addr3 = ADDR(.stack) );
  PROVIDE( _ld_ram_sha_size = SIZEOF(.sha_ocram) );
  PROVIDE( _ld_ram_sha_addr = ADDR(.sha_ocram) );

  /* BOOT2 parameter */
  PROVIDE( __boot2_pt_addr_src = BOOT2_PT_ADDR );

  /* EM size */
  PROVIDE( __LD_CONFIG_EM_SEL = __EM_SIZE );

  /* BLE_CORE_REG size */
  PROVIDE( __LD_BLE_CORE_REG_SIZE = __EM_SIZE ? 0xA4 : 0 );
}
