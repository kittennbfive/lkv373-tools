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
	'5ef0'=>'bootloader?'
);

#block 2
our %known_funcs_bl2=(
	'1b018'=>'os_create_task(r1=callback)',
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
	'c90'=>'config_cpu_regs()',
	'7fc'=>'configure_icm_whatsthis()',
	'd34'=>'configure_mmu_and_unknown_stuff()',
	'1814c'=>'strcmp()?',
	'3f6a0'=>'TFEth_Init()',
	'1a980'=>'copy_some_stuff()?'
);

1;
