SRC_DIR := src
BUILD_DIR := build

KERNEL := $(BUILD_DIR)/kernel.elf 
ISO := $(BUILD_DIR)/thanOS.iso

CXX := x86_64-elf-g++
LD := x86_64-elf-ld

AS := nasm

PARTED := parted
MKFS := mkfs.fat
MCOPY := mcopy

CXXFLAGS := -ffreestanding -fno-exceptions -fno-rtti -O2 -Wall -Wextra -Ilimine -mcmodel=kernel -mno-red-zone -mgeneral-regs-only -I$(SRC_DIR)

CPP_SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
ASM_SRCS := $(shell find $(SRC_DIR) -name '*.asm')

CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CPP_SRCS))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.o, $(ASM_SRCS))

OBJS := $(CPP_OBJS) $(ASM_OBJS)

.PHONY: all clean run 

all: $(ISO) disk.img

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AS) -f elf64 $< -o $@

$(KERNEL): $(OBJS)
	$(LD) -T linker.ld -o $@ $(OBJS) -z max-page-size=0x1000

USER_CXXFLAGS := -ffreestanding -fno-exceptions -fno-rtti -O2 -Wall -Wextra -mno-red-zone -mgeneral-regs-only -mcmodel=large -fno-pie -nostdlib

$(BUILD_DIR)/test.elf: user/test.cpp user/syscalls.h user/user.ld
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(USER_CXXFLAGS) -no-pie -static -z max-page-size=0x1000 -T user/user.ld -o $@ user/test.cpp

$(BUILD_DIR)/proc_a.elf: user/proc_a.cpp user/syscalls.h user/user.ld
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(USER_CXXFLAGS) -no-pie -static -z max-page-size=0x1000 -T user/user.ld -o $@ user/proc_a.cpp

$(BUILD_DIR)/proc_b.elf: user/proc_b.cpp user/syscalls.h user/user.ld
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(USER_CXXFLAGS) -no-pie -static -z max-page-size=0x1000 -T user/user.ld -o $@ user/proc_b.cpp

$(BUILD_DIR)/echo.elf: user/echo.cpp user/syscalls.h user/user.ld
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(USER_CXXFLAGS) -no-pie -static -z max-page-size=0x1000 -T user/user.ld -o $@ user/echo.cpp

$(BUILD_DIR)/launcher.elf: user/launcher.cpp user/syscalls.h user/user.ld
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(USER_CXXFLAGS) -no-pie -static -z max-page-size=0x1000 -T user/user.ld -o $@ user/launcher.cpp

$(BUILD_DIR)/shell.elf: user/shell.cpp user/syscalls.h user/user.ld
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(USER_CXXFLAGS) -no-pie -static -z max-page-size=0x1000 -T user/user.ld -o $@ user/shell.cpp

disk.img:
	qemu-img create -f raw disk.img 64M
	$(PARTED) -s $@ mklabel msdos
	$(PARTED) -s $@ mkpart primary fat32 1MiB 100%
	$(MKFS) -F 32 -n "THANOS_HDD" --offset 2048 $@
	echo "This is a text file from the Makefile" > test.txt
	$(MCOPY) -i $@@@1M test.txt ::TEST.TXT

$(ISO): $(KERNEL) limine.conf $(BUILD_DIR)/test.elf $(BUILD_DIR)/proc_a.elf $(BUILD_DIR)/proc_b.elf $(BUILD_DIR)/echo.elf $(BUILD_DIR)/launcher.elf $(BUILD_DIR)/shell.elf
	mkdir -p iso_root/limine
	mkdir -p iso_root/EFI/BOOT

	cp $(KERNEL) iso_root/
	cp limine.conf iso_root/
	cp limine.conf iso_root/limine/ 

	cp Limine/limine-bios.sys Limine/limine-bios-cd.bin Limine/limine-uefi-cd.bin iso_root/limine/
	cp Limine/BOOTX64.EFI iso_root/EFI/BOOT/

	rm -rf $(BUILD_DIR)/initrd_root
	mkdir -p $(BUILD_DIR)/initrd_root
	cp -r src/initrd/. $(BUILD_DIR)/initrd_root/
	cp $(BUILD_DIR)/test.elf $(BUILD_DIR)/proc_a.elf $(BUILD_DIR)/proc_b.elf $(BUILD_DIR)/echo.elf $(BUILD_DIR)/launcher.elf $(BUILD_DIR)/shell.elf $(BUILD_DIR)/initrd_root/
	tar -cf $(BUILD_DIR)/initrd.tar -C $(BUILD_DIR)/initrd_root .
	cp $(BUILD_DIR)/initrd.tar iso_root/

	xorriso -as mkisofs -b limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label \
		iso_root -o $(ISO)

	./Limine/limine bios-install $(ISO)
	rm -rf iso_root

run: $(ISO) disk.img
	qemu-system-x86_64 \
		-drive id=disk,file=disk.img,if=none,format=raw \
		-device ide-hd,drive=disk,bus=ide.0,unit=0 \
		-drive id=cd,file=$(ISO),if=none,format=raw\
		-device ide-cd,drive=cd,bus=ide.1,unit=0 \
		-boot d \
		-m 512M \
		-display sdl

clean:
	rm -rf $(BUILD_DIR)
	rm disk.img	

