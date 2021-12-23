
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"


PUBLIC	PROCESS			proc_table[NR_TASKS];

PUBLIC	char			task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK	task_table[NR_TASKS] = {{TESTA, STACK_SIZE_TESTA, "A"},
					{TESTB, STACK_SIZE_TESTB, "B"},
					{TESTC, STACK_SIZE_TESTC, "C"},
                    {TESTD, STACK_SIZE_TESTD, "D"},
                    {TESTE, STACK_SIZE_TESTE, "E"},
                    {TESTF, STACK_SIZE_TESTF, "F"}};

PUBLIC	irq_handler		irq_table[NR_IRQ];

PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_milli_sleep ,sys_my_disp_str,sys_print_color,sys_sem_p,sys_sem_v};

