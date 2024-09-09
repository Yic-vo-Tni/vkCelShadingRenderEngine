//
// Created by lenovo on 7/21/2024.
//

#ifndef VKCELSHADINGRENDERER_MODELLOADER_H
#define VKCELSHADINGRENDERER_MODELLOADER_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/FileOperator/FileOperation.h"
#include "Engine/ECS/Model/ModelStruct.h"
#include "Engine/RHI/Descriptor.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <Saba/Base/Path.h>
#include <Saba/Base/File.h>
#include <Saba/Base/UnicodeUtil.h>
#include <Saba/Base/Time.h>

namespace sc {

    class ModelLoader{
        using pt = std::filesystem::path;
    public:
        static auto Load(const std::string& path) -> Model;

    private:
        static auto process(const aiScene *scene, const std::filesystem::path &modelDirPath) -> Model;
        static auto processMeshCenter(const aiScene *scene, Model &model) -> aiVector3D;
    };

} // sc

#endif //VKCELSHADINGRENDERER_MODELLOADER_H
