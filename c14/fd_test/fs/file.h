#ifndef __FS_FILE_H
#define __FS_FILE_H
#include "stdint.h"
#include "ide.h"
#include "dir.h"
#include "global.h"

// 文件结构
struct file {
    // 记录当前文件操作的偏移地址， 以 0 为起始
    uint32_t fd_pos;
    uint32_t fd_flag;// 文件操作标识
    struct inode* fd_inode;// 标识文件的 inode
};

// 标准输入输出描述符
enum std_fd {
    stdin_no,   // 0 标准输入
    stdout_no,  // 1 标准输出
    stderr_no   // 2 标准错误
};

// 位图类型
enum bitmap_type {
    INODE_BITMAP,   // inode 位图
    BLOCK_BITMAP    // 块位图
};

#define MAX_FILE_OPEN 32    // 系统可打开的最大文件数

extern struct file file_table[MAX_FILE_OPEN];
int32_t inode_bitmap_alloc(struct partition* part);
int32_t block_bitmap_alloc(struct partition* part);
int32_t file_create(struct dir* parent_dir, char* filename, uint8_t flag);
void bitmap_sync(struct partition* part, uint32_t bit_idx, uint8_t btmp_type);
int32_t get_free_slot_in_global(void);
int32_t pcb_fd_install(int32_t globa_fd_idx);

#endif