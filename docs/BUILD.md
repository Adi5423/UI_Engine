# Build Guide

Complete instructions for building and running the UI Engine from source.

---

## Prerequisites

### Required Tools

| Tool             | Minimum Version | Purpose                         |
| ---------------- | --------------- | ------------------------------- |
| **CMake**        | 3.20+           | Build system generator          |
| **C++ Compiler** | C++20 support   | Code compilation                |
| **Git**          | Any recent      | Clone repository and submodules |

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

> **Note:** For loading all dependencies in `vendor/` as Git submodules or vendored directly.

> Run "git submodule update --init --recursive"

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

- `UICheckEngine` - Shared library (.dll) containing engine systems
- `imgui` - Shared library (.dll) for ImGui
- `glad` - Shared library (.dll) for OpenGL loader
- `glfw` - Shared library (.dll) for windowing
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
│   └── CMakeLists.txt      # Builds UICheckEngine shared library
│
├── Editor/
│   └── CMakeLists.txt      # Builds UICheckEditor executable
│
└── vendor/
    ├── glfw/CMakeLists.txt   # GLFW build (Shared)
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

````yaml
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

---

## Professional Release Packaging (Windows)

To package the engine for distribution so it runs on any Windows machine without requiring MinGW or development tools installed.

### 1. The Release Workflow

Run these commands in order to create a clean, portable `ReleaseBuild` folder:

```powershell
# 1. Clear old build data (Optional but recommended)
Remove-Item -Recurse -Force build
Remove-Item -Recurse -Force ReleaseBuild

# 2. Configure for Release with MinGW
cmake -G "MinGW Makefiles" -B build -DCMAKE_BUILD_TYPE=Release

# 3. Build the binaries
cmake --build build --config Release

# 4. Install/Package to the ReleaseBuild directory
cmake --install build --prefix ReleaseBuild --config Release
````

### 2. What's in the Box?

The `ReleaseBuild` folder is designed to be "Direct Play" ready:

- **Modular DLLs**: `UICheckEngine.dll`, `glad.dll`, `imgui.dll`, etc., are in the root next to the `.exe`.
- **Runtime Bundling**: The system automatically pulls `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, and `libwinpthread-1.dll` from your compiler path and includes them.
- **Assets**: All files in `Drop_at_EXE/` (like `settings/` and `imgui.ini`) are automatically copied.

### 3. Verification & Testing

We use these commands to ensure the build is perfectly portable before zipping:

#### Verify File Structure

```powershell
tree /F ReleaseBuild
```

_Checks if all DLLs, EXEs, and settings folders are in their correct relative positions._

#### Verify Dynamic Linking (Important)

We use `objdump` (included with MinGW) to check if the executable is correctly looking for the DLLs in its own folder rather than static linking or external system paths.

```powershell
# Check dependencies of the Editor
objdump -p ReleaseBuild/UICheckEditor.exe | Select-String "DLL Name"

# Check dependencies of the Engine DLL
objdump -p ReleaseBuild/UICheckEngine.dll | Select-String "DLL Name"
```

_If you see `UICheckEngine.dll`, `glad.dll`, and `libstdc++-6.dll` in the output, the dynamic linking is working perfectly._

#### Portability Test

1. Zip the `ReleaseBuild` folder.
2. Send to a machine without MinGW installed.
3. Run `UICheckEditor.exe`. It should start immediately.

---

## Cross-Platform Distribution (Direct Play)

To create a "Direct Play" release on Linux and macOS, follow these platform-specific steps. The engine is configured to launch **Maximized** on all devices by default.

### Universal Release Workflow

On all systems, the standard command sequence is:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --prefix ReleaseBuild --config Release
```

### 1. Windows (MinGW)

The system automatically bundles:

- `UICheckEngine.dll`, `glad.dll`, `imgui.dll`, `ImGuizmo.dll`, `glfw3.dll`
- **Compiler Runtimes**: `libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`
- **Assets**: `settings/`, `imgui.ini`

### 2. Linux (Ubuntu/Debian/Fedora)

On Linux, we use `RPATH` to ensure the executable finds the `.so` files in its own directory.

**Build Requirements:**

```bash
sudo apt install build-essential cmake libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

**Packaging Note:**

- The `ReleaseBuild` will contain `UICheckEditor` (executable) and `libUICheckEngine.so`, `libimgui.so`, etc.
- **Portability**: Most Linux systems have `libstdc++` installed. However, for a truly standalone zip, you should bundle the shared objects.
- **Running**: The user can simply unzip and run `./UICheckEditor`.

### 3. macOS (Intel/Apple Silicon)

On macOS, we handle two key things: **Retina Scaling** and **Shared Library paths**.

**Build Requirements:**

- Xcode Command Line Tools (`xcode-select --install`)
- CMake via Brew (`brew install cmake`)

**Bundling System:**

- The engine uses `#ifdef __APPLE__` to enable `GLFW_OPENGL_FORWARD_COMPAT` and `GLFW_COCOA_RETINA_FRAMEBUFFER`.
- This ensures the UI looks crisp on Retina displays and uses a modern OpenGL 4.1+ core profile (Apple's limit).
- The `ReleaseBuild` folder will work as a portable directory on other Macs of the same architecture (Intel vs M1/M2/M3).

---

## Environment Check (Pre-Zip)

Before zipping your `ReleaseBuild` for other users, check these:

1. **Maximized Window**: The engine will automatically attempt to fill the screen on launch.
2. **Relative Assets**: Ensure `settings/theme/params.json` exists in the `ReleaseBuild` folder.
3. **Clean Build**: Always delete the `build/` and `ReleaseBuild/` folders before a final production build to avoid stale debug files.

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

Happy building!
```
