SECTIONS
{
.resource_table_ucie0 : {
    __resource_table_ucie0_start = .;
    . = ALIGN(4);
    KEEP(*(.resource_table_ucie0))
    __resource_table_ucie0_end = .;
} > remote_proc_rsc_table

}
