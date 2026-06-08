#pragma once
#include <vector>
#include "SceneInfo.h"

// Сканирует папку Scenes относительно Launcher.exe
// Возвращает вектор найденных сцен
std::vector<SceneInfo> ScanScenes();