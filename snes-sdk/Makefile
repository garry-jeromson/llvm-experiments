# SNES SDK Makefile
# Supports both host-side unit tests and W65816 target builds

# Directories
SDK_ROOT := $(dir $(lastword $(MAKEFILE_LIST)))
INCLUDE_DIR := $(SDK_ROOT)include
SRC_DIR := $(SDK_ROOT)src
TEST_DIR := $(SDK_ROOT)test
BUILD_DIR := $(SDK_ROOT)build
VENV_DIR := $(SDK_ROOT).venv

# Python virtual environment
PYTHON := python3
VENV_PYTHON := $(VENV_DIR)/bin/python
VENV_PIP := $(VENV_DIR)/bin/pip

# Host compiler (for unit tests)
HOST_CXX := clang++
HOST_CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -I$(INCLUDE_DIR) -DSNES_TESTING

# W65816 compiler (from LLVM build)
LLVM_BUILD ?= ../build
W65816_CC := $(LLVM_BUILD)/bin/clang
W65816_CXXFLAGS := -target w65816-unknown-none -O2 -fno-exceptions -fno-rtti -I$(INCLUDE_DIR)

# Assembler and linker (ca65/ld65 from cc65)
AS := ca65
ASFLAGS := --cpu 65816
LD := ld65
LDFLAGS := -C $(SDK_ROOT)linker_configs/lorom.cfg

# Source files
SDK_SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
TEST_SOURCES := $(wildcard $(TEST_DIR)/unit/test_*.cpp)

# Object files
SDK_HOST_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/host/%.o,$(SDK_SOURCES))
SDK_W65816_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/w65816/%.o,$(SDK_SOURCES))

.PHONY: all clean test-unit test-sdk-integration test-sdk host-build w65816-build venv

# Default target
all: host-build

# Create build directories
$(BUILD_DIR)/host $(BUILD_DIR)/w65816:
	mkdir -p $@

# ============================================================================
# Python Virtual Environment
# ============================================================================

$(VENV_DIR):
	$(PYTHON) -m venv $(VENV_DIR)
	$(VENV_PIP) install --upgrade pip
	$(VENV_PIP) install -r $(SDK_ROOT)requirements.txt

venv: $(VENV_DIR)
	@echo "Virtual environment ready at $(VENV_DIR)"
	@echo "Activate with: source $(VENV_DIR)/bin/activate"

# ============================================================================
# Host Build (for unit tests)
# ============================================================================

host-build: $(BUILD_DIR)/host $(SDK_HOST_OBJS)

$(BUILD_DIR)/host/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)/host
	$(HOST_CXX) $(HOST_CXXFLAGS) -c $< -o $@

# Unit tests
$(BUILD_DIR)/test_unit: $(TEST_DIR)/unit/run_unit_tests.cpp $(SDK_HOST_OBJS) | $(BUILD_DIR)/host
	$(HOST_CXX) $(HOST_CXXFLAGS) -I$(TEST_DIR)/unit $< $(SDK_HOST_OBJS) -o $@

test-unit: $(BUILD_DIR)/test_unit
	@echo "Running unit tests..."
	@$(BUILD_DIR)/test_unit

# ============================================================================
# W65816 Build (for actual SNES)
# ============================================================================

w65816-build: $(BUILD_DIR)/w65816 $(SDK_W65816_OBJS)

$(BUILD_DIR)/w65816/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)/w65816
	$(W65816_CC) $(W65816_CXXFLAGS) -c $< -o $@

# ============================================================================
# Integration Tests (run in emulator)
# ============================================================================

test-sdk-integration:
	@echo "Running SDK integration tests..."
	@echo "TODO: Implement integration test runner"

# ============================================================================
# Combined test target
# ============================================================================

test-sdk: test-unit test-sdk-integration

# ============================================================================
# Examples
# ============================================================================

examples: w65816-build
	@echo "Building examples..."
	@echo "TODO: Build example programs"

# ============================================================================
# Clean
# ============================================================================

clean:
	rm -rf $(BUILD_DIR)

# ============================================================================
# Help
# ============================================================================

help:
	@echo "SNES SDK Build Targets:"
	@echo ""
	@echo "  make venv                - Create Python virtual environment"
	@echo "  make test-unit           - Run host-side unit tests"
	@echo "  make test-sdk-integration - Run emulator integration tests"
	@echo "  make test-sdk            - Run all SDK tests"
	@echo "  make host-build          - Build SDK for host (testing)"
	@echo "  make w65816-build        - Build SDK for W65816 target"
	@echo "  make examples            - Build example programs"
	@echo "  make clean               - Remove build artifacts"
	@echo ""
	@echo "Variables:"
	@echo "  LLVM_BUILD=path          - Path to LLVM build directory (default: ../build)"
	@echo ""
	@echo "ROM Builder (snes-sdk/rom_builder/):"
	@echo "  python rom_builder/build_rom.py main.cpp output.sfc"
	@echo "  python rom_builder/fix_checksum.py rom.sfc"
	@echo "  python rom_builder/run_rom.py rom.sfc"
