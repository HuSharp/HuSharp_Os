[bits 32]
; 对于CPU会自动压入错误码的中断类型，无需额外的操作
%define ERROR_CODE nop
; 如果CPU没有压入错误码，为了保持处理逻辑的一致性，我们需要手动压入一个0
%define ZERO push 0

# 调用 interrupt 中的函数来中断处理
# extern put_str  ; 声明外部函数
extern idt_table    ; idt_table 是 C 中注册的中断处理程序数组    

section .data
global intr_entry_table
intr_entry_table:

; 中断处理程序宏定义
; 接受两个参数
%macro VECTOR 2
section .text
intr%1entry:        ; %1 表示中断向量号，用于确定中断位置
    %2      ; 需要首先进行压入操作 nop / push 0

    ; 保护上下文
    push ds
    push es
    push fs
    push gs
; PUSHAD指令压入32位寄存器,其入栈顺序是: EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI
    pushad

    ; 如果是从片上进入的中断,除了往从片上发送EOI外,还要往主片上发送EOI 
    mov al, 0x20        ; 中断结束命令 EOI
    out 0xa0, al        ; 向从片发送
    out 0x20, al        ; 向主片发送

; 不管idt_table中的目标程序是否需要参数,都一律压入中断向量号,调试时很方便
    push %1     ; 向量号
    call [idt_table + %1*4]     ; 调用 idt_table 中的中断处理函数
    jmp intr_exit

section .data
    dd intr%1entry

%endmacro

section .text
global intr_exit
intr_exit:	     
; 以下是恢复上下文环境
    add esp, 4			   ; 跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4			   ; 跳过error_code
    iretd

VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO 
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO 
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE 
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO 
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO 
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO 
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE 
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO 
VECTOR 0x20,ZERO	;时钟中断对应的入口
VECTOR 0x21,ZERO	;键盘中断对应的入口
VECTOR 0x22,ZERO	;级联用的
VECTOR 0x23,ZERO	;串口2对应的入口
VECTOR 0x24,ZERO	;串口1对应的入口
VECTOR 0x25,ZERO	;并口2对应的入口
VECTOR 0x26,ZERO	;软盘对应的入口
VECTOR 0x27,ZERO	;并口1对应的入口
VECTOR 0x28,ZERO	;实时时钟对应的入口
VECTOR 0x29,ZERO	;重定向
VECTOR 0x2a,ZERO	;保留
VECTOR 0x2b,ZERO	;保留
VECTOR 0x2c,ZERO	;ps/2鼠标
VECTOR 0x2d,ZERO	;fpu浮点单元异常
VECTOR 0x2e,ZERO	;硬盘
VECTOR 0x2f,ZERO	;保留

;;;;;;;;;;;;;;;;   0x80号中断   ;;;;;;;;;;;;;;;;
[bits 32]
extern syscall_table

section .text
global syscall_handler
syscall_handler:
; 保存上下文环境
; 为保证格式，需要压入错误码 和 中断号 
    push 0			    ; 压入0, 使栈中格式统一

    push ds
    push es
    push fs
    push gs
    pushad      ; PUSHAD指令压入32位寄存器，其入栈顺序是:
                ; EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI
    push 0x80   ; 此位置压入 0x80 也是为了保持统一栈格式

;2 从内核栈中获取cpu自动压入的用户栈指针esp的值
    mov ebx, [esp + 4 + 48 + 4 + 12]; 12表示 eip cs eflags esp3
    ; ebx 此时为 栈底指针

; 3 将参数压入内核栈
    push dword [ebx + 12]       ; 第 3 个参数
    push dword [ebx + 8]
    push dword [ebx + 4]
    mov edx, [ebx]      ; 功能号

; 调用子功能处理函数
    ; 编译器会在栈中根据C函数声明匹配正确数量的参数
    call [syscall_table + edx * 4]  
    add esp, 12     ; 跨过三个参数

; 将call调用后的返回值存入待当前内核栈中eax的位置
    mov [esp + (7 + 1)*4], eax  ; 1 表示 push 0x80
    jmp intr_exit       ; intr_exit 返回恢复上下文