#pragma once
#include <string>

struct SceneInfo
{
    // Полный путь к файлу сцены
    std::string sceneFile;

    // Имя сцены без расширения
    std::string name;

    // Путь к изображению-превью (можно потом скриншот из 3D-Viewer)
    std::string previewFile;

    // Дата создания (пока "Unknown")
    std::string creationDate;

    // Количество объектов в сцене
    int objectCount = 0;
};