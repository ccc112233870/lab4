
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{	
	p_proc_ready++;
	for(;;	p_proc_ready++){
		if (p_proc_ready >= proc_table + NR_TASKS){
			p_proc_ready = proc_table;
		}
		if (p_proc_ready->LastVisit == 0 && p_proc_ready->isStop == FALSE){
			break;
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}



/*======================================================================*
                           sys_milli_sleep
 *======================================================================*/
PUBLIC void sys_milli_sleep(int mill_seconds){
	p_proc_ready->LastVisit = mill_seconds*HZ/1000;
	schedule();

}



/*======================================================================*
                           sys_print
 *======================================================================*/

PUBLIC void sys_my_disp_str(char *str){
	if(disp_pos >= 160*25){
		disp_pos = 0;
		for(int i = 0; i<=160*25; i++){
			disp_str(" ");
		}
		disp_pos = 0;
	}
	disp_str(str);
}


/*======================================================================*
                           sys_print_color
 *======================================================================*/

PUBLIC void sys_print_color(char *str,int color){
	if(disp_pos >= 160*25){
		disp_pos = 0;
		for(int i = 0; i<=160*25; i++){
			disp_str(" ");
		}
		disp_pos = 0;
	}
	disp_color_str(str,color);
}

/*======================================================================*
                           sys_sem_p
 *======================================================================*/

PUBLIC void sys_sem_p(SEMAPHORE *s){
	s->value--;
	if (s->value<0)
	{
		p_proc_ready->isStop = TRUE;
		s->queue[s->rear] = p_proc_ready;
		s->rear = (s->rear+1)%List_size;
		schedule();
	}
}


/*======================================================================*
                           sys_sem_v
 *======================================================================*/

PUBLIC void sys_sem_v(SEMAPHORE *s){
	s->value++;
	if (s->value<=0)
	{
		s->queue[s->front]->isStop = FALSE;
		p_proc_ready = s->queue[s->front];
		s->queue[s->front] = NULL;
		s->front = (s->front+1)%List_size;
	}
}