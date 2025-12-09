# Build Guide

Complete instructions for building and running the UI Engine from source.

---

## Prerequisites

### Required Tools

| Tool | Minimum Version | Purpose |
|------|----------------|---------|
| **CMake** | 3.20+ | Build system generator |
| **C++ Compiler** | C++20 support | Code compilation |
| **Git** | Any recent | Clone repository and submodules |

### Platform-Specific Compilers

#### Windows
- **MinGW-w64** (recommended) - GCC toolchain for Windows
  - Download from [WinLibs](https://winlibs.com/) or [MSYS2](https://www.msys2.org/)
  - Ensure `g++` is in PATH
- **Visual Studio 2019/2022** - MSVC toolchain
  - Install "Desktop development with C++" workload

#### Linux
- **GCC 10+** or **Clang 12+**
  ```bash
  # Ubuntu/Debian
  sudo apt install build-essential cmake git
  
  # Fedora
  sudo dnf install gcc-c++ cmake git
  ```

#### macOS
- **Xcode Command Line Tools**
  ```bash
  xcode-select --install
  ```
- CMake via Homebrew:
  ```bash
  brew install cmake
  ```

---

## Quick Build

### Clone Repository

```bash
git clone <repository-url>
cd UI_Engine
```

> **Note:** All dependencies are included in `vendor/` as Git submodules or vendored directly. No external library installation needed.

### Configure CMake

```bash
cmake -S . -B build
```

**Options:**
- Specify generator: `-G "Ninja"` or `-G "Unix Makefiles"`
- Set build type: `-DCMAKE_BUILD_TYPE=Release` (default is Debug)

### Build

```bash
cmake --build build
```

Or for multi-core compilation:
```bash
cmake --build build -j8
```

### Run Editor

```bash
# Windows
./build/bin/UICheckEditor.exe

# Linux/macOS
./build/bin/UICheckEditor
```

---

## Detailed Build Steps

### 1. Verify Compiler

Check that your compiler supports C++20:

```bash
# GCC
g++ --version  # Should be 10.0 or higher

# Clang
clang++ --version  # Should be 12.0 or higher

# MSVC (in Visual Studio Developer Command Prompt)
cl  # Should be version 19.28 or higher
```

### 2. CMake Configuration

Navigate to project root and configure:

```bash
cd UI_Engine
cmake -S . -B build
```

**What This Does:**
- Configures build system in `build/` directory
- Detects compiler and platform
- Processes all `CMakeLists.txt` files
- Builds vendor libraries (GLAD, GLFW, ImGui)

**Common Configuration Options:**

```bash
# Specify build type
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Use Ninja generator (faster builds)
cmake -S . -B build -G Ninja

# Specify compiler explicitly
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++
```

### 3. Compilation

```bash
cmake --build build
```

**Build Targets:**
- `UICheckEngine` - Static library containing engine systems
- `imgui` - Static library for ImGui
- `glad` - Static library for OpenGL loader
- `glfw` - Static library for windowing
- `UICheckEditor` - Main executable

**Parallel Compilation:**

```bash
# Use all CPU cores
cmake --build build --parallel

# Or specify core count
cmake --build build -j8
```

**Build Specific Target:**

```bash
# Only build the editor
cmake --build build --target UICheckEditor
```

### 4. Output Location

Executables are placed in `build/bin/`:

```
build/
└── bin/
    └── UICheckEditor.exe  (or UICheckEditor on Linux/macOS)
```

---

## IDE Integration

### Visual Studio Code

1. Install C++ extension: `ms-vscode.cpptools`
2. Install CMake Tools: `ms-vscode.cmake-tools`

**Configure** (`.vscode/settings.json`):
```json
{
  "cmake.sourceDirectory": "${workspaceFolder}",
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.configureOnOpen": true
}
```

3. Use CMake Tools sidebar to configure and build
4. Press `F5` to debug

### Visual Studio 2019/2022

1. Open folder: **File → Open → Folder** → Select `UI_Engine/`
2. Visual Studio auto-detects CMake
3. Select build configuration from toolbar dropdown
4. Build with `Ctrl+Shift+B`
5. Run with `F5`

### CLion

1. **File → Open** → Select `UI_Engine/`
2. CLion auto-configures CMake
3. Build from **Build → Build Project** or `Ctrl+F9`
4. Run with `Shift+F10`

---

## Troubleshooting

### CMake Errors

#### "CMake 3.20 or higher is required"

**Solution:** Update CMake
```bash
# Download latest from https://cmake.org/download/
# Or use package manager:
pip install cmake --upgrade
```

#### "Could not find OpenGL"

**Windows:** Install graphics drivers  
**Linux:**
```bash
sudo apt install libgl1-mesa-dev  # Ubuntu/Debian
sudo dnf install mesa-libGL-devel # Fedora
```

### Compiler Errors

#### "C++20 features not available"

**Solution:** Ensure compiler version meets requirements (GCC 10+, Clang 12+, MSVC 19.28+)

#### MinGW: "undefined reference to `__imp_*`"

**Solution:** Ensure linking OpenGL correctly (already handled in `Engine/CMakeLists.txt`)

### Build Errors

#### "fatal error: GLFW/glfw3.h: No such file or directory"

**Solution:** Vendor submodules not initialized
```bash
git submodule update --init --recursive
```

#### Linker errors with ImGui

**Solution:** Clean and rebuild
```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

### Runtime Errors

#### "Failed to create GLFW window"

**Causes:**
- Graphics drivers out of date
- OpenGL 4.6 not supported by GPU

**Solution:** Update graphics drivers or check GPU specifications

#### "Failed to load GLAD"

**Cause:** OpenGL context creation failed

**Solution:** Ensure GLFW window created successfully first

#### Black/blank viewport

**Causes:**
- Framebuffer creation failed
- Shader compilation error

**Solution:** Check console output for error messages

---

## Build Variants

### Debug Build (Default)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

**Features:**
- Debug symbols included
- Optimizations disabled
- Assertions enabled
- Larger binary size
- Slower runtime

### Release Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**Features:**
- Optimizations enabled (`-O3` / `/O2`)
- Debug symbols stripped
- Smaller binary size
- Faster runtime

### RelWithDebInfo Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

**Features:**
- Optimizations enabled
- Debug symbols included
- Best for profiling

---

## Clean Build

Remove build artifacts:

```bash
# Remove entire build directory
rm -rf build

# Or use CMake
cmake --build build --target clean
```

Then reconfigure and rebuild:

```bash
cmake -S . -B build
cmake --build build
```

---

## Advanced CMake Options

### Specify Install Prefix

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/ui_engine
cmake --build build --target install
```

### Enable Verbose Makefiles

```bash
cmake -S . -B build -DCMAKE_VERBOSE_MAKEFILE=ON
cmake --build build
```

### Cross-Compilation (Advanced)

Requires toolchain file. Example for ARM:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-arm.cmake
cmake --build build
```

---

## Build System Structure

```
UI_Engine/
├── CMakeLists.txt          # Root: sets up vendors, includes subdirs
│
├── Engine/
│   └── CMakeLists.txt      # Builds UICheckEngine static library
│
├── Editor/
│   └── CMakeLists.txt      # Builds UICheckEditor executable
│
└── vendor/
    ├── glfw/CMakeLists.txt   # GLFW build
    ├── glm/CMakeLists.txt    # GLM (header-only)
    └── entt/                 # EnTT (single header)
```

**Build Flow:**

1. Root CMake processes vendor libraries (GLAD, GLFW, ImGui)
2. Engine CMake compiles engine source to static lib
3. Editor CMake compiles editor, links engine + vendor libs
4. Final executable placed in `build/bin/`

---

## Continuous Integration

Example GitHub Actions workflow (`.github/workflows/build.yml`):

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
        
    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive
        
    - name: Configure CMake
      run: cmake -S . -B build
      
    - name: Build
      run: cmake --build build
      
    - name: Test
      run: ./build/bin/UICheckEditor --version
```

---

## Next Steps

Once built successfully:

1. Read [ARCHITECTURE.md](ARCHITECTURE.md) to understand the engine structure
2. Review [INPUT_SYSTEM.md](INPUT_SYSTEM.md) for camera controls
3. Explore [SCENE_SYSTEM.md](SCENE_SYSTEM.md) to learn about ECS
4. Check [RENDERING.md](RENDERING.md) for rendering pipeline details

---

## Getting Help

If you encounter issues not covered here:

1. Check the [GitHub Issues](repository-issues-url) for similar problems
2. Open a new issue with:
   - OS and compiler version
   - CMake version
   - Full error output
   - Steps to reproduce

Happy building! 🚀
