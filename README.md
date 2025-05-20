# GLVolumeRenderer

GLVolumeRenderer is a rendering tool for visualizing NxNxN 3D binary volume data.


## Features
*   **3D Volume Rendering**: Displays NxNxN format binary volume data in 3D.

## Screenshots
![Demo1](/Document/Demo-1.gif)
![Demo2](/Document/Demo-2.png)


## Dependencies
* **OpenGL**
* **GLEW (OpenGL Extension Wrangler Library)** - MIT License  
* **GLFW (Graphics Library Framework)** - zlib/libpng License  
* **GLM (OpenGL Mathematics)** - MIT License  
* **IMGUI (Included)** - MIT License  


## Installation and Execution Example

### 1. Clone the repository

```bash
git clone https://github.com/regusan/GLVolumeRenderer.git
cd GLVolumeRenderer
```

### 2. Preparing Dependencies

```bash
sudo apt-get update
sudo apt-get install libglew-dev libglfw3-dev libglm-dev
```

### 3. Build
```bash
cmake -S . -B build
cmake --build build -j
```
### 4. Run


```bash
./build/volumen
```
## Usage
* Right-click drag: Rotate view
* Mouse wheel: Zoom in/out

## Third-Party Licenses

This project uses the following third-party libraries. Their licenses are as follows:

- **GLEW (OpenGL Extension Wrangler Library)**  
  Licensed under the MIT License.  
  [GLEW GitHub Repository](https://github.com/nigels-com/glew)  

- **GLFW (Graphics Library Framework)**  
  Licensed under the zlib/libpng License.  
  [GLFW GitHub Repository](https://github.com/glfw/glfw)  

- **GLM (OpenGL Mathematics)**  
  Licensed under the MIT License.  
  [GLM GitHub Repository](https://github.com/g-truc/glm)  

- **IMGUI (Dear ImGui)**  
  Licensed under the MIT License.  
  [Dear ImGui GitHub Repository](https://github.com/ocornut/imgui)  
