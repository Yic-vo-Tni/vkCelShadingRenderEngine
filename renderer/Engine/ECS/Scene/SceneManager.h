//
// Created by lenovo on 10/29/2024.
//

#ifndef VKCELSHADINGRENDERER_SCENE_H
#define VKCELSHADINGRENDERER_SCENE_H

namespace sc {

    struct MeshBufAddr{
        uint64_t vertAddr;
        uint64_t indexAddr;
    };

    struct Scene{
        explicit Scene(vot::string id) : id(std::move(id)){}

        vot::string id;
        vot::rhi::Accel_sptr tlas;
        vot::rhi::Descriptor_sptr rayTracingDescriptor;
        vot::vector<vot::rhi::Accel_sptr> blass;
        vot::vector<sc::MeshBufAddr> meshBufAddr;
        vot::rhi::Buffer_sptr meshBufAddrBuf;
//        vot::AABB aabb{glm::vec3{-FLT_MAX}, glm::vec3{FLT_MAX}};
    };

    class SceneManager {
    public:
        SceneManager();
        ~SceneManager();

    private:
        ev::pVkSetupContext ct;

        uint32_t mSceneCount{0};
        Scene* mActiveScene;
        vot::Image_sptr mRtOffImage;
        vot::unordered_map<vot::string, Scene> mScenes;
    };

} // sc

#endif //VKCELSHADINGRENDERER_SCENE_H
