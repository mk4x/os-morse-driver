#
# Module Makefile for DM510 (2026)
#

# Change this if you keep your files elsewhere
ROOT = ..
KERNELDIR = ${ROOT}/linux
PWD = $(shell pwd)


obj-m += morse_dev.o

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) LDDINC=$(KERNELDIR)/include ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

