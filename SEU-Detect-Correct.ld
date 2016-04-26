/*
*****************************************************************************
**

**  File        : LinkerScript.ld
**
**  Abstract    : Linker script for STM32F417IGHx Device with
**                1024KByte FLASH, 128KByte RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
**  (c)Copyright Ac6.
**  You may use this file as-is or modify it according to the needs of your
**  project. Distribution of this file (unmodified or modified) is not
**  permitted. Ac6 permit registered System Workbench for MCU users the
**  rights to distribute the assembled, compiled & linked contents of this
**  file as part of an application binary file, provided that it is built
**  using the System Workbench for MCU toolchain.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = 0x2001FFFF;    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x200;;      /* required amount of heap  */
_Min_Stack_Size = 0x400;; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 1024K
RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 128K
CCMRAM (rw)      : ORIGIN = 0x10000000, LENGTH = 64K
}

/* Define output sections */
SECTIONS
{
  /* The vector table goes first into FLASH */
  .isr_vector :
  {
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /* Section 1 code is next */
  .section1 :
  {
    KEEP(*(.section1))
    . = ALIGN(4);
  } >FLASH

  /* The program code and constant data goes into FLASH */
  .text :
  {
  	_stext = .;
    *(.text)
    *(.text.*)
	_exit = .;
    *(.text.Default_Handler)
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))
    KEEP(*(.rodata))    /* .rodata sections (constants, strings, etc.) */
    KEEP(*(.rodata*))   /* .rodata* sections (constants, strings, etc.) */

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Section 3 code is next */
  .section3 :
  {
    KEEP(*(.section3))
    . = ALIGN(4);
  } >FLASH

  .jcr :
  {
    *(.jcr)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH

  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  .block_pad :
  {
    . = (13311 - ((. - ORIGIN(FLASH)) % 13311) + .);
    . = ALIGN(4);
  } >FLASH

  .ecc_data :
  {
    RS_DATA_START = .; LONG(ADDR(.ecc_data) - ORIGIN(FLASH));      /* RS_DATA_START */
    RS_PARITY_START = .;LONG(ADDR(.rs_parity) - ORIGIN(FLASH));     /* RS_PARITY_START */
    CRC_START = .;LONG(ADDR(.crc_data) - ORIGIN(FLASH));      /* CRC_SPACE */
    RS_BLOCK_COUNT = .; LONG(((ADDR(.rs_parity) - ORIGIN(FLASH)) / 13311));                         /* BLOCK_COUNT */
  } >FLASH

  .rs_parity :
  {
    KEEP(*(.rs_parity*))
    . = ALIGN(4);
  } >FLASH

  .crc_data :
  {
    KEEP(*(.crc_data*))
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : AT(ORIGIN(RAM))
  {
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  }

  _siccmram = LOADADDR(.ccmram);

  /* CCM-RAM section
  *
  * IMPORTANT NOTE!
  * If initialized variables will be placed in this section,
  * the startup code needs to be modified to copy the init-values.
  */
  .ccmram (NOLOAD) : AT(ORIGIN(CCMRAM))
  {
    _sccmram = .;       /* create a global symbol at ccmram start */
    *(.ccmram)
    *(.ccmram*)

    . = ALIGN(4);
    _eccmram = .;       /* create a global symbol at ccmram end */
  }

  /* Uninitialized data section */
  .bss (NOLOAD) : AT(ADDR(.data) + SIZEOF(.data))
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  }

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack (NOLOAD) : AT(ADDR(.bss) + SIZEOF(.bss))
  {
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(4);
  }

  /DISCARD/ :
  {
    *(.ARM.attributes)
  }
}


