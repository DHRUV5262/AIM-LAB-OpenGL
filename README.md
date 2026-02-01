# OpenGL 3D Rendering Demo: Skybox & Lighting

A high-performance 3D graphics simulation implemented in **C++** and **OpenGL**, demonstrating advanced rendering techniques including environment mapping, real-time lighting interaction, and transparency blending.

<img width="797" height="632" alt="image" src="https://github.com/user-attachments/assets/28e4dc7c-717b-4071-b73f-3103d9a5279e" />

## 🚀 Features

### 1. Immersive Skybox
- Implemented a **Cube Map** rendering technique using 6-sided textures to create a seamless 3D environment background.
- Optimized depth testing (using `GL_LEQUAL`) to ensure the skybox is rendered behind all other geometry efficiently.

### 2. Advanced Lighting Model
- Features a **Phong Lighting Model** (Ambient, Diffuse, and Specular components) that interacts dynamically with object surfaces.
- Light sources accurately reflect off spherical geometry, creating realistic highlights and shading based on the camera position.

### 3. Transparency & Blending
- Implemented **Alpha Blending** (`glBlendFunc`) to render transparent spheres.
- Correctly handles draw order to ensure transparent objects blend seamlessly with the opaque background and skybox.

### 4. Geometry & Interaction
- Procedural generation of spherical meshes with customizable resolution (stacks/slices).
- Scene composition allows for testing depth perception and lighting interactions between opaque and transparent objects.

## 🛠️ Tech Stack

- **Language:** C++
- **Graphics API:** OpenGL (Core Profile)
- **Libraries:**
  - **GLFW**: Window management and input handling.
  - **GLAD/GLEW**: OpenGL function loading.
  - **GLM**: Mathematics library for vector and matrix operations (transformations, camera, projections).
  - **stb_image**: Image loading for textures and cubemaps.

## 🎮 Controls

| Key / Input | Action |
| :--- | :--- |
| **W, A, S, D** | Move Camera (Forward, Left, Backward, Right) |
| **Mouse** | Look around (Pitch/Yaw) |
| **Scroll** | Zoom (FOV adjustment) |
| **Esc** | Close Application |

## 🔧 Setup & Build

1. **Clone the Repository**
   ```bash
   git clone https://github.com/DHRUV5262/Your-Repo-Name.git
   cd Your-Repo-Name
