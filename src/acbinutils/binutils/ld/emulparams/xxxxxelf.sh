SCRIPT_NAME=elf
OUTPUT_FORMAT=`"elf32-'___arch_name___`"'
TEXT_START_ADDR=0x1000
TEMPLATE_NAME=elf32
GENERATE_SHLIB_SCRIPT=yes
MAXPAGESIZE=0x1000
ARCH=___arch_name___
ENTRY="_start"
EMBEDDED=yes
OTHER_BSS_SYMBOLS='__bss_start__ = .;'
OTHER_BSS_END_SYMBOLS='_bss_end__ = . ; __bss_end__ = . ; __end__ = . ;'
