SRC_DIR := src
BUILD_DIR := build

KERNEL := $(BUILD_DIR)/kernel.elf 
ISO := $(BUILD_DIR)/thanOS.iso

CXX := x86_64-elf-g++
LD := x86_64-elf-ld

AS := nasm

CXXFLAGS := -ffreestanding -fno-exceptions -fno-rtti -O2 -Wall -Wextra -Ilimine -mcmodel=kernel -mno-red-zone -mgeneral-regs-only

CPP_SRCS := $(wildcard src/*.cpp)
ASM_SRCS := $(wildcard src/*.asm)

CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CPP_SRCS))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.o, $(ASM_SRCS))

OBJS := $(CPP_OBJS) $(ASM_OBJS)

.PHONY: all clean run 

all: $(ISO) 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AS) -f elf64 $< -o $@

$(KERNEL): $(OBJS) 
	$(LD) -T linker.ld -o $@ $(OBJS) -z max-page-size=0x1000

$(ISO): $(KERNEL) limine.conf
	mkdir -p iso_root/limine
	mkdir -p iso_root/EFI/BOOT

	cp $(KERNEL) iso_root/

	cp limine.conf iso_root/
	cp limine.conf iso_root/limine/ 

	cp Limine/limine-bios.sys Limine/limine-bios-cd.bin Limine/limine-uefi-cd.bin iso_root/limine/
	cp Limine/BOOTX64.EFI iso_root/EFI/BOOT/

	xorriso -as mkisofs -b limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label \
		iso_root -o $(ISO)

	./Limine/limine bios-install $(ISO)
	rm -rf iso_root

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 512M -display sdl

clean:
	rm -rf $(BUILD_DIR) 

