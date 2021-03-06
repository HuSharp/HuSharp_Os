# HuSharp_OS
HuSharp's little os

参考《操作系统真象还原》
豆瓣链接 https://book.douban.com/subject/26745156/

##代码结构
```c
FILE: boot/mbr.asm
    | 加载loader，并跳转
    |
    |
FILE: boot/loader.asm
    | 调用BIOS中断获取内存大小；构建GDT，进入保护模式；加载kernel；创建页目录PD和页表PT，开启分页机制；
    | 解析kernel的ELF，将ELF文件中的段segment拷贝到各段自己被编译的虚拟地址处；跳转
    |
    |
FILE: kernel/main.c
    |
    |---- FILE: lib/print.S       put_str("i am kernel\n");
    |       | 打印提示信息
    |       | 汇编实现打印函数(写入显存0xb_8000，loader阶段设为GDT第3号)，C语言调用
    |       |---- FILE: kernel/print.S		put_char()
    |       |       | 把栈中的1个字符写入光标所在处
    |       |       | 利用视频段选择子 	SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0
    |       |       | 实质上，打印字符就是把字符写到显存的某个位置
    |
    |---- FILE: kernel/init.c       init_all()
    |       | 初始化
    |       |
    |       |---- FILE: kernel/interrupt.c  idt_init() 
    |       |       | 初始化中断   
    |       |       | 构建IDT，这里IDT中的每一项都指向对应的一段汇编代码，再由汇编调用C语言中断处理函数
    |       |       | 初始化可编程中断控制器8259A，放开时钟中断0x20
    |       |       | 中断发生时，会根据IDTR中的IDT基地址+中断向量*8，跳转到对应的汇编代码
    |       |       |           
    |       |       |---- FILE: kernel/core_interrupt.asm
    |       |       |
    |       |       |---- FILE: lib/io.h
    |       |               | 内联汇编实现的读写端口函数
    |       |               | 凡是包含io.h的文件，都会获得一份io.h中所有函数的拷贝
    |       |               | inline是建议处理器将函数编译为内嵌方式，即在该函数调用处原封不动地展开
    |       |
    |       |---- FILE: kernel/memory.c     mem_init() 
    |       |       | 初始化内存管理系统
    |       |       | mem_pool_init()初始化内存池: 内核虚拟地址内存池、内核/用户物理内存池
    |       |       |     虚拟地址内存池: 虚拟地址bitmap、虚拟地址内存池起始地址
    |       |       |     物理内存池：物理内存bitmap、物理内存起始地址、物理内存池大小
    |       |       |    
    |       |       |---- FILE: lib/bitmap.c    bitmap_init()  bitmap_scan()  bitmap_set()
    |       |       |       | bitmap的基本操作
    |       |       |       |
    |       |       |       |---- FILE: lib/string.c    memset()
    |       |       |               | 内存/字符串的基本操作
    |       |       |    
    |       |       |---- FILE: kernel/debug.h      ASSERT()  PANIC()
    |       |               | 断言
    |       |               |
    |       |               |---- FILE: kernel/debug.c      panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)
    |       |                       | 关中断，打印相关信息，while(1)使程序悬停
    |       |                       |
    |       |                       |---- FILE: kernel/interrupt.c      intr_disable();
    |       |                               | 关中断
    |       |
    |       |---- FILE: thread/thread.c     thread_init()
    |       |       | 初始化线程相关结构
    |       |       | list_init()初始化就绪队列、初始化全部队列
    |       |       | make_main_thread()将当前内核main函数创建为线程
    |       |       |     running_thread()通过esp得到PCB基地址(main线程的PCB在loader阶段已分配)
    |       |       |     init_thread()在PCB中初始化线程基本信息: 状态 优先级 嘀嗒时间数 ...
    |       |       |     list_append()加入全部队列
    |       |       |     
    |       |       |---- FILE: list/list.c
    |       |               | 结点: struct list_elem{struct list_elem *prev; struct list_elem *next; };
    |       |               | 链表: struct list{struct list_elem head; struct list_elem tail; };
    |       |               | 对链表的一些修改操作需关中断来确保原子操作
    |       |       
    |       |---- FILE: device/timer.c      timer_init()
    |       |       | frequency_set()初始化可编程定时计时器8253
    |       |       |     使用8253来给IRQ0引脚上的时钟中断信号“提速”，使其发出的中断信号频率快一些。
    |       |       |     默认的频率是18.206Hz，即一秒内大约发出18 次中断信号。
    |       |       |     通过对8253编程，使时钟一秒内发100次中断信号，即中断信号频率为100Hz.
    |       |       |    
    |       |       |---- FILE: kernel/interrupt.c      register_handler(0x20, intr_timer_handler)
    |       |       |       | 注册安装中断处理程序 idt_table[vector_id] = function
    |       |       |       | 时钟的中断向量号0x20
    |       |       |       
    |       |       |---- FILE: device/timer.c      intr_timer_handler()
    |       |               | 时钟的中断处理程序
    |       |               | running_thread()通过esp得到PCB基地址
    |       |               | if(current_thread->ticks == 0)  
    |       |               |     schedule();           // 若进程时间片用完，就开始调度新的进程上CPU
    |       |               | else
    |       |               |     current_thread->ticks--;  // 将当前进程的时间片-1
    |       |               |
    |       |               |---- FILE: thread/thread.c     schedule()
    |       |                       | 任务调度 (由时钟中断触发)
    |       |                       | *** 调度 ****************
    |       |                       | 若状态为RUNNING, 则添加到就绪队列尾并重置ticks、状态改为READY
    |       |                       | 就绪队列第一个就绪线程弹出, 由结构体成员地址得到首地址, 这里即为PCB基地址
    |       |                       | 修改状态为RUNNING
    |       |                       |
    |       |                       |---- FILE: thread/switch.asm   switch_to(current_thread, next);
    |       |                               | 切换栈、切换执行流eip
    |       |                               | *********************************************
    |       |                               | 此处的理解需结合线程栈信息struct thread_stack和函数thread_create()
    |       |                               | *********************************************
    |       |                               | 保存current线程的寄存器, 将next线程的寄存器装载到处理器
    |       |                               | 传入的2个参数自动压入了current栈中, 这2个参数为2线程PCB基地址
    |       |                               | PCB底部为线程/进程信息struct task_struct, 第一个成员为栈顶地址
    |       |                               | 切换栈: 伪代码 mov [current], esp; mov esp, [next];
    |       |                               | 切换执行线程: ret。这里利用了ret的特性, 自动从栈弹出给eip
    |       |                               |         栈中存放eip处, 已由函数thread_create赋值为kernel_thread()
    |       |                               |
    |       |                               |---- FILE: thread/thread.c     kernel_thread(function, func_arg)
    |       |                                       | 执行创建线程时指定的函数
    |       |                                       | 这里2个参数的值已由函数thread_create赋值在栈中
    |       |                                       | intr_enable()开中断, 避免时钟中断被屏蔽而无法调度其它线程    
    |       |                                       | function(func_arg)执行新线程，
    |       |
    |       |---- FILE: device/console.c    console_init()
    |       |       | 初始化控制台
    |       |       | 结合lock锁机制，对print族函数加了层封装，如void console_put_str()
    |       |       |     { console_acquire(); put_str(str); console_release(); }
    |       |       | static struct lock console_lock;    // 声明控制台锁
    |       |       |
    |       |       |---- FILE: thread/sync.c   lock_init(&console_lock)
    |       |       |       | 初始化锁
    |       |       |       | 基于二元信号量实现的锁
    |       |       |       | struct semaphore{
    |       |       |       |     unsigned char value; struct list waiters; };
    |       |       |       |     信号量初值, 此信号量上阻塞的所有线程    
    |       |       |       | struct lock{
    |       |       |       |     struct task_struct *holder; struct semaphore sema; unsigned int holder_repeat_num; };
    |       |       |       |     锁的持有者, 信号量, 锁的持有者重复申请锁的次数    
    |       |       |       
    |       |       |---- FILE: thread/sync.c   lock_acquire()
    |       |       |       | 获取锁
    |       |       |       | sema_down(&lock->semaphore)信号量P操作
    |       |       |       |     while(sema->value == 0)   // value为0表明已经被别人持有
    |       |       |       |         list_append()当前线程把自己加入该锁的等待队列
    |       |       |       |         thread_block()当前线程阻塞自己，并触发调度，切换线程
    |       |       |       |         ************************************* 调度 *********
    |       |       |       | lock->holder = running_thread()
    |       |       |       |
    |       |       |       |---- FILE: thread/thread.c     thread_block()
    |       |       |               | 当前线程将自己阻塞
    |       |       |               | 修改线程状态为阻塞、触发调度, 切换线程执行
    |       |       |               |
    |       |       |               |---- FILE: thread/thread.c     schedule()
    |       |       |                       | 任务调度 (由当前阻塞线程主动触发)    
    |       |       |
    |       |       |---- FILE: thread/sync.c   lock_release()
    |       |               | 释放锁
    |       |               | lock->holder = NULL
    |       |               | sema_up(&lock->semaphore)信号量V操作
    |       |               |     list_pop(&sema->waiters)从等待队列中取出一个线程
    |       |               |     thread_unblock()唤醒该阻塞线程: 将阻塞线程加入就绪队列，并修改状态为READY
    |       |               |     sema->value++    
    |       |               |
    |       |               |---- FILE: thread/thread.c     thread_unblock()
    |       |                       | 唤醒阻塞线程
    |       |                       | 将阻塞线程加入就绪队列，并修改状态为READY
    |       |
    |       |---- FILE: device/keyboard.c   keyboard_init()
    |               | 初始化键盘
    |               | 初始化键盘的环形缓冲区 ioqueue_init(&keyboard_buf)
    |               | 注册安装中断处理程序 register_handler(0x21, intr_keyboard_handler)
    |               |     键盘的中断向量号0x21
    |               |
    |               |---- FILE: device/ioqueue.c    ioqueue_init()
    |               |       | 初始化环形缓冲区
    |               |       | 结合锁机制、生产者消费者模型
    |               |       | struct ioqueue{
    |               |       |     struct lock lock; // 锁
    |               |       |     struct task_struct *producer, *consumer; // 睡眠的生产者/消费者
    |               |       |     char buf[buffersize]; // 缓冲区
    |               |       |     signed int head, tail; }; // 队首写入, 队尾读出
    |               |
    |               |---- FILE: device/ioqueue.c    ioq_getchar()
    |               |       | 消费者消费一个字符
    |               |       | while(ioq_empty(ioq)) // 缓冲区为空时, 消费者睡眠
    |               |       |     lock_acquire(&ioq->lock);    // 获取锁, 每个锁对应的信号量都会有一个阻塞队列
    |               |       |     ioq_wait(&ioq->consumer);    // 消费者睡眠
    |               |       |     lock_release(&ioq->lock);    // 释放锁
    |               |       | char byte = ioq->buf[ioq->tail]; // 消费一个字符
    |               |       | if(ioq->producer !=NULL) wakeup(&ioq->producer); // 唤醒生产者(生产者睡眠是因为缓冲区满)    
    |               |       
    |               |---- FILE: device/ioqueue.c    ioq_putchar()
    |               |       | 生产者生产一个字符
    |               |       | while(ioq_full(ioq)) // 缓冲区满时, 生产者睡眠
    |               |       |     lock_acquire(&ioq->lock); // 获取锁, 每个锁对应的信号量都会有一个阻塞队列
    |               |       |     ioq_wait(&ioq->producer); // 生产者睡眠
    |               |       |     lock_release(&ioq->lock); // 释放锁
    |               |       | ioq->buf[ioq->head] = byte;   // 生产一个字符
    |               |       | if(ioq->consumer !=NULL) wakeup(&ioq->consumer); // 唤醒消费者(消费者睡眠是因为缓冲区空)
    |               |
    |               |---- FILE: device/keyboard.c   intr_keyboard_handler
    |                       | 键盘的中断处理程序
    |                       | 键盘上8048芯片 -> 主板上8042芯片 -> 中断代理8259A    
    |                       | 从输出缓冲区寄存器端口0x60读取扫描码
    |
    |---- FILE: thread/thread.c     thread_start()
    |       | 创建一个线程并执行
    |       | get_kernel_pages(1)内核空间中申请一页内存作为线程PCB(线程栈也在其中)
    |       |   PCB底部为线程/进程信息struct task_struct, PCB顶部为栈空间
    |       |
    |       | init_thread()在PCB底部 初始化线程基本信息: 状态 优先级 嘀嗒时间数 ...
    |       | thread_create()初始化线程栈: 中断使用的栈信息struct intr_stack, 线程栈信息struct thread_stack, 线程栈
    |       |   线程栈信息struct thread_stack: 栈顶为寄存器和eip(指向kernel_thread函数)和返回地址
    |       |                                  跳过返回地址栈顶+4为参数function、栈顶+8为参数func_arg
    |       | list_append()加入就绪队列、全部队列
    |       |
    |       |---- FILE: kernel/memory.c     get_kernel_pages();
    |               | 申请内核空间中的内存
    |               |   1) 虚拟内存池中申请连续的虚拟页 vaddr_get()
    |               |   2) 依次为每一个虚拟页在物理内存池中申请物理页 palloc()
    |               |   3) 在页表中完成虚拟地址和物理地址的映射 page_table_add()
    |
    |---- FILE: kernel/main.c       test_thread_1()
    |       | 线程1
    |       | (测试)作为键盘缓冲区的消费者
    |       | if(!ioq_empty(&keyboard_buf)) // 键盘缓冲区不为空时
    |       |     char byte = ioq_getchar(&keyboard_buf); // 从键盘缓冲区消费一个字符
    |       |     console_put_char(byte);   // 在控制台输出该字符
    |
    |---- while(1)

```



**更详细的 OS 相关技术分析位于各个章节的 README.md 中**



### 有效代码量展示 6323 行

![2020-12-05 11-08-30 的屏幕截图](./README.assets/2020-12-05 11-08-30 的屏幕截图.png)