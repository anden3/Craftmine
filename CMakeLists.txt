cmake_minimum_required(VERSION 3.3)
project(Test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/Users/mac/ClionProjects/C++/build/")

set(SOURCE_FILES main.cpp)
add_executable(Test ${SOURCE_FILES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L /usr/local/include -l SOIL -l glfw3 -l assimp -l noise -l noiseutils -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL")