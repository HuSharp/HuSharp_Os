BUILD_DIR = ./build
# 存储编译的所有文件
ENTRY_POINT = 0xc0001500
# ld 链接 -Ttext参数
# 编译器及编译参数
AS = nasm
CC = gcc
LD = ld
LIB = -I lib/ -I lib/kernel/ -I lib/user/ -I kernel/ -I device/
ASFLAGS = -f elf
ASIB = -I include/
CFLAGS = -Wall -m32 -fno-stack-protector $(LIB) -c -fno-builtin \
		-W -Wstrict-prototypes -Wmissing-prototypes
LDFLAGS =  -m elf_i386 -Ttext $(ENTRY_POINT) -e main -Map $(BUILD_DIR)/kernel.map
OBJS = $(BUILD_DIR)/main.o  $(BUILD_DIR)/init.o  $(BUILD_DIR)/interrupt.o \
		$(BUILD_DIR)/timer.o  $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o \
		$(BUILD_DIR)/debug.o


##############     c代码编译     ###############
$(BUILD_DIR)/main.o: kernel/main.c lib/kernel/print.h \
				lib/stdint.h kernel/init.h
		$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: kernel/init.c kernel/init.h lib/kernel/print.h \
				lib/stdint.h kernel/interrupt.h device/timer.h
		$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c kernel/interrupt.h \
				lib/stdint.h kernel/global.h lib/kernel/io.h lib/kernel/print.h
		$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/timer.c device/timer.h lib/stdint.h\
				lib/kernel/io.h lib/kernel/print.h
		$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: kernel/debug.c kernel/debug.h \
				lib/kernel/print.h lib/stdint.h kernel/interrupt.h
		$(CC) $(CFLAGS) $< -o $@

# 编译loader和mbr
$(BUILD_DIR)/mbr.bin: boot/mbr.S
	$(AS) $(ASIB) $< -o $@

$(BUILD_DIR)/loader.bin: boot/loader.S
	$(AS) $(ASIB) $< -o $@

##############    汇编代码编译    ###############
$(BUILD_DIR)/kernel.o: kernel/kernel.S
		$(AS) $(ASFLAGS) $< -o $@
$(BUILD_DIR)/print.o: lib/kernel/print.S
		$(AS) $(ASFLAGS) $< -o $@

##############    链接所有目标文件    #############
$(BUILD_DIR)/kernel.bin: $(OBJS)
		$(LD) $(LDFLAGS) $^ -o $@

.PHONY : mk_dir hd clean tran all

mk_dir:
	#if [[ ! -d $(BUILD_DIR) ]];then mkdir $(BUILD_DIR);fi
	mkdir $(BUILD_DIR);
hd:
	# echo ".....Writing mbr, loader and kernel to disk....."
	dd if=$(BUILD_DIR)/mbr.bin of=/home/husharp/bochs_hu/bochs/hd60M.img bs=512 count=1 conv=notrunc
	dd if=$(BUILD_DIR)/loader.bin of=/home/husharp/bochs_hu/bochs/hd60M.img bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=/home/husharp/bochs_hu/bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc


clean:
	cd $(BUILD_DIR) && rm -f ./*

build: $(BUILD_DIR)/mbr.bin $(BUILD_DIR)/loader.bin $(BUILD_DIR)/kernel.bin
		# echo ".....build kernel now....."

all: mk_dir build hd tran

tran:
	cd /home/husharp/bochs_hu/bochs/	&& bin/bochs -f bochsrc.disk