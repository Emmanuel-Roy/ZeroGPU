# -----------------------
# ZeroGPU Makefile (Cross-platform: Windows / Linux)
# -----------------------

CXX = g++
CXXFLAGS = -O2 -std=c++17
TARGET = ZeroGPU.exe
SRCS = ZeroGPU.cpp

# Base directory = folder where this makefile lives
BASE_DIR := $(CURDIR)
INCLUDE_DIR := $(BASE_DIR)/src/include
LIB_DIR := $(BASE_DIR)/src/lib

# Detect OS
ifeq ($(OS),Windows_NT)
    # Windows (MinGW)
    CXXFLAGS += -I"$(INCLUDE_DIR)"
    LDFLAGS = -L"$(LIB_DIR)" -lmingw32 -lSDL2main -lSDL2
    RM = del
else
    # Linux / macOS
    CXXFLAGS += $(shell sdl2-config --cflags)
    LDFLAGS = $(shell sdl2-config --libs)
    TARGET = ZeroGPU
    RM = rm -f
endif

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

info:
	@echo "Compiler flags: $(CXXFLAGS)"
	@echo "Linker flags:   $(LDFLAGS)"
	@echo "Target:         $(TARGET)"

clean:
	$(RM) $(TARGET)
