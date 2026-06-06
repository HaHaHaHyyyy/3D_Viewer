cmake_minimum_required(VERSION 3.10)
project(3D_Viewer)

# Переключаем на C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем пути к MSYS2 (mingw64)
set(MSYS2_ROOT "C:/msys64/mingw64")

# Подключаем заголовки
include_directories(${MSYS2_ROOT}/include)

# Указываем папку с библиотеками
link_directories(${MSYS2_ROOT}/lib)

# Определяем макрос для статической линковки freeglut
add_definitions(-DFREEGLUT_STATIC)

# Все исходники (main.cpp теперь)
set(SOURCES
        main.cpp
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
        stdc++      # стандартная библиотека C++
)

if(EXISTS "${MSYS2_ROOT}/bin/freeglut.dll")
    add_custom_command(TARGET viewer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MSYS2_ROOT}/bin/freeglut.dll"
            $<TARGET_FILE_DIR:viewer>)
endif()