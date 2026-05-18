/*******************************************************************************
  Linker script for SAMD20 bootloader (prime_sn_boot_external_mem)

  - ROM: first 4 KB of flash (0x00000000..0x00000FFF). BOOTPROT=3 protects
    this region at hardware level when the fuse is active.
  - RAM: full 32 KB (0x20000000..0x20007FFF). Stack at the top.
  - The application occupies flash from 0x00001000 to 0x0003FEFF and is a
    separate binary with its own linker script.
  - The emulated EEPROM row (0x0003FF00..0x0003FFFF) must never be touched
    by the bootloader; it is reserved via the NVMCTRL EEPROM[2:0] fuse.
*******************************************************************************/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(Reset_Handler)

MEMORY
{
    rom  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00001000   /* 4 KB */
    ram  (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00008000   /* 32 KB */
}

/* Top of RAM — initial MSP value. Stack grows downward from here. */
_estack = ORIGIN(ram) + LENGTH(ram);

SECTIONS
{
    /* Vector table at flash 0x00000000. Must be aligned to 256 B for
     * SCB->VTOR in Cortex-M0+ (see startup_samd20_bootloader.c). */
    .vectors : ALIGN(256)
    {
        KEEP(*(.vectors))
        KEEP(*(.vectors.*))
    } > rom

    /* Code and read-only data */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        *(.glue_7)
        *(.glue_7t)
        . = ALIGN(4);
        _etext = .;
    } > rom

    /* ARM exception unwind info (usually empty for bare-metal, but keep
     * the sections so the linker does not warn). */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > rom

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > rom

    /* .data: initial values live in flash (LMA) and are copied to RAM
     * (VMA) by the reset handler. Normally empty for the bootloader. */
    _sidata = LOADADDR(.data);

    .data : ALIGN(4)
    {
        _sdata = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        _edata = .;
    } > ram AT > rom

    /* .bss: zero-initialized RAM region. Cleared by the reset handler. */
    .bss (NOLOAD) : ALIGN(4)
    {
        _sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > ram

    /* Do NOT define a .stack section: the stack uses whatever RAM is left
     * above .bss, up to _estack. For the bootloader this is >30 KB. */

    /DISCARD/ :
    {
        *(.note.GNU-stack)
        *(.ARM.attributes)
    }
}
