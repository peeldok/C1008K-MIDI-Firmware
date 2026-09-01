/* Linker script for AT32F405RCT7-7 : 256 KB FLASH @ 0x08000000, 96 KB SRAM @ 0x20000000
 * Keychron C100 firmware.
 *
 * Top 16 bytes of RAM are reserved so the C100 ROM-DFU magic word at 0x20017FF0
 * survives a warm reset and is never used as ordinary data/stack.
 */

ENTRY(Reset_Handler)

_estack = 0x20017FF0;          /* stack top: just below the reserved DFU magic word */
_dfu_magic_addr = 0x20017FF0;  /* C100 ROM-DFU request word ("DFU!" = 0x44465521) */

_Min_Heap_Size  = 0x400;       /* newlib/_sbrk reserve (FreeRTOS heap_4 is a separate static array) */
_Min_Stack_Size = 0x800;       /* MSP stack used before scheduler start + all ISRs */

/* Top two 2 KB sectors are reserved for firmware/nvs.c and kept out of the image:
 *   0x0803F000  settings store
 *   0x0803F800  custom palettes */
_nvs_settings_addr = 0x0803F000;
_nvs_palettes_addr = 0x0803F800;

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 256K - 4K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 96K - 16
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text) *(.text*)
    *(.glue_7) *(.glue_7t)
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata) *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  _sidata = LOADADDR(.data);
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data) *(.data*)
    *(.RamFunc) *(.RamFunc*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH

  . = ALIGN(4);
  .bss :
  {
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss) *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
