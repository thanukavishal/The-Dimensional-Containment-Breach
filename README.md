# The Dimensional Containment Breach

An interactive 3D Computer Graphics simulation built in C++ using **OpenGL**, **GLUT**, and **SOIL2**. The application features an ancient high-tech containment chamber securing an unstable multi-ring relic surrounded by structural pillars, custom procedural geometry, dynamic lighting, texture mapping, and a first-person drone camera with bounding-box collision detection.

---

## 🏛️ Features & Technical Overview

- **Modular Architecture**: 
  - `Lib` static library: Provides a procedural `Mesh` generator (textured boxes, cylinders, multi-ring tori, vertex/normal calculations) and a `MeshRenderer` for solid and wireframe rendering.
  - `App` executable: Manages the scene graph, input processing, dynamic lighting, texture binding, and collision detection.
- **Dynamic Relic Animation**:
  - Three concentric bronze rings rotating simultaneously along different 3D axes.
  - Periodic floating/levitation motion modeled with sinusoidal oscillation.
  - Central pulsating energy core with custom specular and emission material properties.
- **Lighting & Materials**:
  - Smooth Phong/Gouraud shading with positioned ambient, diffuse, and specular point light sources.
  - Multiple distinct material definitions (Bronze alloy, emissive energy core, matte stone textures).
- **First-Person Drone Exploration**:
  - Smooth camera translation and yaw rotation.
  - Vertical altitude control (elevation up/down).
  - Axis-Aligned Bounding Box (AABB) collision detection preventing camera penetration into walls, the dais, or pillars.
- **Texture Mapping**:
  - Tiled high-resolution texture mapping for floors, walls, and pillars using SOIL2.

---

## 🎮 Controls

| Key / Action | Function |
| :--- | :--- |
| <kbd>W</kbd> | Move Forward (in facing direction) |
| <kbd>S</kbd> | Move Backward |
| <kbd>A</kbd> | Strafe Left |
| <kbd>D</kbd> | Strafe Right |
| <kbd>←</kbd> (Left Arrow) | Rotate Camera Left (Yaw) |
| <kbd>→</kbd> (Right Arrow) | Rotate Camera Right (Yaw) |
| <kbd>↑</kbd> (Up Arrow) | Ascend (Increase altitude) |
| <kbd>↓</kbd> (Down Arrow) | Descend (Decrease altitude) |
| <kbd>ESC</kbd> | Exit Application |

---

## 📁 Project Structure

```
The-Dimensional-Containment-Breach/
├── .gitignore               # Comprehensive Git ignore rules for MSBuild/VS
├── README.md                # Project documentation and build guide
└── App/
    ├── App.slnx             # Visual Studio 2022 Solution file
    ├── App/                 # Application Entrypoint & Scene
    │   ├── App.vcxproj      # Application project file
    │   ├── Main.cpp         # Main scene rendering, camera, input & animation loop
    │   ├── dais.jpg         # Texture asset
    │   ├── floor.jpg        # Texture asset
    │   ├── pillar.jpg       # Texture asset
    │   ├── wall.jpg         # Texture asset
    │   └── glut32.dll       # GLUT runtime dynamic link library
    └── Lib/                 # Core Graphics Static Library
        ├── Lib.vcxproj      # Static library project file
        ├── Mesh.h / .cpp    # Procedural 3D mesh definitions (Boxes, Cylinders, Tori)
        ├── MeshRenderer.h   # Render pipeline abstractions (Solid & Wireframe)
        └── pch.h / .cpp     # Precompiled header files
```

---

## 🛠️ Build & Setup Instructions

### Prerequisites
- **Operating System**: Windows 10 / 11
- **IDE**: Visual Studio 2022 (v17.10 or newer recommended for `.slnx` solution support)
- **C++ Standard**: C++20
- **Target Architecture**: `x86` (Win32)
- **External Dependencies**:
  - [GLUT / FreeGLUT](https://www.opengl.org/resources/libraries/glut/) (header: `glut.h`, lib: `glut32.lib`, runtime: `glut32.dll`)
  - [SOIL2](https://github.com/SpartanJ/SOIL2) (header: `SOIL2.h`, lib: `soil2-debug.lib`)

### Building in Visual Studio
1. Clone the repository:
   ```bash
   git clone https://github.com/thanukavishal/The-Dimensional-Containment-Breach.git
   ```
2. Open `App/App.slnx` in Visual Studio 2022.
3. Configure target configuration to **Debug** and platform to **Win32 (x86)**.
4. Verify that the project's **Additional Include Directories** and **Additional Library Directories** point to your local installation paths for GLUT and SOIL2 (or set an environment variable).
5. Build the solution (`Ctrl + Shift + B`).
6. Run the project (`F5` or `Ctrl + F5`). Ensure `glut32.dll` and texture files (`*.jpg`) are located in the working directory alongside the executable.

---

## 📄 License
Academic & Educational Use - University of Peradeniya (UOP) Graphics LAB.
