# Vulkan Triangle Renderer

A basic real-time graphics application built with **C++, Vulkan, GLFW, and CMake**. This project is my introduction to low-level graphics programming and the foundation for developing a custom game engine.

## Overview

This project renders a triangle using the Vulkan graphics API. Unlike higher-level game engines, Vulkan requires explicit management of graphics resources and rendering operations, making it a valuable way to understand how modern graphics pipelines work.

The project serves as an early milestone in my journey toward building a full-stack game engine from the ground up.

## Technologies

- **C++23** — Core programming language
- **Vulkan** — Low-level graphics API
- **GLFW** — Window creation and input handling
- **CMake** — Build system

## Features

- Vulkan instance and rendering setup
- GLFW window creation
- Graphics pipeline initialization
- Triangle rendering
- Swapchain-based presentation
- CMake build configuration

*Features listed above should be adjusted to match the functionality currently implemented in the codebase.*

## Prerequisites

Before building the project, ensure you have:

- A C++ compiler with C++23 support
- CMake 3.25 or later
- Vulkan SDK
- GLFW

On macOS, Vulkan can run through **MoltenVK**, which translates Vulkan calls to Apple's Metal graphics API.

## Building and Running

Clone the repository:

```bash
git clone <your-repository-url>
cd <your-project-directory>
```

Configure the project:

```bash
cmake -S . -B build
```

Build the executable:

```bash
cmake --build build
```

Run the application from the project directory:

```bash
./build/MyGame
```

If your executable is generated in a different location, adjust the final command accordingly.

## What I'm Learning

Through this project, I'm developing an understanding of:

- Low-level graphics programming
- GPU resource management
- Graphics pipelines and command buffers
- Swapchains and frame presentation
- C++ application architecture
- Build systems and dependency management

## Project Goals

This triangle renderer is the starting point for a larger game-engine project. My long-term goal is to build a custom engine with real-time 3D rendering, an editor interface, camera controls, object manipulation, and eventually AI-powered development tools.

## Author

**Frank Saralde**

Computer Science student interested in game-engine development, graphics programming, and artificial intelligence.
