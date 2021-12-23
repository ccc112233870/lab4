
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int		ticks;

EXTERN	int		disp_pos;
EXTERN	u8		gdt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	DESCRIPTOR	gdt[GDT_SIZE];
EXTERN	u8		idt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	GATE		idt[IDT_SIZE];

EXTERN	u32		k_reenter;

EXTERN	TSS		tss;
EXTERN	PROCESS*	p_proc_ready;


EXTERN  SEMAPHORE   reader_count_mutex; // reader_count的修改保护锁
EXTERN  SEMAPHORE   writer_count_mutex; //  writer_count的修改保护锁
EXTERN  SEMAPHORE   read_write_mutex;   // 读者写者互斥访问的资源
EXTERN  SEMAPHORE   max_reader_mutex;   // 限制读者个数的保护锁
EXTERN  SEMAPHORE   reader_queue_mutex;       //  writeFirst的互斥锁，用来给写进程开始后的第一个reader排队，后来的writer不用排这个队，即插队，优先级更高
EXTERN  SEMAPHORE   readers_queue_mutex;        // writeFirst的互斥锁，用来给写进程开始后的readers排队，后来的writer不用排这个队，即插队，优先级更高
EXTERN  SEMAPHORE   writers_queue_mutex;        // readFirst的互斥锁，用来给writer排队，后来的reader不用排这个队，即插队，优先级更高

EXTERN  SEMAPHORE   readers1;
EXTERN  SEMAPHORE   writers1;
EXTERN  SEMAPHORE   book1; 
EXTERN  SEMAPHORE   maxreaders1;
EXTERN  SEMAPHORE   writers22;
EXTERN  SEMAPHORE   readers2;

extern	PROCESS		proc_table[];
extern	char		task_stack[];
extern  TASK            task_table[];
extern	irq_handler	irq_table[];



