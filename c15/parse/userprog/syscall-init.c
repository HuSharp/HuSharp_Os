#include "syscall-init.h"
#include "syscall.h"
#include "stdint.h"
#include "print.h"
#include "thread.h"
#include "console.h"
#include "string.h"
#include "fs.h"
#include "fork.h"
#include "memory.h"

// 定义 调用子功能数组
#define syscall_nr 32
typedef void* syscall;
syscall syscall_table[syscall_nr];

/*********   内核空间中的函数 + sys   ***************/
// 返回当前任务的 pid 
uint32_t sys_getpid(void) {
   return running_thread()->pid;
}

// 现在的 sys_write 中 fs.h 中实现
// // 打印字符串（未实现文件系统前版本）
// // 返回字符串长度
// uint32_t sys_write(char* str) {
//    console_put_str(str);
//    return strlen(str);
// }

/********************************************/

// 初始化系统调用
void syscall_init(void) {
   put_str("syscall_init start!\n");
   syscall_table[SYS_GETPID] = sys_getpid;
   syscall_table[SYS_WRITE] = sys_write;
   syscall_table[SYS_MALLOC] = sys_malloc;
   syscall_table[SYS_FREE] = sys_free;
   syscall_table[SYS_FORK] = sys_fork;
   syscall_table[SYS_READ] = sys_read;
   syscall_table[SYS_PUTCHAR] = sys_putchar;
   syscall_table[SYS_CLEAR]   = cls_screen;// 位于 print.S
   put_str("syscall_init done!\n");
}