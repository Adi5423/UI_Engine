# UI Engine

A modern 3D game engine built with OpenGL, featuring an Entity-Component-System (ECS) architecture powered by EnTT and an ImGui-based editor interface.

---

## ✨ Features

- **Modern OpenGL 4.6 Core Profile** rendering
- **Entity-Component-System** architecture using [EnTT](https://github.com/skypjack/entt)
- **ImGui-based Editor** with dockable viewports and scene hierarchy
- **Viewport-Scoped Input System** for precise camera control
- **3D Mesh Rendering** with primitive generation (cube, triangle, circle)
- **Editor Camera** with fly-through controls (WASD + right-mouse look)
- **Framebuffer Rendering** to ImGui viewport textures
- **Component-based Scene Management** with transform hierarchy
- **Cross-platform CMake Build System**

---

## 🚀 Quick Start

### Prerequisites

- CMake 3.20+
- C++20 compiler (MinGW-w64 on Windows, GCC/Clang on Linux)
- Git (for cloning submodules)

### Build Instructions

```bash
# Clone the repository
git clone <repository-url>
cd UI_Engine

# Configure CMake
cmake -S . -B build

# Build the project
cmake --build build

# Run the editor
./build/bin/UICheckEditor
```

For detailed build instructions and troubleshooting, see [docs/BUILD.md](docs/BUILD.md).

---

## 📂 Project Structure

```
UI_Engine/
├── Engine/                 # Core engine library
│   ├── Core/              # Application, Window, Input systems
│   ├── Rendering/         # Renderer, Shaders, Buffers, Camera, Mesh
│   └── Scene/             # ECS Scene, Entity, Components
│
├── Editor/                # ImGui-based editor application
│   ├── Core/              # EditorLayer, ImGuiLayer
│   └── main.cpp           # Editor entry point
│
├── vendor/                # Third-party dependencies
│   ├── glfw/             # Window and input (included)
│   ├── glad/             # OpenGL loader (included)
│   ├── glm/              # Math library (included)
│   ├── imgui/            # UI framework (included)
│   ├── entt/             # ECS library (included)
│   └── stb/              # Image utilities (included)
│
├── docs/                 # Documentation
└── CMakeLists.txt        # Root build configuration
```

---

## 🎮 Editor Controls

### Camera Movement (Viewport-Scoped)

Camera controls only activate when:
1. Mouse cursor is **inside the viewport** window
2. **Right mouse button** is held down

**Movement:**
- `W` / `S` - Move forward/backward
- `A` / `D` - Strafe left/right
- `Q` / `E` - Move down/up
- **Mouse movement** - Look around (while RMB held)

The cursor automatically locks when camera is active and releases when you release the right mouse button.

---

## 📚 Documentation

### Architecture & Systems

- **[Architecture Overview](docs/ARCHITECTURE.md)** - High-level system design and module interactions
- **[Rendering System](docs/RENDERING.md)** - Mesh system, shaders, framebuffers, and rendering pipeline
- **[Scene & ECS](docs/SCENE_SYSTEM.md)** - Entity-Component-System using EnTT
- **[Input System](docs/INPUT_SYSTEM.md)** - Viewport-scoped input and camera controls

### Development

- **[Build Guide](docs/BUILD.md)** - Detailed build instructions and troubleshooting

---

## 🔧 Technology Stack

| Component | Library | Version |
|-----------|---------|---------|
| Graphics API | OpenGL | 4.6 Core |
| Window/Input | GLFW | 3.x |
| GL Loader | GLAD | 4.6 |
| Math | GLM | Latest |
| UI Framework | ImGui | Docking branch |
| ECS | EnTT | Single-header |
| Build System | CMake | 3.20+ |

---

## 🏗️ Current State

This engine is under active development. Current features include:

✅ Basic rendering pipeline with mesh abstraction  
✅ ECS-based scene management  
✅ ImGui editor with viewport rendering  
✅ Viewport-scoped camera input system  
✅ Transform component with matrix calculations  
✅ Primitive mesh generation (cube, triangle, circle)  

### Planned Features

- [ ] Material system with PBR shading
- [ ] Scene serialization (save/load)
- [ ] Asset management system
- [ ] Lighting system (point, directional, spot)
- [ ] Physics integration
- [ ] Audio system
- [ ] Entity hierarchy (parent-child transforms)

---

## 👨‍💻 Developer

**Aditya Tiwari**

- 💼 LinkedIn: [Aditya Tiwari](https://www.linkedin.com/in/adii5423/)
- 🐱 GitHub: [adi5423](https://github.com/adi5423)
- 📧 Email: adii54ti23@gmail.com
- 🐦 Twitter: [@Adii5423](https://twitter.com/Adii5423)
- 📸 Instagram: [@Adii5423.exe](https://instagram.com/Adii5423)

---

## 🤝 Contributing

Contributions are welcome! Feel free to:
- Report bugs by opening an issue
- Suggest new features
- Submit pull requests with improvements

Please ensure your code follows the existing style and includes appropriate documentation.

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE.txt) file for details.

---

## 🙏 Acknowledgments

Special thanks to the developers of:
- [GLFW](https://www.glfw.org/) - Window and input handling
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode GUI
- [EnTT](https://github.com/skypjack/entt) - Fast and reliable ECS library
- [GLM](https://github.com/g-truc/glm) - OpenGL Mathematics
- [GLAD](https://glad.dav1d.de/) - OpenGL loader generator
