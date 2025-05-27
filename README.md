# GLVolumeRenderer

GLVolumeRenderer is a rendering tool for visualizing NxNxN 3D binary volume data.

## Features

* Raymarching rendering mode
* Point cloud rendering mode
* Cross-section view using Near-Far Clip
* Filtering by volume density

## Screenshots

![Demo1](/Document/Demo-1.gif)
![Demo2](/Document/Demo-2.png)

The void cube volume data was created using [SDF2Volume](https://github.com/regusan/SDF2Volume).

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
./build/volumen volume/voidcube-256.dat
```

## Usage

* Right-click drag: Rotate view
* Mouse wheel: Zoom in/out

## Third-Party Licenses

This project uses the following third-party libraries. Their licenses are as follows:

- **GLEW (OpenGL Extension Wrangler Library)**Licensed under the MIT License.[GLEW GitHub Repository](https://github.com/nigels-com/glew)
- **GLFW (Graphics Library Framework)**Licensed under the zlib/libpng License.[GLFW GitHub Repository](https://github.com/glfw/glfw)
- **GLM (OpenGL Mathematics)**Licensed under the MIT License.[GLM GitHub Repository](https://github.com/g-truc/glm)
- **IMGUI (Dear ImGui)**
  Licensed under the MIT License.
  [Dear ImGui GitHub Repository](https://github.com/ocornut/imgui)

[self link](https://github.com/regusan/GLVolumeRenderer)
