#include "SceneScanner.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

std::vector<SceneInfo> ScanScenes()
{
    std::vector<SceneInfo> result;

#ifdef _WIN32
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    fs::path appDir = fs::path(exePath).parent_path();
#else
    fs::path appDir = fs::current_path();
#endif

    fs::path scenesPath = appDir / "Scenes";

    std::cout << "Current exe dir: " << appDir << std::endl;
    std::cout << "Scanning scenes in: " << scenesPath << std::endl;

    if (!fs::exists(scenesPath) || !fs::is_directory(scenesPath))
    {
        std::cout << "Scenes folder not found!" << std::endl;
        return result;
    }

    for (const auto& entry : fs::directory_iterator(scenesPath))
    {
        if (!entry.is_regular_file()) continue;

        std::string ext = entry.path().extension().string();
        if (ext != ".txt" && ext != ".scene") continue;

        SceneInfo scene;
        scene.sceneFile = entry.path().string();
        scene.name = entry.path().stem().string();

        scene.previewFile.clear();
        fs::path jpgPreview = scenesPath / (scene.name + "_preview.jpg");
        fs::path pngPreview = scenesPath / (scene.name + "_preview.png");

        if (fs::exists(jpgPreview))
            scene.previewFile = jpgPreview.string();
        else if (fs::exists(pngPreview))
            scene.previewFile = pngPreview.string();

        if (!scene.previewFile.empty())
            std::cout << "Preview found: " << scene.previewFile << std::endl;
        else
            std::cout << "Preview missing for " << scene.name << std::endl;

        scene.objectCount = 0;
        std::ifstream file(scene.sceneFile);
        std::string line;
        while (std::getline(file, line))
            if (line.rfind("obj ",0)==0) scene.objectCount++;

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