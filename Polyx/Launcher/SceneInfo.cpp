#pragma once

#include <string>

struct SceneInfo
{
    // Полный путь к файлу сцены
    std::string sceneFile;

    // Имя сцены без расширения
    std::string name;

    // Путь к изображению-превью
    std::string previewFile;

    // Дата создания (для отображения в лаунчере)
    std::string creationDate;

    // Количество объектов в сцене
    int objectCount = 0;

};