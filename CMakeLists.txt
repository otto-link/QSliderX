cmake_minimum_required(VERSION 3.10)
project(qsliderx-root VERSION 0.0.0)

option(QSLIDERX_ENABLE_TESTS "Enable QSliderX tests" ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # MSVC compiler options
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Od")
else()
  # GNU compiler options
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wno-dangling-reference \
      -fno-omit-frame-pointer \
      -Wpedantic -Wno-psabi -Wno-unused-parameter \
      -Wcast-align -Wcast-qual  \
      -Wnull-dereference -Wnon-virtual-dtor -Woverloaded-virtual \
      -Wzero-as-null-pointer-constant -Wduplicated-cond \
      -Wduplicated-branches -Wlogical-op -Wuseless-cast \
      -Wshadow -Wconversion -Wold-style-cast \
      ")
endif()

set(CMAKE_CXX_STANDARD 20)

# --- copy data
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/QSliderX/data/bg.png
     DESTINATION ${EXECUTABLE_OUTPUT_PATH}/.)

# ---  dependencies
find_package(GSL REQUIRED COMPONENTS gsl gslcblas)
find_package(spdlog REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# add_subdirectory(external)
add_subdirectory(QSliderX)

if(QSLIDERX_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
