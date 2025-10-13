# 2D SPH Fluid Simulation

A real-time 2D Smoothed Particle Hydrodynamics (SPH) fluid simulation implemented in C++ with OpenGL for visualization.

![Simulation Preview](https://github.com/user-attachments/assets/ac03922a-1967-4c6e-b1be-ffaa5d3bc5bd)

## Features

- **SPH Physics**: Pressure-based particle interactions using Poly6 and Spiky kernel functions
- **Verlet Integration**: Stable time-stepping for smooth particle motion
- **Interactive Forces**: Mouse-driven attraction and repulsion forces
- **Visual Feedback**: Color-coded particles based on velocity (blue = slow, red = fast)
- **Boundary Handling**: Soft boundary collisions with damping

## Technical Details

- **Language**: C++17
- **Graphics**: OpenGL 3.3 Core Profile
- **Libraries**: GLFW, GLAD, GLM
- **Build System**: CMake

## Building

### Prerequisites
- CMake 3.5+
- C++17 compatible compiler
- GLFW3
- OpenGL 3.3+ support

### Linux/macOS
```bash
cmake --preset default
cmake --build build/default
./build/default/opengl_program
```

### Windows
```bash
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
build\Release\opengl_program.exe
```

## Controls

- **Left Click**: Attract particles to cursor
- **Right Click**: Repel particles from cursor


## Inspiration

Based on concepts from [Sebastian Lague's Coding Adventure: Simulating Fluids](https://www.youtube.com/watch?v=rSKMYc1CQHE)

## License

This project is open source and available for educational purposes.
