/****************************************************************************************
* @file map.txt
*
* @brief This file is the map file (gnuarm or armgcc).
*
* Copyright (C) BouffaloLab 2018
*
****************************************************************************************
*/

/* configure the CPU type */
OUTPUT_ARCH( "riscv" )
/* link with the standard c library */
INPUT(-lc)
/* link with the standard GCC library */
INPUT(-lgcc)
/* configure the entry point */
ENTRY(_enter)

StackSize = 0x1000; /*  4KB */
VerOffset = 0xc00;
Boot2Size = 0xC000;
PROVIDE(__boot2_pass_param_addr = 0x4202DC00);
MEMORY
{
	xip_memory  (rx)  : 	ORIGIN = 0x23000000, LENGTH = 3K + 256
 	xip_memory1 (rx)  : 	ORIGIN = 0x23000d00, LENGTH = 44K + 768
    itcm_memory (rx)  : 	ORIGIN = 0x22014000, LENGTH = 16K
    dtcm_memory (rx)  : 	ORIGIN = 0x42018000, LENGTH = 4K
    ram_memory  (!rx) : 	ORIGIN = 0x42019000, LENGTH = 83K
    hbn_memory  (!rx) : 	ORIGIN = 0x40010000, LENGTH = 4K
}

SECTIONS
{
    PROVIDE(__metal_chicken_bit = 0);

    .init :
    {
    	__text_code_start__ = .;

        KEEP (*(.text.metal.init.enter))
        KEEP (*(SORT_NONE(.init)))

        /* section information for usb desc */
        . = ALIGN(4);
        _usb_desc_start = .;
        KEEP(*(usb_desc))
        . = ALIGN(4);
        _usb_desc_end = .;
    }

	.ver 0 : AT(ORIGIN(xip_memory) + VerOffset)
    {

		LONG(0x42464c42); /* BLFB */
		LONG(0x46524556); /* VERF */
		KEEP (*(.blverinf))
		LONG(0x42464c42); /* BLFB */
		LONG(0x46524556); /* VERF */
		KEEP (*(.verinfo))
    }

    .text :
    {
        *(EXCLUDE_FILE
        	( *bl702_uart*.o*
        	  *hal_uart*.o*
        	  *bl702_sec_eng*.o*
        	  *bl702_glb*.o*
        	  *hal_sec_hash*.o*
        	  *hal_mtimer*.o*
        	  *hal_boot2*.o*
        	  *bflb_eflash_loader_interface*.o*
        	  *bflb_eflash_loader_uart*.o*
        	  *bflb_platform*.o*
        	  *xz_port*.o*
        	  *drv_device*.o*
        	  *bl702_clock*.o*
        	  *_udivdi3*.o*
        	  *lib_a-memset*.o*) .text*)

        *(.rodata)
        *(.rodata.*)
        *(.srodata)
        *(.srodata.*)

        . = ALIGN(4);
        __text_code_end__ = .;
    } > xip_memory1

    __itcm_load_addr = __text_code_end__;

    .itcm_region : AT (__itcm_load_addr)
    {
        . = ALIGN(4);
        __tcm_code_start__ = .;
        *(.tcm_code.*)
        *(.tcm_const.*)
        *(.sclock_rlt_code.*)
        *(.sclock_rlt_const.*)
        *bl702_romapi*.o*(.text)
        *bl702_romapi*.o*(.text.*)
        *bl702_romapi*.o*(.rodata)
        *bl702_romapi*.o*(.rodata.*)
        *bl702_romapi*.o*(.srodata)
        *bl702_romapi*.o*(.srodata.*)
        *bl702_uart*.o* (.text*)
        *hal_uart*.o* (.text*)
        *bl702_sec_eng*.o* (.text*)
        *bl702_glb*.o* (.text*)
        *hal_sec_hash*.o* (.text*)
        *hal_mtimer*.o* (.text*)
        *hal_boot2*.o* (.text*)
        *bflb_eflash_loader_interface*.o* (.text*)
        *bflb_eflash_loader_uart*.o* (.text*)
        *bflb_platform*.o* (.text*)
        *xz_port*.o* (.text*)
        *drv_device*.o* (.text*)
        *bl702_clock*.o* (.text*)
        *udivdi3*(.rodata*)
        *udivdi3*(.text*)
        *memset*(.text*)
        . = ALIGN(4);
        __tcm_code_end__ = .;

    } > itcm_memory

    __dtcm_load_addr = __itcm_load_addr + SIZEOF(.itcm_region);

    .dtcm_region : AT (__dtcm_load_addr)
    {
        . = ALIGN(4);
        __tcm_data_start__ = .;

        *(.tcm_data.*)

        . = ALIGN(4);
        __tcm_data_end__ = .;
    } > dtcm_memory

    __StackTop = ORIGIN(dtcm_memory) + LENGTH(dtcm_memory);
    __StackLimit = __StackTop - StackSize;

    /* Check if data + heap + stack exceeds dtcm limit */
    ASSERT(__StackLimit >= __tcm_data_end__, "region dtcm overflowed with stack")

    __system_ram_load_addr = __dtcm_load_addr + SIZEOF(.dtcm_region);

    .system_ram_data_region :
    {
        . = ALIGN(4);
        __system_ram_data_start__ = .;

        *(.system_ram)

        . = ALIGN(4);
        __system_ram_data_end__ = .;
    } > ram_memory


    __ram_load_addr = __system_ram_load_addr + SIZEOF(.system_ram_data_region);

    /* Data section */
    .ram_data_region : AT (__ram_load_addr)
    {
        . = ALIGN(4);
        __ram_data_start__ = .;

        PROVIDE( __global_pointer$ = . + 0x800 );

        *(.data)
        *(.data.*)
        *(.sdata)
        *(.sdata.*)
        *(.sdata2)
        *(.sdata2.*)

        . = ALIGN(4);
        __ram_data_end__ = .;
    } > ram_memory

    /* check flash size must less 48K */
    __etext_final__ = (__ram_load_addr + SIZEOF(.ram_data_region));
    ASSERT(__etext_final__ <= ORIGIN(xip_memory) + Boot2Size, "xip memory overflowed")

    .nocache_ram_region (NOLOAD) :
    {
        . = ALIGN(4);
        *(.nocache_ram)
        *(.nocache_noinit_ram)
        . = ALIGN(4);
    } > ram_memory

    .system_ram_noinit_data_region (NOLOAD) :
    {
        . = ALIGN(4);
        *(.system_ram_noinit)
        . = ALIGN(4);
    } > ram_memory

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __bss_start__ = .;

        *(.bss*)
        *(.sbss*)

        . = ALIGN(4);
        __bss_end__ = .;
    } > ram_memory

    .noinit_data (NOLOAD) :
    {
        . = ALIGN(4);
        *(.noinit_data*)
        . = ALIGN(4);
    } > ram_memory

    .noinit_hbn_ram_region (NOLOAD) :
    {
        . = ALIGN(4);
        *(.noinit_hbn_ram_data*)
        . = ALIGN(4);
    } > hbn_memory

}

