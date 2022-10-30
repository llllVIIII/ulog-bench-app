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
  /*
  * The section contents are declared with KEEP so that they are not removed
  * from the ELF. These are never emitted in the final binary or loaded into
  * memory.
  */
  .ulog_rodata 0x0 (INFO) :
  {
    KEEP(*(.ulog_rodata));
  }
}

INCLUDE "nrf_common.ld"
