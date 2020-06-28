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
	'252c8'=>'some_output_func()',
	'2cf50'=>'uart_putchar()',
	'35668'=>'programm_memory()?',
	'14df8'=>'putstr()?',
	'14edc'=>'some_output_func()',
	'1595c'=>'strlen(r0)?',
	'70cc'=>'post_http()?',
	'15754'=>'strcmp()',
	'730c'=>'httpd_respond_to_request',
	'5c24'=>'trigger_reboot()',
	'5ef0'=>'print_header_register_tasks',
	'15ad8'=>'register task',
	'8c60'=>'uart_putchar(r0)',
	'39f94'=>'EthernetTask',
	'6400'=>'ConsoleTask',
	'88e8'=>'HTTPTask',
	'393b0'=>'MAC_transmit()',
	'155cc'=>'some_copy_loop',
	'1464'=>'OS_MemClr (memset to 0)',
	'18178'=>'NetTask',
	'5434'=>'OSInit'
);

#block 2
our %known_funcs_bl2=(
	'1b018'=>'selfmade_create_task(r1=callback)', #calls OSTaskCreateExt
	'7bf4'=>'some_init_register_main()',
	'de0'=>'call_init_and_stop()',
	'39ddc'=>'enable_interrupts()',
	'3a944'=>'wait_for_uart_ready()',
	'75c0'=>'TF_MAIN_Task',
	'6564'=>'TF_AV_Task',
	'cbd8'=>'TF_Console_Task',
	'ee54'=>'TF_HTTP_Task',
	'7330'=>'TF_TFVEP_Task',
	'1d994'=>'NetTask',
	'3edc4'=>'Task_unknown1', #might trigger send of UDP-status-packets (?)
	'5dd8'=>'Task_unknown2', #sends out the packets (?)
	'cdc0'=>'some_output_func()',
	'324f8'=>'uart_putchar()',
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
	'1bc0'=>'OS_TaskIdle()',
	'53e4'=>'OSInit()',
	'1414'=>'OS_MemClr() (memset to 0)',
	'1008'=>'OS_SchedNew', #OS-internal
	'fe0'=>'OSIntEnter()',
	'5918'=>'OSIntCtxSw()',
	'1830'=>'OSTimeDly()',
	'1bf4'=>'OSTaskSuspend()',
	'1d50'=>'OS_TaskStatStkChk()',
	'1fe4'=>'OS_TCBInit()',
	'17ac'=>'OS_Sched()', #OS-internal
	'58ca'=>'OS_TASK_SW()',
	'4308'=>'OSTaskCreate()',
	'43b4'=>'OS_TaskStkClr()',
	'5adc'=>'OSTaskStkInit()',
	'442c'=>'OSTaskCreateExt()',
	'3d44'=>'OSSemCreate()',
	'13dc'=>'OS_EventWaitListInit()',
	'1628'=>'OSTaskNameSet()',
	'5140'=>'OSTmr_Init()',
	'14c4'=>'OS_QInit()',
	'1510'=>'OS_MemInit()',
	'1590'=>'OS_FlagInit()',
	'1278'=>'OSVersion()',
	'3eec8'=>'TX_packet()',
	'3d14'=>'OSSemAccept()',
	'10dc'=>'OSSchedLock()',
	'1120'=>'OSStart()',
	'3ea8'=>'OSSemPend()',
	'4028'=>'OSSemPost()',
	'4094'=>'OSSemQuery()??',
	'4a84'=>'OSTimeDlyResume()',
	'4b58'=>'OSTimeGet()',
	'4b76'=>'OSTimeSet()',
	'48f4'=>'OSTimeDlyHMSM()',
	'1178'=>'OSTimeTick()',
	'e10'=>'OS_StrCopy()',
	'12bc'=>'OS_EventTaskRdy()??', #OS-internal
	'1284'=>'OS_EventTaskRemove()', #OS-internal
	'fa0'=>'OS_EventTaskRemoveMulti()', #OS-internal
	'e30'=>'OSEventNameGet()',
	'ea4'=>'OSEventNameSet()',
	'e8c'=>'OS_StrLen()',
	'f10'=>'OS_EventTaskWaitMulti()', #OS-internal
	'1368'=>'OS_EventTaskWait()', #OS-internal
	'16c0'=>'OS_MemCopy()',
	'18ac'=>'OSStatInit()',
	'1918'=>'OSSchedUnlock()',
	'1978'=>'OSEventPendMulti()',
	'2138'=>'OSFlagAccept()',
	'21fc'=>'OSFlagCreate()',
	'2338'=>'OS_FlagBlock()',
	'36f4'=>'OSQCreate()',
	'37a4'=>'OSQDel()',
	'38c8'=>'OSQFlush()',
	'38f0'=>'OSQPend()',
	'3ab4'=>'OSQPost()',
	'3b3c'=>'OSQPostFront()',
	'19ff0'=>'putchar()',
	'19f8c'=>'putchar_int()',
	'598c'=>'ISR_Timer',
	'103c'=>'OSIntExit()?'
);

1;
