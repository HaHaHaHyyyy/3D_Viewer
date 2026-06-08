#include "SceneScanner.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

std::vector<SceneInfo> ScanScenes()
{
    std::vector<SceneInfo> result;

    fs::path basePath = fs::current_path();

    fs::path scenesPath =
        basePath.parent_path().parent_path() / "Scenes";

    std::cout << "Current path: " << basePath << std::endl;
    std::cout << "Scanning scenes in: " << fs::absolute(scenesPath) << std::endl;

    if (!fs::exists(scenesPath) || !fs::is_directory(scenesPath))
    {
        std::cout << "Scenes folder not found!" << std::endl;
        return result;
    }

    for (const auto& entry : fs::directory_iterator(scenesPath))
    {
        if (!entry.is_regular_file())
            continue;

        SceneInfo scene;
        scene.sceneFile = entry.path().string();
        scene.name = entry.path().stem().string();

        // ---------------------------
        // Поиск превью рядом со сценой
        // Поддерживаем .jpg и .png
        // ---------------------------
        std::vector<std::string> exts = {".jpg", ".png"};
        scene.previewFile.clear();

        for (auto& ext : exts)
        {
            fs::path p = entry.path().parent_path() / (scene.name + "_preview" + ext);
            if (fs::exists(p) && fs::is_regular_file(p))
            {
                scene.previewFile = p.string();
                std::cout << "Preview found: " << scene.previewFile << std::endl;
                break;
            }
        }

        if (scene.previewFile.empty())
        {
            std::cout << "Preview missing for " << scene.name << std::endl;
        }

        // Подсчёт объектов
        scene.objectCount = 0;
        std::ifstream file(scene.sceneFile);
        std::string line;
        while (std::getline(file, line))
        {
            if (line.rfind("obj ", 0) == 0)
                scene.objectCount++;
        }

        // Дата изменения
        try
        {
            auto ftime = fs::last_write_time(entry.path());
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
            std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
            scene.creationDate = std::ctime(&cftime);
            if (!scene.creationDate.empty() && scene.creationDate.back() == '\n')
                scene.creationDate.pop_back();
        }
        catch (...)
        {
            scene.creationDate = "Unknown";
        }

        result.push_back(scene);
    }

    std::cout << "Scenes found: " << result.size() << std::endl;
    return result;
}