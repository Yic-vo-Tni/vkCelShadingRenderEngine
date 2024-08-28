//
// Created by lenovo on 7/21/2024.
//

#ifndef VKCELSHADINGRENDERER_MODELLOADER_H
#define VKCELSHADINGRENDERER_MODELLOADER_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/FileOperator/FileOperation.h"
#include "Engine/ECS/Model/ModelStruct.h"
#include "Engine/RHI/Descriptor.h"

#include <Saba/Base/Path.h>
#include <Saba/Base/File.h>
#include <Saba/Base/UnicodeUtil.h>
#include <Saba/Base/Time.h>

namespace sc {



    class ModelLoader {
        using pt = std::filesystem::path;
    public:
        ModelLoader() = default;
        ~ModelLoader() = default;

        static auto Load(const std::string& path, PipelineDesSetLayout& setLayout) -> Model::Generic;
        static auto buildDescriptor(Model::Generic& model, PipelineDesSetLayout& setLayout) -> void;
//        static auto Pmx(const std::string& path, PipelineDesSetLayout& setLayout) -> Model::Pmx;
//        static auto Vmd(const std::string& path, Model::Pmx& pmx) -> void;
    private:
        static auto process(const aiScene *scene, const std::filesystem::path& modelDirPath, PipelineDesSetLayout& setLayout) -> Model::Generic;
        static auto processMesh(const aiMesh* aiMesh, Mesh& mesh, const aiVector3D& center) -> void;
        static auto processMeshCenter(const aiScene* scene, Model::Generic& model) -> aiVector3D;
        static auto processMaterial(aiMaterial *aiMaterial, Model::Generic& model, Mesh& mesh, oneapi::tbb::concurrent_unordered_map<pt, uint32_t>& paths, const std::filesystem::path& modelDirPath) -> void;

};

} // sc

#endif //VKCELSHADINGRENDERER_MODELLOADER_H
