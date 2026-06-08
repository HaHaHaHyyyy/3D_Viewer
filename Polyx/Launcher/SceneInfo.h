#ifndef SCENE_INFO_H
#define SCENE_INFO_H

#include <string>

struct SceneInfo
{
    std::string sceneFile;
    std::string name;

    std::string previewFile;
    unsigned int previewTexture = 0;

    std::string creationDate;
    int objectCount = 0;
};

#endif