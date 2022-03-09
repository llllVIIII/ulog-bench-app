/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x100000
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x40000
}

SECTIONS
{
  .ulog_rodata 0x0 (INFO) :
  {
    KEEP(*(.ulog_rodata))
  }
}

INCLUDE "nrf_common.ld"
