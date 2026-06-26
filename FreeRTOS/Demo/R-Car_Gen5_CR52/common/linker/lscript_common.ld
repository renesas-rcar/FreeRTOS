MEMORY
{
   sram_base_addr : ORIGIN = 0x00000000, LENGTH = 0x2000000
   vram1_base_addr : ORIGIN = 0x10200000, LENGTH = 0x1400000
   vram2_base_addr : ORIGIN = 0x11600000, LENGTH = 0xA00000
   vram3_base_addr : ORIGIN = 0x1160000, LENGTH = 0xA00000
   dram_base_addr : ORIGIN = 0x40000000, LENGTH = 0xA00000
   dram2_base_addr : ORIGIN = 0x4B000000, LENGTH = 0x100000

   remote_proc_rsc_table : ORIGIN = 0x96600000, LENGTH = 0x1000
   remote_proc_rsc_table_1 : ORIGIN = 0x96650000, LENGTH = 0x1000
   remote_proc_rsc_table_2 : ORIGIN = 0x966a0000, LENGTH = 0x1000
   remote_proc_rsc_table_aiacc0_0 : ORIGIN = 0x90000000, LENGTH = 0x1000
   remote_proc_rsc_table_aiacc0_1 : ORIGIN = 0x90050000, LENGTH = 0x1000
   remote_proc_rsc_table_aiacc1_0 : ORIGIN = 0x90200000, LENGTH = 0x1000
   remote_proc_rsc_table_aiacc1_1 : ORIGIN = 0x90250000, LENGTH = 0x1000
}

/* --- BSP Configuration Symbols --- */
/* 0 = Enable logs output (Default), 1 = Disable logs output */
__CONFIG_SILENT_CONSOLE__ = 0;

ENTRY(_Reset)
