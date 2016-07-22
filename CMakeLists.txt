cmake_minimum_required(VERSION 3.5)
project(Craftmine)

set (SOURCE_PATH ${CMAKE_CURRENT_LIST_DIR}/Classes)

set (SOURCES
    ${SOURCE_PATH}/Blocks.cpp
    ${SOURCE_PATH}/Buffer.cpp
    ${SOURCE_PATH}/Camera.cpp
    ${SOURCE_PATH}/Chat.cpp
    ${SOURCE_PATH}/Chunk.cpp
    ${SOURCE_PATH}/Entity.cpp
    ${SOURCE_PATH}/Interface.cpp
    ${SOURCE_PATH}/Inventory.cpp
    ${SOURCE_PATH}/main.cpp
    ${SOURCE_PATH}/Network.cpp
    ${SOURCE_PATH}/Player.cpp
    ${SOURCE_PATH}/Shader.cpp
    ${SOURCE_PATH}/Sound.cpp
    ${SOURCE_PATH}/System.cpp
    ${SOURCE_PATH}/Timer.cpp
    ${SOURCE_PATH}/UI.cpp
	${SOURCE_PATH}/Worlds.cpp
)

set (LIBRARIES enet freeimage freetype glew glfw3 icuuc noise OpenAL32 opengl32 SOIL vorbisfile)

if (WIN32)
	set (CMAKE_EXE_LINKER_FLAGS /MACHINE:X64)
	set (CMAKE_MODULE_LINKER_FLAGS /MACHINE:X64)
	set (CMAKE_SHARED_LINKER_FLAGS /MACHINE:X64)
	set (CMAKE_STATIC_LINKER_FLAGS /MACHINE:X64)

	list(APPEND LIBRARIES winmm ws2_32)
    set (INCLUDE_DIRS ${SOURCE_PATH} F:/include F:/include/freetype2)
    set (LIBRARY_DIRS F:/lib F:/lib/boost)

    set (CMAKE_GENERATOR "Visual Studio 14 2015 Win64" CACHE INTERNAL "" FORCE)

elseif (APPLE)
    set (INCLUDE_DIRS ${SOURCE_PATH} /usr/local/include /usr/local/include/freetype2)
    set (LIBRARY_DIRS /usr/local/lib /usr/local/boost)
endif()

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_LIST_DIR})

include_directories(${INCLUDE_DIRS})
link_directories(${LIBRARY_DIRS})
add_executable(Craftmine ${SOURCES})

set_target_properties(Craftmine PROPERTIES
    LINKER_LANGUAGE CXX
)

if (APPLE)
    target_compile_options(Craftmine PUBLIC -arch x86_64 -std=gnu++14 -F Build -MMD -MT dependencies -O0 -g)
endif()

target_link_libraries(Craftmine ${LIBRARIES})