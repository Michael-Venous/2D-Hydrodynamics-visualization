cmake_minimum_required(VERSION 3.5)  # CMake version check
project(opengl_program)               # Create project
set(CMAKE_CXX_STANDARD 17)            # Enable c++17 standard

# Add GLAD
add_library(glad src/glad.c)
target_include_directories(glad PUBLIC include)

# Find the glfw package
find_package(glfw3 3.3 REQUIRED)

cmake_policy(SET CMP0072 NEW)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Gather all source files
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Add executables
add_executable(opengl_program ${SRC_FILES} main.cpp)


# Add the header files directory
target_include_directories(opengl_program PRIVATE "${CMAKE_SOURCE_DIR}/header")

# Link OpenGL
target_link_libraries(opengl_program PRIVATE glad OpenGL::GL glfw)

