CXX := g++
CC  := gcc
LD  := g++

ifeq ($(debug),1)
	DEBUG := -DDEBUG
	OPTS := -O0 -g
else
	OPTS := -Ofast -fno-unroll-loops -Os
endif

LIBS     := -lm
WARN     := -Wall -Wextra
CXXFLAGS := $(WARN) $(OPTS) $(DEBUG) -std=c++23 -I/usr/include
CCFLAGS  := $(WARN) $(OPTS) $(DEBUG)            -I/usr/include
LDFLAGS  := $(LIBS)

SRC_DIR     := src
INCLUDE_DIR := include
BUILD_DIR   := build

TARGET := $(BUILD_DIR)/Chess2

CXXSRC := $(shell find $(SRC_DIR) -name '*.cpp')
CXXOBJ := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.cpp.o,$(CXXSRC))
CCSRC  := $(shell find $(SRC_DIR) -name '*.c')
CCOBJ  := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.c.o,$(CCSRC))
OBJ    := $(CXXOBJ) $(CCOBJ)
DIR    := $(sort $(dir $(OBJ)))

RED    := \033[91m
YELLOW := \033[93m
GREEN  := \033[92m
BLUE   := \033[94m
RESET  := \033[0m

all: $(TARGET)

$(TARGET): $(OBJ)
	@printf "$(BLUE)  LD     Linking $@\n$(RESET)"
	@$(LD) $(OBJ) $(LDFLAGS) -o $(TARGET)
ifeq ($(debug),1)
	@printf "$(YELLOW)  WARN   Warning: Compiling in DEBUG MODE\n"
endif

$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp | $(DIR)
	@printf "$(GREEN)  CXX    Building object $@\n$(RESET)"
	@$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c | $(DIR)
	@printf "$(GREEN)  CC     Building object $@\n$(RESET)"
	@$(CC) $(CCFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

$(DIR):
	@mkdir -p $(DIR)

clean:
	@printf "$(RED)  RM     Building directory $(BUILD_DIR)/\n$(RESET)"
	@rm -rf $(BUILD_DIR)

run:
	@printf "$(YELLOW)  RUN    Running executable $(TARGET)\n$(RESET)"
ifeq ($(debug),1)
	@gdb $(TARGET)
else
	@./$(TARGET)
endif
	@printf "$(YELLOW)  RUN    Done running executable $(TARGET)\n$(RESET)"

size:
	@wc -c < $(TARGET) | awk '{printf "%.2f KB\n", $$1 / 1000}'

requirements:
