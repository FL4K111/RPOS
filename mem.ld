OUTPUT_FORMAT(elf32-littleriscv)

MEMORY
{
    FLASH(rx) : ORIGIN = 0x10000000, LENGTH = 4M
    RAM(rwx) : ORIGIN = 0x20000000, LENGTH = 512k
    SCRATCH_X(rwx) : ORIGIN = 0x20080000, LENGTH = 4k
    SCRATCH_Y(rwx) : ORIGIN = 0x20081000, LENGTH = 4k
}

/* The bootrom will enter the image at the point indicated in your
   IMAGE_DEF, which is usually the reset handler of your vector table.

   The debugger will use the ELF entry point, which is the _entry_point
   symbol, and in our case is *different from the bootrom's entry point.*
   This is used to go back through the bootrom on debugger launches only,
   to perform the same initial flash setup that would be performed on a
   cold boot.
*/

ENTRY(_entry_point)

SECTIONS
{
    .text : {
        KEEP (*(.binary_info_header))
        KEEP (*(.embedded_block))
        KEEP (*(.reset))
        . = ALIGN(4);
        *(.text)
    } > FLASH   
    /*这里直接将.text段放入flash中，直接在flash中执行而不需要读入ram*/

    .rodata : {
        . = ALIGN(4);
        *(.rodata)
    } > FLASH

    . = ALIGN(4);
    __binary_info_start = .;
    .binary_info : {
        KEEP (*(.binary_info.keep.*))
        *(.binary_info.*)
    } > FLASH
    __binary_info_end = .;
    . = ALIGN(4);

    .data : {
        . = ALIGN(4);
        __data_start = .;
        *(.data)
        . = ALIGN(4);
        __data_end = .;
    } > RAM AT> FLASH

    __data_load_ptr = LOADADDR(.data); /*__data_start = .这类的赋值指的是内存中的位置，且只有>ram的段有（或者只有这种段是有意义的）*/

    .bss (NOLOAD) : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM

    .scratch_x : {
        . = ALIGN(4);
        __scratch_x_start = .;
        *(.scratch_x.*)
        . = ALIGN(4);
        __scratch_x_end = .;
    } > SCRATCH_X AT> FLASH

    __scratch_x_load_ptr = LOADADDR(.scratch_x);

    .scratch_y : {
        . = ALIGN(4);
        __scratch_y_start = .;
        *(.scratch_y.*)
        . = ALIGN(4);
        __scratch_y_end = .;
    } > SCRATCH_Y AT> FLASH

    __scratch_y_load_ptr = LOADADDR(.scratch_y);
    PROVIDE(__memory_start = ORIGIN(RAM));
    PROVIDE(__memory_end = ORIGIN(RAM) + LENGTH(RAM));
    PROVIDE(__heap_start = __bss_end);
    PROVIDE(__heap_size = __memory_end - __bss_end);
}

