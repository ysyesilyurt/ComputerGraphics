cmake_minimum_required(VERSION 3.15)
project(Rasterization)

set(CMAKE_CXX_STANDARD 11)

add_executable(Rasterization Main.cpp Camera.h Camera.cpp Color.h Color.cpp Helpers.h Helpers.cpp Matrix4.h Matrix4.cpp
        Model.h Model.cpp Rotation.h Rotation.cpp Scaling.h Scaling.cpp Scene.h Scene.cpp tinyxml2.h tinyxml2.cpp
        Translation.h Translation.cpp Triangle.h Triangle.cpp Vec3.h Vec3.cpp Vec4.h Vec4.cpp)