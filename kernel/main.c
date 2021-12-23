
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
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

//测试变量
SEMAPHORE *book;
SEMAPHORE *maxreaders;
SEMAPHORE *readers;
SEMAPHORE *readersmutx;
SEMAPHORE *writers;
SEMAPHORE *writers2;
int readerfirst;
int readercount;
int writercount;

//变量
SEMAPHORE *rc_mutex;
SEMAPHORE *wc_mutex;
SEMAPHORE *rw_mutex;
SEMAPHORE *max_r_mutex;
SEMAPHORE *readers;
SEMAPHORE *readers_queue;
SEMAPHORE *writers_queue; 
int MAX_READER_NUM;
int reader_count;
int writer_count;
int WRITE_FIRST;
char str[] = "A start!\n";
/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");
	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	// 初始化proc_table
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_proc->isStop = FALSE;
		p_proc->LastVisit = 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}
	clean();

	proc_table[0].color = 0xC;
	proc_table[1].color = 0xD;
	proc_table[2].color = 0xE;
	proc_table[3].color = 0x1;
	proc_table[4].color = 0x2;
	proc_table[5].color = 0x00;


	proc_table[0].timepiece = 20000;
	proc_table[1].timepiece = 30000;
	proc_table[2].timepiece = 30000;
	proc_table[3].timepiece = 30000;
	proc_table[4].timepiece = 40000;
	proc_table[5].timepiece = 10000;

	book = &book1;
 	book->front = 0;
	book->rear = 0;
	book->value = 1;

	readers = &readers1;
	readers->front = readers->rear = 0;
	readers->value = 1;

	writers = &writers1;
	writers->front = writers->rear = 0;
	writers->value = 1; 

	maxreaders = &maxreaders;
	maxreaders->value = 3;
	maxreaders->front = 0;
	maxreaders->rear = 0; 

	writers2 = &writers22;
	writers2->value = 1;
	writers2->front = writers2->rear = 0;

	readersmutx = &readers2;
	readersmutx->value = 1;
	readersmutx->front = readersmutx->rear = 0;


	// 写优先的读者队列信号量
	writers_queue = &writers_queue_mutex;
	writers_queue->value = 1;
	writers_queue->front = 0;
	writers_queue->rear = 0;

	readers_queue = &readers_queue_mutex;
	readers_queue->value = 1;
	readers_queue->front = 0;
	readers_queue->rear = 0;

	
	// reader_count一次只能有一个reader访问
	rc_mutex = &reader_count_mutex;
	rc_mutex->value = 1;
	rc_mutex->front = 0;
	rc_mutex->rear = 0;


	// writer_count一次只能有一个reader访问
	wc_mutex = &writer_count_mutex;
	wc_mutex->value = 1;
	wc_mutex->front = 0;
	wc_mutex->rear = 0;

	//读者优先
	readerfirst = 1; 
	readercount = 0;
	writercount = 0;

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table+3;
	
	

	/* 初始化 8253 PIT */
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
	enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

	restart();

	while(1){}
}

void clean(){
	disp_pos = 0;
	for (int i = 0; i < 80*25; i++)
	{
		disp_str(" ");
	}
	disp_pos = 0;
}


//部分输出
void reader_reading_str(PROCESS *p){
	char str[20] = "A ";
	str[0] = str[0] + p->pid;
	strcpy(str + 2,"is reading! \n");
	print_color(str,p->color);
}

void reader_end_str(PROCESS *p){
	char str[20]="A ";
	str[0] = str[0] + p->pid;
	strcpy(str + 2,"ended reading!\n");
	print_color(str,p->color);
}

void writer_start_str(PROCESS *p){
	char str[20] = "A ";
	str[0] = str[0] + p->pid;
	strcpy(str + 2, "is writing! \n");
	print_color(str,p->color);
}

void writer_end_str(PROCESS *p){
	char str[20] = "A ";
	str[0] = str[0] + p->pid;
	strcpy(str + 2, "ended writing! \n");
	print_color(str,p->color);
}

void startdisp(PROCESS *p){
	char str[] = "A start!\n";
	str[0] = str[0]+p->pid;
	print_color(str,p->color);
}

void Reader1(PROCESS *p){
	while(1){
		char *str = "";
		strcpy(str," isz");
		
		//print_color(p->p_name,p->color);
		print_color("A is ready!",p->color);
		//print_color("\n",p->color);
		P(maxreaders);
		if(readerfirst){
			P(readers);
				P(readersmutx);
				readercount++;
				if(readercount == 1){
					P(book);
				}
				V(readersmutx);
			V(readers);
		}
		else{
			P(readersmutx);
			readercount++;
			if(readercount == 1){
				P(book);
			}
			V(readersmutx);
		}

		//print_color(p->p_name,p->color);
		print_color("A is reading!",p->color);
		//print_color("\n",p->color);
		milli_delay(p->timepiece);
		//print_color(p->p_name,p->color);
		print_color("stop reading!",p->color);
		//print_color("\n",p->color);

		P(readersmutx);
		readercount--;
		if(readercount==0){
			V(book);
		}
		V(readersmutx);
		V(maxreaders);
		milli_delay(2*1000);

	}
}
 
 void Writer1(PROCESS *p){
	while(1){
		//print_color(p->p_name,p->color);
		print_color("is ready!",p->color);
		//print_color("\n",p->color);
		if (!readerfirst)
		{
			P(writers);
			writercount++;
			if (writercount == 1)
			{
				P(readers);
			}
			V(writers);
		}
		else
		{			
			P(writers);
				writercount++;
			V(writers);
			P(writers2);
		}
		
		P(book);
		//print_color(p->p_name,p->color);
		print_color("is writing!",p->color);
		//print_color("\n",p->color);
		milli_delay(p->timepiece);
		//print_color(p->p_name,p->color);
		print_color("stop writing!",p->color);
		//print_color("\n",p->color);
		V(book);

		if (!readerfirst)
		{
			P(writers);
				writercount--;
			if (writercount == 0)
			{
				V(readers);
			}
			V(writers);
		}
		else
		{
			P(writers);
				writercount--;
			V(writers);
			V(writers2);
		}

		// 一定程度上防止饿死，让进程执行完后休息一下（防累死）
		milli_delay(3*1000);
	}
}

void readerF(PROCESS *p){
	while(1){
		startdisp(p);
		P(maxreaders);
		P(rc_mutex);
			readercount++;
			if (readercount == 1)
			{
				P(wc_mutex);
			}
		V(rc_mutex);
		reader_reading_str(p);
		milli_delay(p->timepiece);
		reader_end_str(p);
		P(rc_mutex);
		readercount--;
		if (readercount == 0)
		{
			V(wc_mutex);
			//V(book);
		}
		V(rc_mutex);
		V(maxreaders);
		//饿死
		milli_delay(2*10000);
	}
}

void reader(PROCESS *p){
	while(1){
		startdisp(p);
		P(maxreaders);
					P(rc_mutex);
					readercount++;
					if (readercount == 1)
					{
						//
						P(wc_mutex);
						P(book);
					}
					V(rc_mutex);
		reader_reading_str(p);
		milli_delay(p->timepiece);
		reader_end_str(p);

		P(rc_mutex);
		readercount--;
		if (readercount == 0)
		{
			V(wc_mutex);
			V(book);
		}
		V(rc_mutex);
		V(maxreaders);

		//饿死
		milli_delay(2*10000);
	}
}


void writerF(PROCESS *p){
	while(1){
		startdisp(p);
		P(wc_mutex);
				writercount++;
				if (writercount == 1)
				{
					P(rc_mutex);
					//P(readers);
				}
		V(wc_mutex);
		P(book);
			writer_start_str(p);
			milli_delay(p->timepiece);
			writer_end_str(p);
		V(book);
		P(wc_mutex);
				writercount--;
				if (writercount == 0)
				{
					V(rc_mutex);
					//V(readers);
				}
		V(wc_mutex);
		milli_delay(3*10000);
	}
}

void writer(PROCESS *p){
	while(1){
		startdisp(p);
		P(wc_mutex);
		writercount++;
		V(wc_mutex);
		P(book);
			writer_start_str(p);
			milli_delay(p->timepiece);
			writer_end_str(p);
		V(book);
		P(wc_mutex);
			writercount--;
		V(wc_mutex);
		milli_delay(3*10000);
	}
}

/* //reader
void Reader(PROCESS *p){
	while (1) {
	//	processReady(p->pid);
		P(maxreaders);
		if (WRITE_FIRST)
		{
			// 如果有进程已经申请了写，后续的第一个读请求就会堵塞在readers，其他的读请求就会堵塞在readers_queue
			// readers_queue的作用是让readers最多只有一个reader排队，后来的读请求就可以插队到readers里面
			P(readers_queue);
				P(readers);
					P(rc_mutex);
					reader_count++;
					if (reader_count == 1)
					{
						// 读过程中禁止写, 而其他读的进程不会被堵塞
						P(book);
					}
					V(rc_mutex);
				V(readers);
			V(readers_queue);
		}
		else
		{
			P(rc_mutex);
			reader_count++;
			if (reader_count == 1)
			{
				// 读过程中禁止写, 而其他读的进程不会被堵塞
				P(book);
			}
			V(rc_mutex);
		}

		my_disp_str(p);
		milli_delay(p->timepiece);
		my_disp__end_str(p);

		P(rc_mutex);
		reader_count--;
		if (reader_count == 0)
		{
			V(book);
		}
		V(rc_mutex);
		V(maxreaders);

		//饿死
		milli_delay(2*10000);
		
	}
}

//writer
void Writer(PROCESS *p){
	while(1){
		startdisp(p);
		if (WRITE_FIRST)
		{
			P(wc_mutex);
			writer_count++;
			if (writer_count == 1)
			{
				P(readers);
			}
			V(wc_mutex);
		}
		else
		{
			P(wc_mutex);
			writer_count++;
			V(wc_mutex);
			P(writers_queue);
		}
		
		P(book);
		my_disp_str(p);
		milli_delay(p->timepiece);
		my_disp__end_str(p);
		V(book);

		if (WRITE_FIRST)
		{
			P(wc_mutex);
			writer_count--;
			if (writer_count == 0)
			{
				V(readers);
			}
			V(wc_mutex);
		}
		else
		{
			P(wc_mutex);
			writer_count--;
			V(wc_mutex);
			V(writers_queue);
		}
		//饿死
		milli_delay(3*10000);
	}
} */


void TESTA()
{
	//Reading_Process(0,proc_table[0].timepiece);
	//Reader(proc_table);
	//readerF(proc_table);
	reader(proc_table);
}


void TESTB()
{
	//Reading_Process(1,proc_table[1].timepiece);
	//readerF(proc_table+1);
	reader(proc_table+1);
}


void TESTC()
{
	//Reading_Process(2,proc_table[2].timepiece);
	//readerF(proc_table+2);
	reader(proc_table+2);
}


void TESTD()
{
	//Writing_Process(3,proc_table[3].timepiece);
	//writer(proc_table+3);
	writerF(proc_table+3);
}


void TESTE()
{
	//Writing_Process(4,proc_table[4].timepiece);	
	//writer(proc_table+4);
	writerF(proc_table+4);
}

void TESTF()
{
	
	while(1){
		if (readercount == 0 && book->value != 1)
		{
			char str[] = "1 writer\n";
			sys_my_disp_str(str);
		}
		else if (readercount > 0)
		{
			char str[] = "0 READER(S)\n";
			str[0] = str[0]+readercount;
			sys_my_disp_str(str);
		}
		milli_sleep(proc_table[5].timepiece);
	}
}