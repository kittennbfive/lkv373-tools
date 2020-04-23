=pod
(c) 2020 by kitten_nb_five

AGPL v3 or later

NO WARRANTY!
=cut

package known_funcs;

use Exporter;
our @ISA='Exporter';
our @EXPORT=qw(%known_funcs_bl1 %known_funcs_bl2); #Functions and variables which are exported by default

#block 1
our %known_funcs_bl1=(
	'65f0'=>'printf()',
	'6ad8'=>'printf2()',
	'156b0'=>'helper_printf_strcpy??',
	'344c0'=>'helper_printf',
	'3484c'=>'helper2_printf',
	'2cf50'=>'uart_putchar()',
	'6400'=>'console_main?',
	'35668'=>'programm_memory()?',
	'14df8'=>'putstr()?',
	'14edc'=>'some_output_func()',
	'1595c'=>'strlen(r0)?',
	'70cc'=>'post_http()?',
	'15754'=>'strcmp()',
	'730c'=>'httpd_respond_to_request',
	'5c24'=>'trigger_reboot()',
	'5ef0'=>'bootloader?',
	'15ad8'=>'create/register task',
	'8c60'=>'uart_putchar(r0)'
);

#block 2
our %known_funcs_bl2=(
	'1b018'=>'os_create_task(r1=callback, returns 0 on error(?))',
	'7bf4'=>'some_init_register_main()',
	'de0'=>'call_init_and_stop()',
	'39ddc'=>'enable_interrupts()',
	'3a944'=>'wait_for_uart_ready()',
	'75c0'=>'TF_MAIN_Task',
	'6564'=>'TF_AV_Task',
	'cbd8'=>'TF_Console_Task',
	'ee54'=>'TF_HTTP_Task',
	'7330'=>'TF_TFVEP_Task',
	'cdc0'=>'some_output_func()',
	'324f8'=>'uart_putchar()',
	'36f4'=>'create_pipe()??',
	'197e4'=>'print_console_header()',
	'c90'=>'config_cpu_regs_set_SP_memset_0()',
	'7fc'=>'flush_some_cache()',
	'4dc'=>'copy_ISR_code()',
	'd34'=>'flush_some_cache_copy_ISR_code()',
	'1814c'=>'strcmp()?',
	'3f6a0'=>'TFEth_Init()',
	'1a980'=>'copy_some_stuff()?',
	'324e4'=>'uart_getchar()',
	'81c4'=>'some hardware-writes (PMU, unknown)',
	'8134'=>'INTC_setup()',
	'b9e0'=>'something_with_FLASH()',
	'd2a8'=>'some_output_func()',
	'1bc0'=>'idle task?????'
);

1;
