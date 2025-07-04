//
// Created by lenovo on 6/13/2025.
//

#ifndef VKCELSHADINGRENDERER_MMDLOADER_H
#define VKCELSHADINGRENDERER_MMDLOADER_H



namespace rs {

    class MmdLoader {
        struct Vertex{
            glm::vec3 pos;
            glm::vec3 nor;
            glm::vec2 uv;
        };
    public:
        MmdLoader() = default;
        ~MmdLoader() = default;

        auto Load(const vot::string &pt, vot::BasicInfoComponent& basicInfoComponent,
                  vot::VertexDataComponent &vertexDataComponent,
                  vot::RenderComponent &renderComponent) -> vot::string;
        auto vmd(const vot::string& pt) -> void;
        auto bindVmd(const vot::string& vmd, const vot::VertexDataComponent& vc, vot::AnimationComponent& ac) -> void;
        auto updateAnim(const vot::VertexDataComponent& vc, vot::RenderComponent& rc) -> void;
        auto bakeVmd(vot::VertexDataComponent& vc, vot::AnimationComponent& ac) -> void;

        std::string mResDir;
        std::string mMmdDir;

        vot::vector<vot::string> ptVmds;
        vot::vector<vot::vector<vot::Vertex>> bakeVertices;
    };

} // sc

#endif //VKCELSHADINGRENDERER_MMDLOADER_H
