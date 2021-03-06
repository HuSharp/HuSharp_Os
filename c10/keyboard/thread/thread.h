#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H
#include "stdint.h"
#include "list.h"

// 自定义通用函数类型,它将在很多线程函数中做为形参类型
typedef void thread_func(void*);

// 进程或线程的状态
enum task_status {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};

/***********   中断栈intr_stack   ***********
 * 此结构用于中断发生时保护程序(线程或进程)的上下文环境:
 * 进程或线程被外部中断或软中断打断时,会按照此结构压入上下文
 * 寄存器,  intr_exit中的出栈操作是此结构的逆操作
 * 此栈在线程自己的内核栈中位置固定,所在页的最顶端
********************************************/
struct intr_stack {
    uint32_t vec_no;	 // kernel.S 宏VECTOR中push %1压入的中断号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    // 虽然pushad把esp也压入,但esp是不断变化的,所以会被popad忽略
    uint32_t esp_dummy;	 
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

/* 以下由cpu从低特权级进入高特权级时压入 */
    uint32_t err_code;		 // err_code会被压入在eip之后
    void (*eip) (void);
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};

/***********  线程栈thread_stack  ***********
 * 线程自己的栈,用于存储线程中待执行的函数
 * 此结构在线程自己的内核栈中位置不固定,
 * 用在switch_to时保存线程环境。
 * 实际位置取决于实际运行情况。
 ******************************************/
struct thread_stack {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi; 
    
    /* 线程第一次执行时,eip指向待调用的函数kernel_thread 
    其它时候,eip是指向switch_to的返回地址*/
    void (*eip) (thread_func* func, void* func_arg); 

    /*****   以下仅供第一次被调度上cpu时使用   ****/
    // 占位函数，迷惑 ret ,假装为返回地址，实则为获取参数提供栈顶位置
    // 用于找到函数参数位置
    void (*unused_retaddr); 
    thread_func* function;  // 由 Kernel_thread 所调用的函数名
    void* func_arg; // 由 Kernel_thread 所调用函数所需要的的参数
};

// 进程或线程的 pcb， 程序控制块
// ticks 表示的是中断次数
struct task_struct {
    uint32_t* self_kstack;  // 各内核线程都用自己的内核栈
    char name[16];          // 记录任务的名字
    enum task_status status;    // 线程状态
    uint8_t priority;       // 线程优先级
    uint8_t ticks;          // 每次在处理器上执行的时间滴答数
    // 已经执行的滴答数
    uint32_t elapsed_ticks;// elapsed : 流逝
    // 加入到就绪队列中
    struct list_elem general_tag;
    // 值得注意的是 由于需要管控所有的线程资源，
    // 但是一个 list_elem 只有两个指针，因此只能位于一个队列中
    // 所以创建新的队列 all_list 记录全部线程
    struct list_elem all_list_tag;

    uint32_t* pgdir;    // 进程拥有独立地址空间（页表），线程为NULL

    // 通过判断栈的边界溢出魔数值是否发生变化
    uint32_t stack_magic;   // 栈的边界标记，用于检测栈的溢出


};

void thread_create(struct task_struct* pthread, thread_func function, void* func_arg);
void init_thread(struct task_struct* pthread, char* name, int prio);
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg);
struct task_struct* running_thread(void);
void schedule(void);
void thread_environment_init(void);
void thread_block(enum task_status stat);
void thread_unblock(struct task_struct* pthread);

#endif