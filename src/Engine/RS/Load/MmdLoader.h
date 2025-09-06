//
// Created by lenovo on 6/13/2025.
//

#ifndef VKCELSHADINGRENDERER_MMDLOADER_H
#define VKCELSHADINGRENDERER_MMDLOADER_H

#include "Saba/Model/MMD/VMDFile.h"

namespace rs {

    class MmdLoader {
    public:
        MmdLoader();
        ~MmdLoader() = default;

        auto Load(const vot::string &pt, vot::BasicInfoComponent& bic,
                  vot::VertexDataComponent &vdc, vot::RenderComponent &rc) -> vot::string;
        auto LoadVmd(const vot::string &pt) -> void;

    private:
        auto initPmx(const vot::string& pt, vot::BasicInfoComponent& bic, vot::VertexDataComponent &vdc) -> std::unique_ptr<saba::PMXModel>;
        auto buildVertices(saba::PMXModel* pmx, vot::VertexDataComponent& vdc, vot::RenderComponent &rc) -> void;
        auto buildIndices(const saba::PMXModel* pmx, vot::RenderComponent &rc) -> void;
        auto buildMaterials(const saba::PMXModel* pmx, vot::RenderComponent &rc) -> void;
        auto buildSubMeshes(const saba::PMXModel* pmx, vot::RenderComponent &rc) -> void;
        auto buildDS(vot::RenderComponent &rc) -> void;

    public:
        vot::vector<std::pair<vot::string, saba::VMDFile>> vmdFiles{};
    private:
        std::string mResDir{};
        std::string mMmdDir{};
        std::pmr::unsynchronized_pool_resource mVertexPool{};
    };

} // sc

#endif //VKCELSHADINGRENDERER_MMDLOADER_H
