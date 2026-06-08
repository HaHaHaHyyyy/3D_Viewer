#pragma once

#include <string>
#include <GL/gl.h>

struct SceneInfo
{
    // Полный путь к файлу сцены
    std::string sceneFile;

    // Имя сцены без расширения
    std::string name;

    // Путь к изображению-превью
    std::string previewFile;

    // OpenGL-текстура превью
    GLuint previewTexture = 0;

    // Дата создания (для отображения в лаунчере)
    std::string creationDate;

    // Количество объектов в сцене
    int objectCount = 0;
};