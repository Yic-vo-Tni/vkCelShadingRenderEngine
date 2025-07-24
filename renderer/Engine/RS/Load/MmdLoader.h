//
// Created by lenovo on 6/13/2025.
//

#ifndef VKCELSHADINGRENDERER_MMDLOADER_H
#define VKCELSHADINGRENDERER_MMDLOADER_H



namespace rs {

    class MmdLoader {
    public:
        MmdLoader() = default;
        ~MmdLoader() = default;

        auto Load(const vot::string &pt, vot::BasicInfoComponent& basicInfoComponent,
                  vot::VertexDataComponent &vertexDataComponent,
                  vot::RenderComponent &renderComponent) -> vot::string;
        auto vmd(const vot::string& pt) -> void;
        auto bindVmd(const vot::string& vmd, const vot::VertexDataComponent& vc, vot::AnimationComponent& ac) -> void;
        auto updateAnim(vot::VertexDataComponent& vc, vot::RenderComponent& rc) -> void;
        auto updateAnimVert(vot::VertexDataComponent& vc, vot::RenderComponent& rc) -> void;
    //    auto bakeVmd(vot::VertexDataComponent& vc, vot::AnimationComponent& ac) -> void;

        std::string mResDir{};
        std::string mMmdDir{};

        vot::vector<vot::string> ptVmds{};
      //  vot::vector<vot::vector<vot::Vertex>> bakeVertices;

        std::pmr::unsynchronized_pool_resource mVertexPool{};
    };

} // sc

#endif //VKCELSHADINGRENDERER_MMDLOADER_H
