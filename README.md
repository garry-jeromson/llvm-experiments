# LLVM Backend Development Environment

A reproducible environment for developing LLVM backends on macOS (Apple Silicon).

## Quick Start

```bash
# 1. Install dependencies (ninja, ccache)
make deps

# 2. Clone LLVM (shallow clone, ~2GB)
make clone

# 3. Configure LLVM build
make configure

# 4. Build essential tools (llc, opt, llvm-mc, clang)
make build-fast

# 5. Verify installation
make list-targets
```

## Developing a Custom Backend

```bash
# 1. Create backend scaffold
make BACKEND_NAME=MyProcessor scaffold

# 2. Edit files in backend/MyProcessor/
#    - MyProcessor.td (TableGen definitions)
#    - MyProcessorTargetMachine.cpp
#    - TargetInfo/MyProcessorTargetInfo.cpp
#    - MCTargetDesc/MyProcessorMCTargetDesc.cpp

# 3. Link backend into LLVM source tree
make BACKEND_NAME=MyProcessor link-backend

# 4. Reconfigure with experimental backend
make BACKEND_NAME=MyProcessor configure-experimental

# 5. Build and iterate
make build-fast
make rebuild  # for incremental builds
```

## Make Targets

### Setup & Dependencies
| Target | Description |
|--------|-------------|
| `make deps` | Install ninja and ccache via Homebrew |
| `make clone` | Clone LLVM (shallow, ~2GB) |
| `make clone-full` | Clone LLVM with full history (~4GB) |
| `make setup` | Full initial setup (deps + clone) |

### Build
| Target | Description |
|--------|-------------|
| `make configure` | Configure LLVM with CMake |
| `make configure-experimental` | Configure with experimental backend |
| `make build` | Full LLVM build |
| `make build-fast` | Build only llc, opt, llvm-mc, clang |
| `make install` | Install to `./install/` |

### Backend Development
| Target | Description |
|--------|-------------|
| `make scaffold` | Create new backend scaffold |
| `make link-backend` | Symlink backend into LLVM tree |
| `make unlink-backend` | Remove backend symlink |
| `make rebuild` | Incremental rebuild |
| `make tablegen` | Run TableGen for backend |

### Maintenance
| Target | Description |
|--------|-------------|
| `make clean` | Clean build artifacts |
| `make distclean` | Remove build/, install/ |
| `make update` | Pull latest LLVM changes |
| `make info` | Show environment status |
| `make list-targets` | List LLVM targets |
| `make test` | Run LLVM tests |

## Configuration Variables

Override these on the command line:

```bash
make BACKEND_NAME=RISCV64Custom scaffold
make JOBS=8 build-fast
make BUILD_TYPE=Debug configure
```

| Variable | Default | Description |
|----------|---------|-------------|
| `BACKEND_NAME` | MyTarget | Name of your backend |
| `BUILD_TYPE` | RelWithDebInfo | CMake build type |
| `JOBS` | (CPU count) | Parallel build jobs |
| `REFERENCE_TARGETS` | AArch64;X86 | Built-in targets to include |
| `LLVM_PROJECTS` | clang;lld | LLVM projects to enable |
| `LLVM_VERSION` | main | Branch/tag to clone |

## Directory Structure

```
llvm-experiments/
├── Makefile           # Build orchestration
├── README.md          # This file
├── .gitignore         # Git ignore rules
├── src/               # LLVM source (cloned)
│   └── llvm-project/
├── build/             # CMake build directory
├── install/           # Local installation
└── backend/           # Your backend code
    └── MyTarget/      # Backend implementation
        ├── CMakeLists.txt
        ├── MyTarget.td
        ├── MyTargetTargetMachine.cpp
        ├── MCTargetDesc/
        └── TargetInfo/
```

## CMake Configuration

The build is optimized for backend development:

- **RelWithDebInfo** - Optimized with debug symbols
- **Assertions enabled** - Catch errors early
- **ccache** - Fast incremental rebuilds
- **lld linker** - Faster linking
- **Limited targets** - Only AArch64 and X86 for reference
- **Parallel link jobs capped at 4** - Prevent memory exhaustion

Disabled features (faster builds):
- Python/OCaml bindings
- Z3 solver
- Benchmarks, examples, docs
- Clang static analyzer

## Useful Commands

```bash
# Check what targets llc supports
./build/bin/llc --version

# Compile IR to assembly for your target
./build/bin/llc -march=mytarget test.ll -o test.s

# Run TableGen manually
./build/bin/llvm-tblgen -I src/llvm-project/llvm/include \
    backend/MyTarget/MyTarget.td -gen-register-info

# Generate compile_commands.json for IDE
# (automatically linked to project root after configure)
```

## Studying Existing Backends

The best way to learn is by studying existing backends:

- **AArch64** - Modern, clean, feature-complete
- **RISCV** - Relatively new, good example of modular design
- **X86** - Very complex, but comprehensive

```bash
# Browse AArch64 backend
ls src/llvm-project/llvm/lib/Target/AArch64/

# Search for patterns
grep -r "RegisterClass" src/llvm-project/llvm/lib/Target/AArch64/*.td
```

## Resources

- [LLVM Backend Tutorial](https://llvm.org/docs/WritingAnLLVMBackend.html)
- [TableGen Documentation](https://llvm.org/docs/TableGen/index.html)
- [LLVM Target-Independent Code Generator](https://llvm.org/docs/CodeGenerator.html)
