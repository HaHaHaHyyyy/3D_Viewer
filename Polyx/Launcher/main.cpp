#include <iostream>
#include "SceneScanner.h"
#include "LauncherUtils.h"

int main()
{
    auto scenes = ScanScenes();

    for (const auto& scene : scenes)
    {
        std::cout << "Scene: "
                  << scene.name
                  << std::endl;

        std::cout << "Preview: "
                  << scene.previewFile
                  << std::endl;

        std::cout << "Objects: "
                  << scene.objectCount
                  << std::endl;

        std::cout << std::endl;
    }

    return 0;
}