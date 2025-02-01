obj-m += hello.o

# Read KDIR from environment variable
KDIR ?= $(KERNEL_SRC)

# Check if KDIR is set
ifeq ($(KDIR),)
  $(error KERNEL_SRC environment variable is not set. Please set it to the path of your prepared kernel source directory.)
endif

all:
	make -C $(KDIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) modules

clean:
	make -C $(KDIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) clean