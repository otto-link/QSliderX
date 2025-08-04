# QSliderX

QSliderX is a C++ library that extends Qt6 with custom slider widgets inspired by Blender's UX. It provides sliders for numeric and range values, with optional bounds and dynamic visual feedback such as histograms. This library is currently used in the Hesiod GUI, a node-based system for heightmap generation.

https://github.com/user-attachments/assets/803c6243-5c5e-4665-a17e-ed0f7b118827

https://github.com/user-attachments/assets/132d9af3-6fab-48e6-ad54-c21d7983145d

## Features

- **`SliderInt`**  
  An integer slider with optional lower and upper bounds. Includes a compact "Blender-style" value bar and supports precise dragging and direct input.

- **`SliderFloat`**  
  A floating-point slider, also supporting optional bounds and precision control.

- **`SliderRange`**  
  A double-ended range slider for selecting a min/max interval. Can optionally display a live histogram behind the slider for feedback.

- **`CanvasPoints`**

- **`CanvasField`**

## Build Instructions (for Developers)

To build the `QSliderX` library locally, follow the steps below.

### Clone the repository

```bash
git clone https://github.com/otto-link/QSliderX.git
```

### 2. Configure and build with CMake

```bash
cd QSliderX
mkdir build && cd build
cmake ..
make
```

### CMake Requirements

| Tool   | Minimum Version                             |
| ------ | ------------------------------------------- |
| CMake  | 3.16                                        |
| Qt     | 6.x (Core + Widgets)                        |
| C++    | 20                                          |
| spdlog | Installed and available as `spdlog::spdlog` |

You can install dependencies using your package manager, e.g., on Ubuntu:

```bash
sudo apt install qt6-base-dev libspdlog-dev
```

### Run a sample

The project includes a test executable:

```bash
./test_qsx
```

## Integrating QSliderX with CMake

`QSliderX` is a modern C++ library for Qt-based slider components. This guide shows how to integrate it into your own CMake project.

### Project Structure (Assumption)

Your library lives in a subdirectory, e.g.:

```
my-app/
├── CMakeLists.txt
├── external/
│   └── QSliderX/    # Contains this library
```

### Step 1: Add QSliderX to Your Project

In your **top-level `CMakeLists.txt`**:

```cmake
# Add the QSliderX directory
add_subdirectory(external/QSliderX)

# Link to your target
target_link_libraries(my_app PRIVATE qsliderx)
```

### Optional: Use as External Project

If you're not vendoring the source, you can also use **FetchContent**:

```cmake
include(FetchContent)

FetchContent_Declare(
    QSliderX
    GIT_REPOSITORY https://github.com/otto-link/QSliderX.git
    GIT_TAG        main
)

FetchContent_MakeAvailable(QSliderX)

target_link_libraries(my_app PRIVATE qsliderx)
```

> Don't forget to add `FetchContent_Declare` before `project()` if you're using CMake 3.14+ and need reproducible builds.

### QSliderX Requirements

* **Qt 6.x** (`Core`, `Widgets`)
* **C++20** compiler
* `spdlog` (must be available as `spdlog::spdlog` target)

Make sure the following are available in your project:

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(spdlog REQUIRED)
```

### Example

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_app LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(spdlog REQUIRED)

add_executable(my_app main.cpp)

add_subdirectory(external/QSliderX)

target_link_libraries(my_app PRIVATE qsliderx)
```

## License

This project is licensed under the GPL-3.0 license.
