OUTPUT_ARCH( "riscv" )

ENTRY( _enter )

__EM_SIZE = DEFINED(__EM_SIZE) ? __EM_SIZE : 0K;
__CACHE_SIZE = DEFINED(__CACHE_SIZE) ? __CACHE_SIZE : 16K;
__stack_size = DEFINED(__stack_size) ? __stack_size : 2K;

MEMORY
{
  flash  (rxai!w) : ORIGIN = 0x23000000, LENGTH = (2M)
  tcm_ocram (wxa) : ORIGIN = 0x4201C000 + __CACHE_SIZE, LENGTH = (16K - __CACHE_SIZE + 80K - __EM_SIZE - __stack_size)
  sys_stack (wxa) : ORIGIN = 0x42034000 - __EM_SIZE - __stack_size, LENGTH = __stack_size
  hbnram    (wxa) : ORIGIN = 0x40010000, LENGTH = (4K)
}

SECTIONS
{
  PROVIDE(__metal_chicken_bit = 0);

  .init :
  {
    KEEP (*(.text.metal.init.enter))
    KEEP (*(SORT_NONE(.init)))
  } >flash

  .ver :
  {
    . = ORIGIN(flash) + 0xC00;
    LONG(0x42464c42); /* BLFB */
    LONG(0x46524556); /* VERF */
    KEEP(*(.blverinf))
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

  .hbn : ALIGN(4)
  {
    PROVIDE( __system_ram_load_addr = LOADADDR(.hbn) );
    PROVIDE( __system_ram_data_start__ = ADDR(.hbn) );
    PROVIDE( __system_ram_data_end__ = ADDR(.hbn) + SIZEOF(.hbn) );

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
    PROVIDE( __dtcm_load_addr = LOADADDR(.tcmdata) );
    PROVIDE( __tcm_data_start__ = ADDR(.tcmdata) );
    PROVIDE( __tcm_data_end__ = ADDR(.tcmdata) + SIZEOF(.tcmdata) );

    *(.tcm_data)
  } >tcm_ocram

  .tcmcode : ALIGN(4)
  {
    PROVIDE( __itcm_load_addr = LOADADDR(.tcmcode) );
    PROVIDE( __tcm_code_start__ = ADDR(.tcmcode) );
    PROVIDE( __tcm_code_end__ = ADDR(.tcmcode) + SIZEOF(.tcmcode) );

    *(.tcm_code.*)
    *(.tcm_const.*)
    *(.sclock_rlt_code.*)
    *(.sclock_rlt_const.*)
  } >tcm_ocram AT >flash

  .data : ALIGN(4)
  {
    PROVIDE( __ram_load_addr = LOADADDR(.data) );
    PROVIDE( __ram_data_start__ = ADDR(.data) );
    PROVIDE( __ram_data_end__ = ADDR(.data) + SIZEOF(.data) );

    *(.data .data.*)
    *(.gnu.linkonce.d.*)

    . = ALIGN(8);
    PROVIDE( __global_pointer$ = . + 0x7F0 );
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

  .bss (NOLOAD) :
  {
    PROVIDE( __bss_start__ = ADDR(.bss) );
    PROVIDE( __bss_end__ = ADDR(.bss) + SIZEOF(.bss) );

    *(.sbss*)
    *(.gnu.linkonce.sb.*)
    *(.bss .bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
  } >tcm_ocram

  .sha_ocram (NOLOAD) :
  {
    *(.sha_ocram*)
  } >tcm_ocram

  . = ALIGN(8);
  PROVIDE( __HeapBase = . );
  PROVIDE( __HeapLimit = ORIGIN(tcm_ocram) + LENGTH(tcm_ocram) );

  .stack (NOLOAD) : ALIGN(8)
  {
    PROVIDE( __StackLimit = . );
    . = . + __stack_size;
    PROVIDE( __StackTop = . );
  } >sys_stack


  /* EM size */
  PROVIDE( __LD_CONFIG_EM_SEL = __EM_SIZE );

  /* BLE_CORE_REG size */
  PROVIDE( __LD_BLE_CORE_REG_SIZE = __EM_SIZE ? 0x214 : 0 );

  /* rom driver */
  INCLUDE romdriver.ld
}
