//
// Created by lenovo on 7/21/2024.
//

#ifndef VKCELSHADINGRENDERER_LOADMODEL_H
#define VKCELSHADINGRENDERER_LOADMODEL_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/FileOperator/FileOperation.h"
#include "Engine/ECS/Model/ModelStruct.h"
#include "Engine/RHI/Descriptor.h"

namespace sc {

    class ModelLoader {
        using pt = std::filesystem::path;
    public:
        ModelLoader() = default;
        ~ModelLoader() = default;

        static auto Load(const std::string& path, PipelineDesSetLayout& setLayout) -> Model;
    private:
        static auto process(const aiScene *scene, const std::filesystem::path& modelDirPath, PipelineDesSetLayout& setLayout) -> Model;
        static auto processMesh(const aiMesh* aiMesh, Mesh& mesh, const aiVector3D& center) -> void;
        static auto processMeshCenter(const aiScene* scene, Model& model) -> aiVector3D;
        static auto processMaterial(aiMaterial *aiMaterial, Model& model, Mesh& mesh, oneapi::tbb::concurrent_unordered_map<pt, uint32_t>& paths, const std::filesystem::path& modelDirPath) -> void;
    };

} // sc

#endif //VKCELSHADINGRENDERER_LOADMODEL_H
