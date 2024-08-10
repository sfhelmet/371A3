cmake_minimum_required(VERSION 3.10)
project(OpenGLIntro)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(/opt/homebrew/include)

# Link directories
link_directories(/opt/homebrew/lib)

# Add executable
add_executable(OpenGLIntro main.cpp)

# Link libraries
if (APPLE)
    find_library(OpenGL_LIBRARY OpenGL)
    target_link_libraries(OpenGLIntro glfw glew ${OpenGL_LIBRARY})
else()
    target_link_libraries(OpenGLIntro glfw glew OpenGL)
endif()