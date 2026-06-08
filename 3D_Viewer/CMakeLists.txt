cmake_minimum_required(VERSION 3.10)
project(3D_Viewer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MSYS2_ROOT "C:/msys64/mingw64")

include_directories(${MSYS2_ROOT}/include)
link_directories(${MSYS2_ROOT}/lib)

add_definitions(-DFREEGLUT_STATIC)

set(SOURCES
        main.cpp
        scene.cpp
        texture_manager.cpp
        undo.cpp
        ui.cpp
        input.cpp
        mesh.cpp
        camera.cpp
        utils.cpp
        texture.cpp
        lights.cpp
        screenshot.cpp
        Lists.cpp
        imgui.cpp
        imgui_draw.cpp
        imgui_tables.cpp
        imgui_widgets.cpp
        imgui_impl_glut.cpp
        imgui_impl_opengl3.cpp
)

add_executable(viewer ${SOURCES})

target_link_libraries(viewer
        libfreeglut.a
        opengl32
        glu32
        winmm
        gdi32
        stdc++
)

if(EXISTS "${MSYS2_ROOT}/bin/freeglut.dll")
    add_custom_command(TARGET viewer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MSYS2_ROOT}/bin/freeglut.dll"
            $<TARGET_FILE_DIR:viewer>)
endif()