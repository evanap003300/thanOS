ISO = thanOS.iso
KERNEL = kernel.elf

CXX = x86_64-elf-g++
LD = x86_64-elf-ld

CXXFLAGS = -ffreestanding -fno-exceptions -fno-rtti -O2 -Wall -Wextra -ILimine

SRCS = src/kernel.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean run

all: $(ISO)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(KERNEL): $(OBJS)
	$(LD) -T linker.ld -o $@ $(OBJS) -z max-page-size=0x1000

$(ISO): $(KERNEL) limine.cfg
	mkdir -p iso_root
	cp $(KERNEL) limine.cfg Limine/limine-bios.sys Limine/limine-bios-cd.bin Limine/limine-uefi-cd.bin iso_root/
	mkdir -p iso_root/EFI/BOOT
	cp Limine/BOOTX64.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -b limine-bios-cd.bin \
		--no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label \
		iso_root -o $(ISO)
	./Limine/limine bios-install $(ISO)
	rm -rf iso_root

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 512M

clean:
	rm -f $(OBJS) $(KERNEL) $(ISO)
