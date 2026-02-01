# LLVM Backend Development Environment
# =====================================
# Makefile for building LLVM and developing custom backends

# Configuration
# -------------
BACKEND_NAME ?= W65816
LLVM_VERSION ?= main
LLVM_REPO ?= https://github.com/llvm/llvm-project.git

# Directories
ROOT_DIR := $(shell pwd)
SRC_DIR := $(ROOT_DIR)/src/llvm-project
BUILD_DIR := $(ROOT_DIR)/build
INSTALL_DIR := $(ROOT_DIR)/install
BACKEND_DIR := $(ROOT_DIR)/backend/$(BACKEND_NAME)

# Build settings
JOBS ?= $(shell sysctl -n hw.ncpu)
CMAKE_GENERATOR ?= Ninja
BUILD_TYPE ?= RelWithDebInfo

# Reference targets to build (for studying existing backends)
REFERENCE_TARGETS ?= AArch64;X86

# LLVM projects to enable
LLVM_PROJECTS ?= clang;lld

# Tools to build for fast builds
ESSENTIAL_TOOLS := llc opt llvm-mc clang lld FileCheck count not

# Detect ccache
CCACHE := $(shell command -v ccache 2>/dev/null)

# Colors for output
GREEN := \033[0;32m
YELLOW := \033[0;33m
BLUE := \033[0;34m
RED := \033[0;31m
NC := \033[0m

.PHONY: all help deps clone setup configure build build-fast install \
        scaffold link-backend rebuild tablegen \
        clean distclean update update-submodules push-submodules \
        info list-targets test test-w65816 test-llvm \
        deps-runtime build-runtime test-runtime clean-runtime \
        build-test-runner test-integration test-c-integration \
        build-snes-demo run-snes-demo

# Default target
all: help

# =============================================================================
# Help
# =============================================================================

help:
	@echo "$(BLUE)LLVM Backend Development Environment$(NC)"
	@echo "======================================"
	@echo ""
	@echo "$(GREEN)Setup & Dependencies:$(NC)"
	@echo "  make deps          - Install build dependencies (ninja, ccache)"
	@echo "  make clone         - Clone LLVM monorepo (shallow clone)"
	@echo "  make clone-full    - Clone LLVM with full history"
	@echo "  make setup         - Full initial setup (deps + clone)"
	@echo ""
	@echo "$(GREEN)Build Targets:$(NC)"
	@echo "  make configure     - Configure LLVM with CMake"
	@echo "  make build         - Full LLVM build"
	@echo "  make build-fast    - Build only essential tools (llc, opt, llvm-mc, clang)"
	@echo "  make install       - Install to local prefix"
	@echo ""
	@echo "$(GREEN)Backend Development:$(NC)"
	@echo "  make scaffold      - Create scaffold for new backend"
	@echo "  make link-backend  - Symlink backend into LLVM source tree"
	@echo "  make unlink-backend - Remove backend symlink from LLVM source tree"
	@echo "  make rebuild       - Incremental rebuild after changes"
	@echo "  make tablegen      - Run TableGen for backend"
	@echo ""
	@echo "$(GREEN)Testing:$(NC)"
	@echo "  make test               - Run ALL W65816 tests (backend + integration + runtime)"
	@echo "  make test-w65816        - Run W65816 backend FileCheck tests"
	@echo "  make test-integration   - Run integration tests (compile IR, execute)"
	@echo "  make test-runtime       - Run runtime library tests (49 tests)"
	@echo "  make test-llvm          - Run full LLVM test suite"
	@echo ""
	@echo "$(GREEN)Maintenance:$(NC)"
	@echo "  make clean         - Clean build directory"
	@echo "  make distclean     - Remove all generated files"
	@echo "  make update            - Update LLVM source (git pull)"
	@echo "  make update-submodules - Update submodules to latest commits"
	@echo "  make push-submodules   - Push submodule changes to remote"
	@echo "  make info              - Show environment information"
	@echo "  make list-targets      - List available LLVM targets"
	@echo ""
	@echo "$(GREEN)W65816 Runtime Library:$(NC)"
	@echo "  make deps-runtime  - Install cc65 toolchain"
	@echo "  make build-runtime - Build runtime library (w65816_runtime.o)"
	@echo "  make clean-runtime - Clean runtime build files"
	@echo ""
	@echo "$(GREEN)W65816 Integration Testing:$(NC)"
	@echo "  make build-test-runner  - Build 816CE-based CPU emulator runner"
	@echo "  make test-integration-verbose - Run with verbose output"
	@echo "  make test-c-integration - Run C integration tests (compile C, execute)"
	@echo "  make test-c-integration-verbose - C tests with verbose output"
	@echo ""
	@echo "$(GREEN)SNES Demo:$(NC)"
	@echo "  make build-snes-demo    - Build SNES ROM from C code (uses LLVM backend)"
	@echo "  make build-snes-test    - Build standalone test ROM (pure assembly)"
	@echo "  make run-snes-demo      - Build and run C demo in SNES emulator"
	@echo "  make run-snes-test      - Build and run test ROM in SNES emulator"
	@echo ""
	@echo "$(GREEN)Configuration Variables:$(NC)"
	@echo "  BACKEND_NAME=$(BACKEND_NAME)"
	@echo "  LLVM_VERSION=$(LLVM_VERSION)"
	@echo "  BUILD_TYPE=$(BUILD_TYPE)"
	@echo "  JOBS=$(JOBS)"
	@echo ""
	@echo "Example: make BACKEND_NAME=RISCV64Custom scaffold"

# =============================================================================
# Setup & Dependencies
# =============================================================================

deps:
	@echo "$(BLUE)Installing build dependencies...$(NC)"
	@if ! command -v ninja >/dev/null 2>&1; then \
		echo "Installing ninja..."; \
		brew install ninja; \
	else \
		echo "$(GREEN)ninja already installed$(NC)"; \
	fi
	@if ! command -v ccache >/dev/null 2>&1; then \
		echo "Installing ccache..."; \
		brew install ccache; \
	else \
		echo "$(GREEN)ccache already installed$(NC)"; \
	fi
	@echo "$(GREEN)Dependencies installed successfully$(NC)"

clone:
	@echo "$(BLUE)Cloning LLVM monorepo (shallow clone)...$(NC)"
	@mkdir -p $(ROOT_DIR)/src
	@if [ ! -d "$(SRC_DIR)" ]; then \
		git clone --depth 1 --branch $(LLVM_VERSION) $(LLVM_REPO) $(SRC_DIR); \
		echo "$(GREEN)LLVM cloned successfully$(NC)"; \
	else \
		echo "$(YELLOW)LLVM source already exists at $(SRC_DIR)$(NC)"; \
	fi

clone-full:
	@echo "$(BLUE)Cloning LLVM monorepo (full history)...$(NC)"
	@mkdir -p $(ROOT_DIR)/src
	@if [ ! -d "$(SRC_DIR)" ]; then \
		git clone --branch $(LLVM_VERSION) $(LLVM_REPO) $(SRC_DIR); \
		echo "$(GREEN)LLVM cloned successfully$(NC)"; \
	else \
		echo "$(YELLOW)LLVM source already exists at $(SRC_DIR)$(NC)"; \
	fi

setup: deps clone
	@echo "$(GREEN)Setup complete! Run 'make configure' next.$(NC)"

# =============================================================================
# Build Targets
# =============================================================================

configure:
	@echo "$(BLUE)Configuring LLVM...$(NC)"
	@mkdir -p $(BUILD_DIR)
	@EXPERIMENTAL_TARGET=""; \
	if [ -d "$(SRC_DIR)/llvm/lib/Target/$(BACKEND_NAME)" ]; then \
		EXPERIMENTAL_TARGET="$(BACKEND_NAME)"; \
		echo "$(GREEN)Detected linked backend: $(BACKEND_NAME)$(NC)"; \
	fi; \
	cd $(BUILD_DIR) && cmake -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		\
		-DLLVM_ENABLE_ASSERTIONS=ON \
		-DLLVM_TARGETS_TO_BUILD="$(REFERENCE_TARGETS)" \
		-DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD="$$EXPERIMENTAL_TARGET" \
		-DLLVM_ENABLE_PROJECTS="$(LLVM_PROJECTS)" \
		\
		-DLLVM_CCACHE_BUILD=$(if $(CCACHE),ON,OFF) \
		-DLLVM_OPTIMIZED_TABLEGEN=ON \
		-DLLVM_PARALLEL_LINK_JOBS=4 \
		\
		-DLLVM_ENABLE_BINDINGS=OFF \
		-DLLVM_ENABLE_OCAMLDOC=OFF \
		-DLLVM_ENABLE_Z3_SOLVER=OFF \
		-DLLVM_INCLUDE_BENCHMARKS=OFF \
		-DLLVM_INCLUDE_EXAMPLES=OFF \
		-DLLVM_INCLUDE_DOCS=OFF \
		\
		-DCLANG_ENABLE_ARCMT=OFF \
		-DCLANG_ENABLE_STATIC_ANALYZER=OFF \
		\
		$(SRC_DIR)/llvm
	@echo "$(GREEN)Configuration complete!$(NC)"
	@if [ -f "$(BUILD_DIR)/compile_commands.json" ]; then \
		ln -sf $(BUILD_DIR)/compile_commands.json $(ROOT_DIR)/compile_commands.json; \
		echo "$(GREEN)Linked compile_commands.json to project root$(NC)"; \
	fi

configure-experimental:
	@echo "$(BLUE)Configuring LLVM with experimental backend $(BACKEND_NAME)...$(NC)"
	@if [ ! -L "$(SRC_DIR)/llvm/lib/Target/$(BACKEND_NAME)" ]; then \
		echo "$(RED)Error: Backend not linked. Run 'make link-backend' first.$(NC)"; \
		exit 1; \
	fi
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		\
		-DLLVM_ENABLE_ASSERTIONS=ON \
		-DLLVM_TARGETS_TO_BUILD="$(REFERENCE_TARGETS)" \
		-DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD="$(BACKEND_NAME)" \
		-DLLVM_ENABLE_PROJECTS="$(LLVM_PROJECTS)" \
		\
		-DLLVM_CCACHE_BUILD=$(if $(CCACHE),ON,OFF) \
		-DLLVM_OPTIMIZED_TABLEGEN=ON \
		-DLLVM_PARALLEL_LINK_JOBS=4 \
		\
		-DLLVM_ENABLE_BINDINGS=OFF \
		-DLLVM_ENABLE_OCAMLDOC=OFF \
		-DLLVM_ENABLE_Z3_SOLVER=OFF \
		-DLLVM_INCLUDE_BENCHMARKS=OFF \
		-DLLVM_INCLUDE_EXAMPLES=OFF \
		-DLLVM_INCLUDE_DOCS=OFF \
		\
		-DCLANG_ENABLE_ARCMT=OFF \
		-DCLANG_ENABLE_STATIC_ANALYZER=OFF \
		\
		$(SRC_DIR)/llvm
	@echo "$(GREEN)Configuration with experimental backend complete!$(NC)"

build:
	@echo "$(BLUE)Building LLVM (full build)...$(NC)"
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		echo "$(RED)Error: Build directory not found. Run 'make configure' first.$(NC)"; \
		exit 1; \
	fi
	@cmake --build $(BUILD_DIR) -j $(JOBS)
	@echo "$(GREEN)Build complete!$(NC)"

build-fast:
	@echo "$(BLUE)Building essential tools: $(ESSENTIAL_TOOLS)...$(NC)"
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		echo "$(RED)Error: Build directory not found. Run 'make configure' first.$(NC)"; \
		exit 1; \
	fi
	@cmake --build $(BUILD_DIR) -j $(JOBS) -- $(ESSENTIAL_TOOLS)
	@echo "$(GREEN)Build complete!$(NC)"

install:
	@echo "$(BLUE)Installing LLVM to $(INSTALL_DIR)...$(NC)"
	@cmake --build $(BUILD_DIR) --target install
	@echo "$(GREEN)Installation complete!$(NC)"
	@echo "Add to your PATH: export PATH=$(INSTALL_DIR)/bin:\$$PATH"

# =============================================================================
# Backend Development
# =============================================================================

scaffold:
	@echo "$(BLUE)Creating backend scaffold for $(BACKEND_NAME)...$(NC)"
	@if [ -d "$(BACKEND_DIR)" ]; then \
		echo "$(RED)Error: Backend directory already exists at $(BACKEND_DIR)$(NC)"; \
		exit 1; \
	fi
	@mkdir -p $(BACKEND_DIR)
	@# Create basic directory structure
	@mkdir -p $(BACKEND_DIR)/MCTargetDesc
	@mkdir -p $(BACKEND_DIR)/TargetInfo
	@mkdir -p $(BACKEND_DIR)/AsmParser
	@mkdir -p $(BACKEND_DIR)/Disassembler
	@# Create placeholder CMakeLists.txt
	@echo 'add_llvm_component_group($(BACKEND_NAME))' > $(BACKEND_DIR)/CMakeLists.txt
	@echo '' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo 'set(LLVM_TARGET_DEFINITIONS $(BACKEND_NAME).td)' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo 'tablegen(LLVM $(BACKEND_NAME)GenRegisterInfo.inc -gen-register-info)' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo 'tablegen(LLVM $(BACKEND_NAME)GenInstrInfo.inc -gen-instr-info)' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo 'tablegen(LLVM $(BACKEND_NAME)GenSubtargetInfo.inc -gen-subtarget)' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo 'tablegen(LLVM $(BACKEND_NAME)GenDAGISel.inc -gen-dag-isel)' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo 'tablegen(LLVM $(BACKEND_NAME)GenCallingConv.inc -gen-callingconv)' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo 'tablegen(LLVM $(BACKEND_NAME)GenAsmWriter.inc -gen-asm-writer)' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo 'add_public_tablegen_target($(BACKEND_NAME)CommonTableGen)' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo 'add_llvm_target($(BACKEND_NAME)CodeGen' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  $(BACKEND_NAME)TargetMachine.cpp' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  LINK_COMPONENTS' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  Analysis' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  AsmPrinter' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  CodeGen' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  CodeGenTypes' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  Core' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  MC' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  $(BACKEND_NAME)Desc' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  $(BACKEND_NAME)Info' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  SelectionDAG' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  Support' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  Target' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  GlobalISel' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  ADD_TO_COMPONENT' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  $(BACKEND_NAME)' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '  )' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo '' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo 'add_subdirectory(MCTargetDesc)' >> $(BACKEND_DIR)/CMakeLists.txt
	@echo 'add_subdirectory(TargetInfo)' >> $(BACKEND_DIR)/CMakeLists.txt
	@# Create placeholder TableGen file
	@echo '//===-- $(BACKEND_NAME).td - Target definition file -------*- tablegen -*-===//' > $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '//' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '// See https://llvm.org/LICENSE.txt for license information.' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '//' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '//===----------------------------------------------------------------------===//' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo 'include "llvm/Target/Target.td"' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '//===----------------------------------------------------------------------===//' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '// Target Declaration' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '//===----------------------------------------------------------------------===//' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo 'def $(BACKEND_NAME)InstrInfo : InstrInfo;' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo 'def $(BACKEND_NAME) : Target {' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '  let InstructionSet = $(BACKEND_NAME)InstrInfo;' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@echo '}' >> $(BACKEND_DIR)/$(BACKEND_NAME).td
	@# Create MCTargetDesc CMakeLists.txt
	@echo 'add_llvm_component_library(LLVM$(BACKEND_NAME)Desc' > $(BACKEND_DIR)/MCTargetDesc/CMakeLists.txt
	@echo '  $(BACKEND_NAME)MCTargetDesc.cpp' >> $(BACKEND_DIR)/MCTargetDesc/CMakeLists.txt
	@echo '' >> $(BACKEND_DIR)/MCTargetDesc/CMakeLists.txt
	@echo '  LINK_COMPONENTS' >> $(BACKEND_DIR)/MCTargetDesc/CMakeLists.txt
	@echo '  MC' >> $(BACKEND_DIR)/MCTargetDesc/CMakeLists.txt
	@echo '  $(BACKEND_NAME)Info' >> $(BACKEND_DIR)/MCTargetDesc/CMakeLists.txt
	@echo '  Support' >> $(BACKEND_DIR)/MCTargetDesc/CMakeLists.txt
	@echo '' >> $(BACKEND_DIR)/MCTargetDesc/CMakeLists.txt
	@echo '  ADD_TO_COMPONENT' >> $(BACKEND_DIR)/MCTargetDesc/CMakeLists.txt
	@echo '  $(BACKEND_NAME)' >> $(BACKEND_DIR)/MCTargetDesc/CMakeLists.txt
	@echo '  )' >> $(BACKEND_DIR)/MCTargetDesc/CMakeLists.txt
	@# Create TargetInfo CMakeLists.txt
	@echo 'add_llvm_component_library(LLVM$(BACKEND_NAME)Info' > $(BACKEND_DIR)/TargetInfo/CMakeLists.txt
	@echo '  $(BACKEND_NAME)TargetInfo.cpp' >> $(BACKEND_DIR)/TargetInfo/CMakeLists.txt
	@echo '' >> $(BACKEND_DIR)/TargetInfo/CMakeLists.txt
	@echo '  LINK_COMPONENTS' >> $(BACKEND_DIR)/TargetInfo/CMakeLists.txt
	@echo '  MC' >> $(BACKEND_DIR)/TargetInfo/CMakeLists.txt
	@echo '  Support' >> $(BACKEND_DIR)/TargetInfo/CMakeLists.txt
	@echo '' >> $(BACKEND_DIR)/TargetInfo/CMakeLists.txt
	@echo '  ADD_TO_COMPONENT' >> $(BACKEND_DIR)/TargetInfo/CMakeLists.txt
	@echo '  $(BACKEND_NAME)' >> $(BACKEND_DIR)/TargetInfo/CMakeLists.txt
	@echo '  )' >> $(BACKEND_DIR)/TargetInfo/CMakeLists.txt
	@# Create placeholder source files
	@echo '// $(BACKEND_NAME)TargetInfo.cpp' > $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo '#include "llvm/MC/TargetRegistry.h"' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo '' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo 'using namespace llvm;' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo '' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo 'Target &llvm::getThe$(BACKEND_NAME)Target() {' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo '  static Target The$(BACKEND_NAME)Target;' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo '  return The$(BACKEND_NAME)Target;' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo '}' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo '' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo 'extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitialize$(BACKEND_NAME)TargetInfo() {' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo '  RegisterTarget<Triple::UnknownArch> X(getThe$(BACKEND_NAME)Target(), "$(shell echo $(BACKEND_NAME) | tr A-Z a-z)",' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo '                                        "$(BACKEND_NAME)", "$(BACKEND_NAME)");' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo '}' >> $(BACKEND_DIR)/TargetInfo/$(BACKEND_NAME)TargetInfo.cpp
	@echo '// $(BACKEND_NAME)MCTargetDesc.cpp' > $(BACKEND_DIR)/MCTargetDesc/$(BACKEND_NAME)MCTargetDesc.cpp
	@echo '#include "llvm/MC/TargetRegistry.h"' >> $(BACKEND_DIR)/MCTargetDesc/$(BACKEND_NAME)MCTargetDesc.cpp
	@echo '' >> $(BACKEND_DIR)/MCTargetDesc/$(BACKEND_NAME)MCTargetDesc.cpp
	@echo 'using namespace llvm;' >> $(BACKEND_DIR)/MCTargetDesc/$(BACKEND_NAME)MCTargetDesc.cpp
	@echo '' >> $(BACKEND_DIR)/MCTargetDesc/$(BACKEND_NAME)MCTargetDesc.cpp
	@echo 'extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitialize$(BACKEND_NAME)TargetMC() {' >> $(BACKEND_DIR)/MCTargetDesc/$(BACKEND_NAME)MCTargetDesc.cpp
	@echo '  // TODO: Register MC components' >> $(BACKEND_DIR)/MCTargetDesc/$(BACKEND_NAME)MCTargetDesc.cpp
	@echo '}' >> $(BACKEND_DIR)/MCTargetDesc/$(BACKEND_NAME)MCTargetDesc.cpp
	@echo '// $(BACKEND_NAME)TargetMachine.cpp' > $(BACKEND_DIR)/$(BACKEND_NAME)TargetMachine.cpp
	@echo '#include "llvm/MC/TargetRegistry.h"' >> $(BACKEND_DIR)/$(BACKEND_NAME)TargetMachine.cpp
	@echo '#include "llvm/Target/TargetMachine.h"' >> $(BACKEND_DIR)/$(BACKEND_NAME)TargetMachine.cpp
	@echo '' >> $(BACKEND_DIR)/$(BACKEND_NAME)TargetMachine.cpp
	@echo 'using namespace llvm;' >> $(BACKEND_DIR)/$(BACKEND_NAME)TargetMachine.cpp
	@echo '' >> $(BACKEND_DIR)/$(BACKEND_NAME)TargetMachine.cpp
	@echo 'extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitialize$(BACKEND_NAME)Target() {' >> $(BACKEND_DIR)/$(BACKEND_NAME)TargetMachine.cpp
	@echo '  // TODO: Register target machine' >> $(BACKEND_DIR)/$(BACKEND_NAME)TargetMachine.cpp
	@echo '}' >> $(BACKEND_DIR)/$(BACKEND_NAME)TargetMachine.cpp
	@echo "$(GREEN)Backend scaffold created at $(BACKEND_DIR)$(NC)"
	@echo "Next steps:"
	@echo "  1. Edit the TableGen and source files in $(BACKEND_DIR)"
	@echo "  2. Run 'make link-backend' to symlink into LLVM tree"
	@echo "  3. Run 'make configure-experimental' to configure with your backend"
	@echo "  4. Run 'make build-fast' to build"

link-backend:
	@echo "$(BLUE)Linking backend $(BACKEND_NAME) into LLVM source tree...$(NC)"
	@if [ ! -d "$(BACKEND_DIR)" ]; then \
		echo "$(RED)Error: Backend directory not found at $(BACKEND_DIR)$(NC)"; \
		echo "Run 'make scaffold' first."; \
		exit 1; \
	fi
	@if [ ! -d "$(SRC_DIR)" ]; then \
		echo "$(RED)Error: LLVM source not found at $(SRC_DIR)$(NC)"; \
		echo "Run 'make clone' first."; \
		exit 1; \
	fi
	@if [ -e "$(SRC_DIR)/llvm/lib/Target/$(BACKEND_NAME)" ]; then \
		echo "$(YELLOW)Warning: Target already exists, removing old link...$(NC)"; \
		rm -f "$(SRC_DIR)/llvm/lib/Target/$(BACKEND_NAME)"; \
	fi
	@ln -s $(BACKEND_DIR) $(SRC_DIR)/llvm/lib/Target/$(BACKEND_NAME)
	@echo "$(GREEN)Backend linked successfully!$(NC)"
	@echo "Run 'make configure-experimental' to configure LLVM with your backend."

unlink-backend:
	@echo "$(BLUE)Unlinking backend $(BACKEND_NAME) from LLVM source tree...$(NC)"
	@if [ -L "$(SRC_DIR)/llvm/lib/Target/$(BACKEND_NAME)" ]; then \
		rm $(SRC_DIR)/llvm/lib/Target/$(BACKEND_NAME); \
		echo "$(GREEN)Backend unlinked successfully!$(NC)"; \
	else \
		echo "$(YELLOW)No symlink found for $(BACKEND_NAME)$(NC)"; \
	fi

rebuild:
	@echo "$(BLUE)Rebuilding after backend changes...$(NC)"
	@cmake --build $(BUILD_DIR) -j $(JOBS)
	@echo "$(GREEN)Rebuild complete!$(NC)"

tablegen:
	@echo "$(BLUE)Running TableGen for $(BACKEND_NAME)...$(NC)"
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		echo "$(RED)Error: Build directory not found. Run 'make configure' first.$(NC)"; \
		exit 1; \
	fi
	@cmake --build $(BUILD_DIR) --target $(BACKEND_NAME)CommonTableGen
	@echo "$(GREEN)TableGen complete!$(NC)"

# =============================================================================
# Maintenance
# =============================================================================

clean:
	@echo "$(BLUE)Cleaning build directory...$(NC)"
	@if [ -d "$(BUILD_DIR)" ]; then \
		cmake --build $(BUILD_DIR) --target clean; \
		echo "$(GREEN)Build cleaned$(NC)"; \
	else \
		echo "$(YELLOW)Nothing to clean$(NC)"; \
	fi

distclean:
	@echo "$(BLUE)Removing all generated files...$(NC)"
	@rm -rf $(BUILD_DIR) $(INSTALL_DIR)
	@rm -f $(ROOT_DIR)/compile_commands.json
	@echo "$(GREEN)All generated files removed$(NC)"

update:
	@echo "$(BLUE)Updating LLVM source...$(NC)"
	@if [ -d "$(SRC_DIR)" ]; then \
		cd $(SRC_DIR) && git pull; \
		echo "$(GREEN)LLVM updated$(NC)"; \
	else \
		echo "$(RED)Error: LLVM source not found$(NC)"; \
		exit 1; \
	fi

update-submodules:
	@echo "$(BLUE)Updating submodules to latest commits...$(NC)"
	@git submodule update --remote --merge
	@echo "$(GREEN)Submodules updated$(NC)"
	@echo "Run 'git add src/llvm-project && git commit' to record the update"

push-submodules:
	@echo "$(BLUE)Pushing submodule changes...$(NC)"
	@cd $(SRC_DIR) && git push
	@echo "$(GREEN)Submodule changes pushed$(NC)"

info:
	@echo "$(BLUE)Environment Information$(NC)"
	@echo "========================"
	@echo ""
	@echo "$(GREEN)Directories:$(NC)"
	@echo "  Root:    $(ROOT_DIR)"
	@echo "  Source:  $(SRC_DIR)"
	@echo "  Build:   $(BUILD_DIR)"
	@echo "  Install: $(INSTALL_DIR)"
	@echo "  Backend: $(BACKEND_DIR)"
	@echo ""
	@echo "$(GREEN)Build Settings:$(NC)"
	@echo "  Backend Name:      $(BACKEND_NAME)"
	@echo "  Build Type:        $(BUILD_TYPE)"
	@echo "  Parallel Jobs:     $(JOBS)"
	@echo "  Reference Targets: $(REFERENCE_TARGETS)"
	@echo "  LLVM Projects:     $(LLVM_PROJECTS)"
	@echo "  ccache:            $(if $(CCACHE),$(CCACHE),not found)"
	@echo ""
	@echo "$(GREEN)System:$(NC)"
	@echo "  CMake:   $(shell cmake --version | head -1)"
	@echo "  Ninja:   $(shell ninja --version 2>/dev/null || echo 'not installed')"
	@echo "  Clang:   $(shell clang --version | head -1)"
	@echo ""
	@echo "$(GREEN)Status:$(NC)"
	@if [ -d "$(SRC_DIR)" ]; then \
		echo "  LLVM Source: $(GREEN)cloned$(NC)"; \
		cd $(SRC_DIR) && echo "  LLVM Commit: $$(git rev-parse --short HEAD)"; \
	else \
		echo "  LLVM Source: $(YELLOW)not cloned$(NC)"; \
	fi
	@if [ -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		echo "  Build:       $(GREEN)configured$(NC)"; \
	else \
		echo "  Build:       $(YELLOW)not configured$(NC)"; \
	fi
	@if [ -d "$(BACKEND_DIR)" ]; then \
		echo "  Backend:     $(GREEN)scaffold exists$(NC)"; \
		if [ -L "$(SRC_DIR)/llvm/lib/Target/$(BACKEND_NAME)" ]; then \
			echo "  Backend:     $(GREEN)linked to LLVM$(NC)"; \
		else \
			echo "  Backend:     $(YELLOW)not linked$(NC)"; \
		fi \
	else \
		echo "  Backend:     $(YELLOW)no scaffold$(NC)"; \
	fi

list-targets:
	@echo "$(BLUE)Available LLVM Targets:$(NC)"
	@if [ -x "$(BUILD_DIR)/bin/llc" ]; then \
		$(BUILD_DIR)/bin/llc --version | grep -A 100 "Registered Targets:"; \
	else \
		echo "$(RED)Error: llc not built. Run 'make build-fast' first.$(NC)"; \
	fi

test-llvm:
	@echo "$(BLUE)Running LLVM tests...$(NC)"
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		echo "$(RED)Error: Build directory not found. Run 'make configure' first.$(NC)"; \
		exit 1; \
	fi
	@cmake --build $(BUILD_DIR) --target check-llvm
	@echo "$(GREEN)Tests complete!$(NC)"

# Master test target - runs all W65816 tests in order
test: test-w65816 test-integration test-runtime
	@echo ""
	@echo "$(GREEN)=============================================$(NC)"
	@echo "$(GREEN)All W65816 tests passed!$(NC)"
	@echo "$(GREEN)=============================================$(NC)"

test-backend:
	@echo "$(BLUE)Running tests for $(BACKEND_NAME) backend...$(NC)"
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		echo "$(RED)Error: Build directory not found. Run 'make configure' first.$(NC)"; \
		exit 1; \
	fi
	@cmake --build $(BUILD_DIR) --target check-llvm-codegen-$(shell echo $(BACKEND_NAME) | tr A-Z a-z) 2>/dev/null || \
		echo "$(YELLOW)No specific tests found for $(BACKEND_NAME). Run 'make test' for all tests.$(NC)"

# W65816-specific test target (uses LLVM's lit test runner)
TEST_DIR := $(SRC_DIR)/llvm/test/CodeGen/W65816
test-w65816:
	@echo "$(BLUE)Running W65816 backend tests...$(NC)"
	@if [ ! -x "$(BUILD_DIR)/bin/llvm-lit" ]; then \
		echo "$(RED)Error: llvm-lit not built. Run 'make build-fast' first.$(NC)"; \
		exit 1; \
	fi
	@$(BUILD_DIR)/bin/llvm-lit -v $(TEST_DIR)

# =============================================================================
# W65816 Runtime Library
# =============================================================================

RUNTIME_DIR := $(SRC_DIR)/llvm/lib/Target/W65816/runtime
RUNTIME_BUILD_DIR := $(BUILD_DIR)/w65816-runtime

deps-runtime:
	@echo "$(BLUE)Installing cc65 toolchain for runtime builds...$(NC)"
	@if ! command -v ca65 >/dev/null 2>&1; then \
		echo "Installing cc65..."; \
		brew install cc65; \
	else \
		echo "$(GREEN)cc65 already installed$(NC)"; \
	fi

build-runtime: deps-runtime
	@echo "$(BLUE)Building W65816 runtime library...$(NC)"
	@mkdir -p $(RUNTIME_BUILD_DIR)
	@ca65 --cpu 65816 -o $(RUNTIME_BUILD_DIR)/w65816_runtime.o $(RUNTIME_DIR)/w65816_runtime.s
	@echo "$(GREEN)Runtime library built: $(RUNTIME_BUILD_DIR)/w65816_runtime.o$(NC)"

build-runtime-test: deps-runtime
	@echo "$(BLUE)Building W65816 runtime test suite...$(NC)"
	@mkdir -p $(RUNTIME_BUILD_DIR)
	@ca65 --cpu 65816 -o $(RUNTIME_BUILD_DIR)/w65816_runtime.o $(RUNTIME_DIR)/w65816_runtime.s
	@ca65 --cpu 65816 -o $(RUNTIME_BUILD_DIR)/test_runtime.o $(RUNTIME_DIR)/test_runtime.s
	@ld65 -o $(RUNTIME_BUILD_DIR)/test_runtime.bin \
		$(RUNTIME_BUILD_DIR)/test_runtime.o \
		$(RUNTIME_BUILD_DIR)/w65816_runtime.o \
		-C $(RUNTIME_DIR)/test_runtime.cfg
	@echo "$(GREEN)Runtime test built: $(RUNTIME_BUILD_DIR)/test_runtime.bin$(NC)"
	@echo ""
	@echo "To run the test:"
	@echo "  1. Load test_runtime.bin at address 0x8000 in a 65816 emulator"
	@echo "  2. Execute from 0x8000"
	@echo "  3. Check memory after halt:"
	@echo "     0x0000 = Total tests (expect 49)"
	@echo "     0x0002 = Passed"
	@echo "     0x0004 = Failed"
	@echo "     0x0006 = 0x600D (pass) or 0xFA11 (fail)"

test-runtime: build-runtime-test build-test-runner
	@echo "$(BLUE)Running runtime library tests...$(NC)"
	@$(BUILD_DIR)/bin/w65816-runner -e 0x600D -o 0x8000 --result-addr 0x0006 $(RUNTIME_BUILD_DIR)/test_runtime.bin && \
		echo "$(GREEN)Runtime tests passed!$(NC)" || \
		(echo "$(RED)Runtime tests failed!$(NC)"; exit 1)

clean-runtime:
	@echo "$(BLUE)Cleaning runtime build files...$(NC)"
	@rm -rf $(RUNTIME_BUILD_DIR)
	@echo "$(GREEN)Runtime build files cleaned$(NC)"

# =============================================================================
# W65816 Integration Testing
# =============================================================================

RUNNER_DIR := $(ROOT_DIR)/tools/w65816-runner
RUNNER_BIN := $(BUILD_DIR)/bin/w65816-runner
CPU_DIR := $(ROOT_DIR)/tools/816ce/src/cpu

build-test-runner: deps-runtime
	@echo "$(BLUE)Building W65816 integration test runner...$(NC)"
	@if [ ! -d "$(CPU_DIR)" ]; then \
		echo "$(RED)Error: 816CE not found. Clone it to tools/816ce$(NC)"; \
		exit 1; \
	fi
	@mkdir -p $(BUILD_DIR)/bin
	@$(CC) -o $(RUNNER_BIN) \
		$(RUNNER_DIR)/main.c \
		$(CPU_DIR)/65816.c \
		$(CPU_DIR)/65816-util.c \
		$(CPU_DIR)/65816-ops.c \
		-I$(CPU_DIR) \
		-std=c99 -O2 -Wall
	@echo "$(GREEN)Test runner built: $(RUNNER_BIN)$(NC)"

test-integration: build-test-runner
	@echo "$(BLUE)Running W65816 integration tests...$(NC)"
	@if [ ! -x "$(RUNNER_BIN)" ]; then \
		echo "$(RED)Error: Test runner not built. Run 'make build-test-runner' first.$(NC)"; \
		exit 1; \
	fi
	@python3 $(RUNNER_DIR)/run-tests.py -b $(BUILD_DIR) -r $(RUNNER_DIR)

test-integration-verbose: build-test-runner
	@python3 $(RUNNER_DIR)/run-tests.py -b $(BUILD_DIR) -r $(RUNNER_DIR) -v

# =============================================================================
# SNES Demo (Real Emulator Testing)
# =============================================================================

SNES_DIR := $(ROOT_DIR)/snes
SNES_BUILD_DIR := $(BUILD_DIR)/snes
SNES_BUILDER := $(ROOT_DIR)/tools/snes-builder

build-snes-demo: deps-runtime
	@echo "$(BLUE)Building SNES demo ROM...$(NC)"
	@mkdir -p $(SNES_BUILD_DIR)
	@python3 $(SNES_BUILDER)/build_rom.py $(SNES_DIR)/demo.c $(SNES_BUILD_DIR)/demo.sfc
	@echo "$(GREEN)SNES ROM built: $(SNES_BUILD_DIR)/demo.sfc$(NC)"

build-snes-text-demo: deps-runtime
	@echo "$(BLUE)Building SNES text demo ROM...$(NC)"
	@mkdir -p $(SNES_BUILD_DIR)
	@python3 $(SNES_BUILDER)/build_rom.py $(SNES_DIR)/text_demo.c $(SNES_BUILD_DIR)/text_demo.sfc
	@echo "$(GREEN)SNES text demo ROM built: $(SNES_BUILD_DIR)/text_demo.sfc$(NC)"

build-snes-test:
	@echo "$(BLUE)Building standalone SNES test ROM (pure assembly)...$(NC)"
	@mkdir -p $(SNES_BUILD_DIR)
	@ca65 --cpu 65816 -o $(SNES_BUILD_DIR)/test_crt0.o $(SNES_DIR)/test_crt0.s
	@ld65 -C $(SNES_DIR)/lorom.cfg -o $(SNES_BUILD_DIR)/test.sfc $(SNES_BUILD_DIR)/test_crt0.o
	@python3 $(SNES_BUILDER)/fix_checksum.py $(SNES_BUILD_DIR)/test.sfc
	@echo "$(GREEN)Test ROM built: $(SNES_BUILD_DIR)/test.sfc$(NC)"

run-snes-demo: build-snes-demo
	@python3 $(SNES_BUILDER)/run_rom.py $(SNES_BUILD_DIR)/demo.sfc

run-snes-text-demo: build-snes-text-demo
	@python3 $(SNES_BUILDER)/run_rom.py $(SNES_BUILD_DIR)/text_demo.sfc

run-snes-test: build-snes-test
	@python3 $(SNES_BUILDER)/run_rom.py $(SNES_BUILD_DIR)/test.sfc

build-snes-bounce-demo: deps-runtime
	@echo "$(BLUE)Building SNES bounce demo ROM...$(NC)"
	@mkdir -p $(SNES_BUILD_DIR)
	@python3 $(SNES_BUILDER)/build_rom.py $(SNES_DIR)/bounce_demo.c $(SNES_BUILD_DIR)/bounce_demo.sfc
	@echo "$(GREEN)SNES bounce demo ROM built: $(SNES_BUILD_DIR)/bounce_demo.sfc$(NC)"

run-snes-bounce-demo: build-snes-bounce-demo
	@python3 $(SNES_BUILDER)/run_rom.py $(SNES_BUILD_DIR)/bounce_demo.sfc

# =============================================================================
# W65816 C Integration Testing
# =============================================================================

LLVM_RUNTIME_SRC := $(ROOT_DIR)/src/llvm-project/llvm/lib/Target/W65816/runtime/w65816_runtime.s
LLVM_RUNTIME_CFG := $(ROOT_DIR)/test/c-integration/runtime/llvm_runtime.cfg
C_RUNTIME_BUILD_DIR := $(BUILD_DIR)/w65816-runtime

build-c-runtime: deps-runtime
	@echo "$(BLUE)Building W65816 runtime library...$(NC)"
	@mkdir -p $(C_RUNTIME_BUILD_DIR)
	@ca65 --cpu 65816 --listing $(C_RUNTIME_BUILD_DIR)/w65816_runtime.lst -o $(C_RUNTIME_BUILD_DIR)/w65816_runtime.o $(LLVM_RUNTIME_SRC)
	@ld65 -C $(LLVM_RUNTIME_CFG) -m $(C_RUNTIME_BUILD_DIR)/w65816_runtime.map -o $(C_RUNTIME_BUILD_DIR)/w65816_runtime.bin $(C_RUNTIME_BUILD_DIR)/w65816_runtime.o
	@echo "$(GREEN)W65816 runtime built: $(C_RUNTIME_BUILD_DIR)/w65816_runtime.bin$(NC)"

test-c-integration: build-test-runner build-c-runtime
	@echo "$(BLUE)Running W65816 C integration tests...$(NC)"
	@python3 test/c-integration/run_tests.py -b $(BUILD_DIR)

test-c-integration-verbose: build-test-runner build-c-runtime
	@python3 test/c-integration/run_tests.py -b $(BUILD_DIR) -v
