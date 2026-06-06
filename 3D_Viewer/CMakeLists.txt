cmake_minimum_required(VERSION 3.10)
project(3D_Viewer)

set(CMAKE_C_STANDARD 99)

# Указываем пути к MSYS2 (mingw64)
set(MSYS2_ROOT "C:/msys64/mingw64")

# Подключаем заголовки
include_directories(${MSYS2_ROOT}/include)

# Указываем папку с библиотеками
link_directories(${MSYS2_ROOT}/lib)

# Определяем макрос для статической линковки freeglut
add_definitions(-DFREEGLUT_STATIC)

# Ваши исходные файлы
set(SOURCES
        main.c
        mesh.c
        camera.c
        utils.c
        texture.c
        lights.c
        screenshot.c
        Lists.h
        Lists.c
)

add_executable(viewer ${SOURCES})

# Линкуем статическую библиотеку freeglut и системные библиотеки
target_link_libraries(viewer
        libfreeglut.a
        opengl32
        glu32
        winmm
        gdi32
)

# Копируем freeglut.dll (если нужна динамическая, но при статической линковке она не нужна)
# Однако для уверенности скопируем, если есть
if(EXISTS "${MSYS2_ROOT}/bin/freeglut.dll")
    add_custom_command(TARGET viewer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MSYS2_ROOT}/bin/freeglut.dll"
            $<TARGET_FILE_DIR:viewer>)
endif()