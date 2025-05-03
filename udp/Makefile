APP = udp

SRC_DIR := src
SRCS-y := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*.cpp)

INCLUDES := -Iinclude

CXX ?= g++
PKGCONF ?= pkg-config

PC_FILE := $(shell $(PKGCONF) --path libdpdk 2>/dev/null)

CXXFLAGS += -O3 -std=c++17 $(shell $(PKGCONF) --cflags libdpdk) $(INCLUDES) -DALLOW_EXPERIMENTAL_API
LDFLAGS += -lpthread

LDFLAGS_SHARED = $(shell $(PKGCONF) --libs libdpdk)
LDFLAGS_STATIC = $(shell $(PKGCONF) --static --libs libdpdk)

build/$(APP)-shared: $(SRCS-y) Makefile $(PC_FILE) | build
	$(CXX) $(CXXFLAGS) $(SRCS-y) -o $@ $(LDFLAGS) $(LDFLAGS_SHARED)

build/$(APP)-static: $(SRCS-y) Makefile $(PC_FILE) | build
	$(CXX) $(CXXFLAGS) $(SRCS-y) -o $@ $(LDFLAGS) $(LDFLAGS_STATIC)
