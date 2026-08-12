SECTIONS
{
    .resource_table : {
        __resource_table_start = .;
        . = ALIGN(4);
        KEEP(*(.resource_table))
        __resource_table_end = .;
    } > remote_proc_rsc_table_ucie_0_0
}
