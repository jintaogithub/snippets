# Compiler and flags for x86 (native compilation)
CXX_x86 ?= g++
CFLAGS_x86 ?= -g -Wall -O2

# Compiler and flags for ARM64 cross-compilation
CXX_arm64 ?= aarch64-linux-gnu-g++
CFLAGS_arm64 ?= -g -Wall -O2

# Target architecture (default to x86 if not specified)
TARGET_ARCH ?= x86

# Macro to select compiler and flags based on target architecture
ifeq ($(TARGET_ARCH),x86)
  CXX = $(CXX_x86)
  CFLAGS = $(CFLAGS_x86)
else ifeq ($(TARGET_ARCH),arm64)
  CXX = $(CXX_arm64)
  CFLAGS = $(CFLAGS_arm64)
else
  $(error Invalid TARGET_ARCH. Supported values are x86 and arm64)
endif

# Default target
all:
	$(CXX) sync_sum.cc -o sync_sum

clean:
	rm -rf sync_sum

.PHONY: all
