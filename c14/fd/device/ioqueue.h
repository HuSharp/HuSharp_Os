#ifndef __DEVICE_IOQUEUE_H
#define __DEVICE_IOQUEUE_H
#include "stdint.h"
#include "thread.h"
#include "sync.h"

#define bufsize 64  // ���д�СΪ 64����������ͷβָ��� 1 ���������Ϊ 63

// ���ζ���
struct ioqueue {
    // ������-������ ���⣬��Ҫʵ��ͬ������
    struct lock lock;
    // ������
    struct task_struct* producer;
    // ������
    struct task_struct* consumer;
    // ������
    char buf[bufsize];
    int32_t head;//ͷָ��
    int32_t tail;//βָ��
};

void ioqueue_init(struct ioqueue* ioq);
bool ioq_full(struct ioqueue* ioq);
bool ioq_empty(struct ioqueue* ioq);
char ioq_getchar(struct ioqueue* ioq);
void ioq_putchar(struct ioqueue* ioq, char byte);

#endif