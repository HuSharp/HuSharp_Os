;-------------	 loader和kernel   ----------
; 即为 宏名  equ  值
LOADER_BASE_ADDR equ 0x900 
LOADER_START_SECTOR equ 0x2
KERNEL_BIN_BASE_ADDR equ 0x70000
KERNEL_IMAGE_BASE_ADDR  equ   0x1500
KERNEL_START_SECTOR equ 0x9

PAGE_DIR_TABLE_POS equ 0x100000

;--------------   gdt描述符属性  -------------
; 段描述符的23位 表示粒度为 4K
DESC_G_4K   equ	  1_00000000000000000000000b    ;  23
DESC_D_32   equ	   1_0000000000000000000000b    ;   22 位 1表示在32位下
DESC_L	    equ	    0_000000000000000000000b	;  64位代码标记，此处标记为0便可。
DESC_AVL    equ	     0_00000000000000000000b	;  cpu不用此位，暂置为0  
; 第二部分段界限值，由于采用了32位平坦模型，所以段界限为(4GB / 4KB) - 1 = 0xFFFFF，故为全1
DESC_LIMIT_CODE2  equ 1111_0000000000000000b    ; 代码段的 16-19的limit 总的段限界为fffff
DESC_LIMIT_DATA2  equ DESC_LIMIT_CODE2          ; 数据段的 
DESC_LIMIT_VIDEO2  equ 0000_000000000000000b    ; 限界为 bffff 
DESC_P	    equ		      1_000000000000000b
DESC_DPL_0  equ		       00_0000000000000b
DESC_DPL_1  equ		       01_0000000000000b
DESC_DPL_2  equ		       10_0000000000000b
DESC_DPL_3  equ		       11_0000000000000b
DESC_S_CODE equ		         1_000000000000b
DESC_S_DATA equ	  DESC_S_CODE
DESC_S_sys  equ		         0_000000000000b
DESC_TYPE_CODE  equ	          1000_00000000b	;x=1,c=0,r=0,a=0 代码段是可执行的,非依从的,不可读的,已访问位a清0.  
DESC_TYPE_DATA  equ	          0010_00000000b	;x=0,e=0,w=1,a=0 数据段是不可执行的,向上扩展的,可写的,已访问位a清0.


; 代码段描述符的高32位表示，其中(0x00 << 24表示最高8位的段基址值
; 由于我们采用的是平坦模型，故基址为零)，后面唯一可变的就是段界限值
DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
            DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + \
            DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
            DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + \
            DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
; 由于显存区为 0b8000, 因此段基址高四位为 b 
DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
            DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + \
            DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0b


; 选择子属性
; 请求特权级
RPL0 equ 00b
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b
; 表示是 GDT 还是 LDT
TI_GDT equ 000b
TI_LDT equ 100b

;----------------   页表相关属性    --------------
PG_P  equ   1b      ; 是否在磁盘上
PG_RW_R	 equ  00b   ; 只读
PG_RW_W	 equ  10b   ; 只写
PG_US_S	 equ  000b  ; 第三位为 0 表示为 特权级 3
PG_US_U	 equ  100b 