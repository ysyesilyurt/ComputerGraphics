cmake_minimum_required(VERSION 3.15)
project(OpenGL)

set(CMAKE_CXX_STANDARD 11)

# The main executable
#add_executable(OpenGL src/flat/helper.h src/flat/helper.cpp src/flat/jconfig.h src/flat/hw3_flat.cpp)
add_executable(OpenGL src/sphere/helper.h src/sphere/helper.cpp src/sphere/jconfig.h src/sphere/hw3_sphere.cpp)
set_property(TARGET OpenGL PROPERTY CXX_STANDARD 11)
target_compile_options(OpenGL PRIVATE -Wall)

# glfw
add_subdirectory(lib/glfw EXCLUDE_FROM_ALL)
target_link_libraries(OpenGL PRIVATE glfw)

# glew
add_definitions(-DGLEW_STATIC)
add_subdirectory(lib/glew EXCLUDE_FROM_ALL)
target_link_libraries(OpenGL PRIVATE libglew_static)

# glm
add_subdirectory(lib/glm EXCLUDE_FROM_ALL)
target_link_libraries(OpenGL PRIVATE glm)

# jpeg
target_link_libraries(OpenGL PRIVATE jpeg)
