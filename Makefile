#### CONFIG ####
# set to hvm on mac OS, tcg if you are on windows/don't have KVM, set to xen if you have xen hypervisor
ACCEL := kvm

#### PROGRAMS ####
CC := x86_64-elf-gcc
CXX := x86_64-elf-g++
AS := nasm
LD := x86_64-elf-ld
PARTED := parted

#### FLAGS ####
K_CFLAGS := fail-cannot-use-c-code-in-kernel
K_CXXFLAGS := -c -I. -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -ggdb
K_ASFLAGS := -felf64 -gdwarf
K_LDFLAGS := -Tlinker.ld

U_CFLAGS := fail-todo
U_CXXFLAGS := fail-todo
U_ASFLAGS := -felf64 -gdwarf
U_LDFLAGS := fail-todo

#### VARS ####
OBJS_KCPP := $(patsubst Kernel/%.cpp,Build/Kernel/%.o,$(wildcard Kernel/*.cpp))
OBJS_KASM := $(patsubst Kernel/%.s,Build/Kernel/%.o,$(wildcard Kernel/*.s))
OBJS_KRNL := $(OBJS_KASM) $(OBJS_KCPP)

#### RULES ####
.PHONY: build build-pxe run run-pxe
build: Build/Kernel.hdd
build-pxe: Build/PXEEnv/ready
run: Build/Kernel.hdd
	qemu-system-x86_64 -hda Build/Kernel.hdd -accel $(ACCEL)
run-pxe: Build/PXEEnv/ready
	qemu-system-x86_64 -cpu host -accel $(ACCEL) \
		-netdev user,id=net0,net=192.168.88.0/24,tftp=Build/PXEEnv,bootfile=/limine-pxe.bin \
		-device virtio-net-pci,netdev=net0 \
		-object rng-random,id=virtio-rng0,filename=/dev/urandom \
		-device virtio-rng-pci,rng=virtio-rng0,id=rng0,bus=pci.0,addr=0x9 \
		-boot n

Build/Kernel.hdd: Build/DriveEnv/ready
	rm Build/Kernel.hdd
	fallocate Build/Kernel.hdd -l 24M
	parted -s Build/Kernel.hdd mklabel gpt
	parted -s Build/Kernel.hdd mkpart primary 2048s 100%
	echfs-utils -g -p0 Build/Kernel.hdd quick-format 512
	sh import.sh
	limine-install Build/Kernel.hdd


Build/DriveEnv/ready: Build/Env/ready
	mkdir -vp Build/DriveEnv
	cp -r Build/Env/* Build/DriveEnv
	touch Build/DriveEnv/ready
Build/PXEEnv/ready: Build/Env/ready
	mkdir -vp Build/PXEEnv
	cp -r Build/Env/* Build/PXEEnv
	touch Build/PXEEnv/ready

Build/Env/ready: Build/Kernel/Final.elf Config/limine.cfg
	mkdir -vp Build/Env/Boot
	cp Config/limine.cfg Build/Env
	cp Build/Kernel/Final.elf Build/Env/Boot/Kernel.elf
	touch Build/Env/ready

Build/Kernel/Final.elf: $(OBJS_KRNL) linker.ld
	mkdir -vp Build/Kernel
	$(LD) $(K_LDFLAGS) $(OBJS_KRNL) -o $@

Build/Kernel/%.o: Kernel/%.s
	mkdir -vp $(shell dirname $@)
	$(AS) $(K_ASFLAGS) $< -o $@

Build/Kernel/%.o: Kernel/%.cpp
	mkdir -vp $(shell dirname $@)
	$(CXX) $(K_CXXFLAGS) $< -o $@

