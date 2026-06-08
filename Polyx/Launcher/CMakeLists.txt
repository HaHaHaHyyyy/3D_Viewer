cmake_minimum_required(VERSION 3.20)

project(Launcher)

# Стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Пути к MSYS2 (заголовки и библиотеки)
set(MSYS2_ROOT "C:/msys64/mingw64")
include_directories(${MSYS2_ROOT}/include)
link_directories(${MSYS2_ROOT}/lib)

# Все исходники Launcher
set(LAUNCHER_SOURCES
        main.cpp
        SceneScanner.cpp
        SceneInfo.cpp
        LauncherUtils.cpp
        LauncherUtils.h

        # ImGui из 3D_Viewer
        ../3D_Viewer/imgui.cpp
        ../3D_Viewer/imgui_draw.cpp
        ../3D_Viewer/imgui_tables.cpp
        ../3D_Viewer/imgui_widgets.cpp
        ../3D_Viewer/imgui_impl_glut.cpp
        ../3D_Viewer/imgui_impl_opengl3.cpp
)

add_executable(Launcher ${LAUNCHER_SOURCES})

target_include_directories(Launcher PRIVATE
        .
        ../3D_Viewer
        C:/msys64/mingw64/include
)

target_link_libraries(Launcher
        freeglut
        opengl32
        glu32
        winmm
        gdi32
        comdlg32
)

# Опционально: скопировать DLL FreeGLUT рядом с exe после сборки
if(EXISTS "${MSYS2_ROOT}/bin/freeglut.dll")
    add_custom_command(TARGET Launcher POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MSYS2_ROOT}/bin/freeglut.dll"
            $<TARGET_FILE_DIR:Launcher>
    )
endif()