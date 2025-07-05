# ImGradient

**ImGradient** is a lightweight and extensible gradient editor widget for [Dear ImGui](https://github.com/ocornut/imgui). It allows users to visually create and edit color gradients through an intuitive UI.

<p align="center">
  <img src="https://github.com/user-attachments/assets/98832659-cd19-4538-bb12-a93795e5d26b" alt="Gradient Picker Preview" />
</p>

## Features

- Interactive gradient marker editing
- Alpha channel support (optional)
- Read-only mode
- Easy integration with any ImGui-based application
- Configurable behavior via flags

## Demo

An example usage is provided in the `example/` folder. It demonstrates setting up ImGui with GLFW and OpenGL, and rendering a gradient picker.

## Getting Started

### Requirements

- C++11 or higher
- [Dear ImGui](https://github.com/ocornut/imgui)
- A backend for ImGui (e.g., GLFW + OpenGL)

### Integration

Copy or include the following files in your project:

- `imgradient.h`
- `imgradient.cpp`
- `imgradient_internal.h`

Alternatively, add ImGradient as a subdirectory and use the provided CMake setup (see below).

Example:

```cpp
ImGradient::AddInitialMarker(ImVec4(1, 0, 0, 1), 0.0f);
ImGradient::AddInitialMarker(ImVec4(0, 1, 0, 1), 0.5f);
ImGradient::AddInitialMarker(ImVec4(0, 0, 1, 1), 1.0f);
ImGradient::GradientPicker("Gradient");
```

## Using CMake in Your Project

To integrate **ImGradient** into your own CMake-based project:

### 1. Add Dear ImGui to Your Project

Define an ImGui target in your `CMakeLists.txt` (example):

```cmake
add_library(imgui ...)

target_include_directories(imgui PUBLIC
    path/to/imgui
    path/to/backends
)
```

### 2. Add ImGradient

```cmake
set(IMGRADIENT_IMGUI_TARGET_NAME imgui)  # Use the name of your ImGui target
add_subdirectory(path/to/imgradient)
```

### 3. Link ImGradient to Your App

```cmake
target_link_libraries(your_app PRIVATE imgradient)
```

