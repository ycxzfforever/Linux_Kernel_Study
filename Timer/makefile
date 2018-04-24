ifeq ($(KERNELRELEASE),)
KERNELDIR = /opt/linux-3.2.0-psp04.06.00.11
INSTALL_PATH = /mymodules
ARCH = arm 
CROSS_COMPILE = /opt/am335x_gcc/bin/arm-linux-gnueabihf-
PWD = $(shell pwd)
OBJ = timer
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
install:
#$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_PATH=$(INSTALL_PATH)
	cp -f $(OBJ).ko $(INSTALL_PATH)
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module* modules*    
.PHONY: modules modules_install clean
else
	obj-m := timer.o
endif
