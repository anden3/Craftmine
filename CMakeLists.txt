cmake_minimum_required(VERSION 3.5)
project(Craftmine)

set (SOURCE_PATH ${CMAKE_CURRENT_LIST_DIR}/Classes)
set (EXEC_LOCATION ${CMAKE_CURRENT_LIST_DIR}/Craftmine.app/Contents/MacOS/Craftmine)
set (EXEC_LIBS ${CMAKE_CURRENT_LIST_DIR}/Craftmine.app/Contents/Libraries)
set (EXEC_FWORKS ${CMAKE_CURRENT_LIST_DIR}/Craftmine.app/Contents/Frameworks)

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
    ${SOURCE_PATH}/Stack.cpp
    ${SOURCE_PATH}/System.cpp
    ${SOURCE_PATH}/UI.cpp
	${SOURCE_PATH}/Worlds.cpp
)

file (GLOB BLOCK_SCRIPTS "BlockScripts/*.cpp")
list (APPEND SOURCES ${BLOCK_SCRIPTS})

set (LIBRARIES optimized enet debug enet_debug FreeImage freetype glew glfw3 icuuc noise ogg SOIL vorbis vorbisfile)

if (WIN32)
	set (CMAKE_EXE_LINKER_FLAGS "/MACHINE:X64")
	set (CMAKE_MODULE_LINKER_FLAGS "/MACHINE:X64")
	set (CMAKE_SHARED_LINKER_FLAGS "/MACHINE:X64")
	set (CMAKE_STATIC_LINKER_FLAGS "/MACHINE:X64")

	list(APPEND LIBRARIES OpenAL32 opengl32 winmm ws2_32)
    set (LIBRARY_DIRS F:/lib F:/lib/boost)
    set (INCLUDE_DIRS ${SOURCE_PATH} F:/include F:/include/freetype2)

    set (CMAKE_GENERATOR "Visual Studio 14 2015 Win64" CACHE INTERNAL "" FORCE)

    add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/BlockScripts/Block_Scripts.cpp
        COMMAND powershell -executionpolicy bypass BlockScripts\\Load_Scripts.ps1
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )

elseif (APPLE)
    set (INCLUDE_DIRS ${SOURCE_PATH} /usr/local/include /usr/local/include/freetype2)
    set (LIBRARY_DIRS /usr/local/lib)

    find_library(OPENGL_LIB OpenGL)
    find_library(OPENAL_LIB OpenAL)
    find_library(BOOST_SYSTEM boost_system)
    find_library(BOOST_FILESYSTEM boost_filesystem)

    list (APPEND LIBRARIES
        ${OPENGL_LIB} ${OPENAL_LIB}
        ${BOOST_SYSTEM} ${BOOST_FILESYSTEM}
    )

    execute_process(
        COMMAND Load_Scripts.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/BlockScripts
    )
endif()

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_LIST_DIR})

include_directories(${INCLUDE_DIRS})
link_directories(${LIBRARY_DIRS})
add_executable(Craftmine ${SOURCES} ${CMAKE_CURRENT_LIST_DIR}/BlockScripts/Block_Scripts.cpp)

set_target_properties(Craftmine PROPERTIES LINKER_LANGUAGE CXX)

if (APPLE)
    set (FOLDERS_TO_COPY
        BlockData
        Fonts
        Images
        Shaders
        Sounds
        Structures
        Worlds
    )

    target_compile_options(Craftmine PUBLIC
        -arch x86_64 -std=gnu++14 -F Build -MMD -MT dependencies -O0 -g -Wall
        -Wno-missing-braces
    )

    set (LIBS enet freeimage freetype glew glfw3 icuuc noise ogg SOIL vorbis vorbisfile)

    set (LIB_PATHS "")

    foreach (f ${LIBS})
        list(APPEND LIB_PATHS "/usr/local/lib/lib${f}.dylib")
    endforeach(f)

    list (APPEND LIB_PATHS
        ${BOOST_SYSTEM} ${BOOST_FILESYSTEM}
    )

    set (FRAMEWORKS ${OPENGL_LIB} ${OPENAL_LIB})

    add_custom_command (TARGET Craftmine PRE_LINK
        COMMAND mkdir -p Craftmine.app/Contents/MacOS
        COMMAND mkdir -p ${EXEC_LIBS}
        COMMAND mkdir -p Craftmine.app/Contents/Resources
        COMMAND mkdir -p Craftmine.app/Contents/Frameworks

        COMMAND rsync -La --chmod=+rw ${LIB_PATHS} ${EXEC_LIBS}
        COMMAND rsync -a ${FRAMEWORKS} Craftmine.app/Contents/Frameworks
        COMMAND rsync -a ${FOLDERS_TO_COPY} Craftmine.app/Contents/Resources

        COMMAND install_name_tool -id @executable_path/../Libraries/libenet.dylib ${EXEC_LIBS}/libenet.dylib
        COMMAND install_name_tool -id @executable_path/../Libraries/libFreeImage.dylib ${EXEC_LIBS}/libFreeImage.dylib
        COMMAND install_name_tool -id @executable_path/../Libraries/libfreetype.dylib ${EXEC_LIBS}/libfreetype.dylib
        COMMAND install_name_tool -id @executable_path/../Libraries/libglew.dylib ${EXEC_LIBS}/libglew.dylib
        COMMAND install_name_tool -id @executable_path/../Libraries/libglfw3.dylib ${EXEC_LIBS}/libglfw3.dylib
        COMMAND install_name_tool -id @executable_path/../Libraries/libicuuc.dylib ${EXEC_LIBS}/libicuuc.dylib
        COMMAND install_name_tool -id @executable_path/../Libraries/libnoise.dylib ${EXEC_LIBS}/libnoise.dylib
        COMMAND install_name_tool -id @executable_path/../Libraries/libogg.dylib ${EXEC_LIBS}/libogg.dylib
        COMMAND install_name_tool -id @executable_path/../Libraries/libSOIL.dylib ${EXEC_LIBS}/libSOIL.dylib
        COMMAND install_name_tool -id @executable_path/../Libraries/libvorbis.dylib ${EXEC_LIBS}/libvorbis.dylib
        COMMAND install_name_tool -id @executable_path/../Libraries/libvorbisfile.dylib ${EXEC_LIBS}/libvorbisfile.dylib

        COMMAND install_name_tool -id @executable_path/../Libraries/libboost_system.dylib ${EXEC_LIBS}/libboost_system.dylib
        COMMAND install_name_tool -id @executable_path/../Libraries/libboost_filesystem.dylib ${EXEC_LIBS}/libboost_filesystem.dylib
        COMMAND install_name_tool -id @executable_path/../Frameworks/OpenAL.framework/Versions/Current/OpenAL ${EXEC_FWORKS}/OpenAL.framework/Versions/Current/OpenAL
        COMMAND install_name_tool -id @executable_path/../Frameworks/OpenGL.framework/Versions/Current/OpenGL ${EXEC_FWORKS}/OpenGL.framework/Versions/Current/OpenGL

        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )

    add_custom_command(TARGET Craftmine POST_BUILD
        COMMAND mv Craftmine ${EXEC_LOCATION}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )
endif()

target_link_libraries(Craftmine ${LIBRARIES})
