//
// Created by lenovo on 7/16/2025.
//

#ifndef VKCELSHADINGRENDERER_DIRECTIONLIGHT_H
#define VKCELSHADINGRENDERER_DIRECTIONLIGHT_H

namespace sm {

    class DirectionLightTool {
    public:
        static auto updateLightSpaceMat(const glm::vec3& dir, const glm::mat4 &proj, const glm::mat4 &view) -> glm::mat4;
    private:
        static auto getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view) -> vot::vector<glm::vec3>;
    private:
    };

} // sm



#endif //VKCELSHADINGRENDERER_DIRECTIONLIGHT_H
