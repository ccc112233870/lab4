# lab4

## 1. 系统调用封装：

### （1） disp_str封装：

​		在**syscall.asm**中，添加函数：my_disp_str:

```
my_disp_str:
	mov	eax, _NR_disp_str
	mov ebx, [esp+4]
	int	INT_VECTOR_SYS_CALL
	ret

```

​	同时，添加全局调用，以及调用的系统函数号:

```
_NR_disp_str		equ 2 ;

global  my_disp_str
```

### （2） PV系统调用添加

​		同（1）中添加方式，首先在**syscal.asm**中添加相应函数



### （3） 系统调用函数定义：

​		在**proto.h**中定义，要注意引入**proc.h**以方便参数使用proc中的**SEMAPHONE**结构体，具体定义如下：

```c
/* proc.c */
PUBLIC  int     sys_get_ticks();        /* sys_call */
PUBLIC void     sys_my_delay(int mills);
PUBLIC void     sys_my_disp_str(char * str);
PUBLIC  void    sys_my_disp_color_str(char * str, int color);
PUBLIC void     sys_sem_p(SEMAPHORE *s);
PUBLIC void     sys_sem_v(SEMAPHORE *s);


/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
PUBLIC  void    my_delay(int mills);
PUBLIC  void    my_disp_str(char * str);
PUBLIC  void    my_disp_color_str(char * str, int color);
PUBLIC  void    P(SEMAPHORE *s);
PUBLIC  void    V(SEMAPHORE *s);
```



### （4） 函数实现：

​		在**global.c**中函数列表里添加系统调用函数：

```c
PUBLIC	system_call		sys_call_table[NR_SYS_CALL] ={sys_get_ticks,sys_my_delay,sys_my_disp_str,sys_my_disp_color_str,sys_sem_p,sys_sem_v};
```

​		记得修改NR_SYS_CALL大小。

​		在**proc.c**中实现系统调用函数：

```c
//sys_my_delay
PUBLIC void sys_milli_sleep(int mill_seconds){
	p_proc_ready->LastVisit = mill_seconds*HZ/1000;
	schedule();

}
```

```c
//sys_my_disp_str
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
```

```c
//sys_my_disp_color_str
PUBLIC void sys_my_disp_color_str(char *str,int color){
	if(disp_pos >= 160*25){
		disp_pos = 0;
		for(int i = 0; i<=160*25; i++){
			disp_str(" ");
		}
		disp_pos = 0;
	}
	disp_color_str(str,color);
}
```

```c
//P
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
```

```c
//V
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
```

## 2. main函数部分调用：

读优先，写优先函数：

```c
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
		}
		V(rc_mutex);
		V(maxreaders);
		//饿死
		milli_delay(2*10000);
	}
}
```

```c
void writerF(PROCESS *p){
	while(1){
		startdisp(p);
		P(wc_mutex);
				writercount++;
				if (writercount == 1)
				{
					P(rc_mutex);
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
				}
		V(wc_mutex);
		milli_delay(3*10000);
	}
}
```

正常读，正常写函数：

```c
void reader(PROCESS *p){
	while(1){
		startdisp(p);
		P(maxreaders);
					P(rc_mutex);
					readercount++;
					if (readercount == 1)
					{
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
```

```c
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

```

