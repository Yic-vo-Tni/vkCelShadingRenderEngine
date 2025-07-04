//
// Created by lenovo on 6/12/2025.
//

#ifndef VKCELSHADINGRENDERER_AUXILIARY_H
#define VKCELSHADINGRENDERER_AUXILIARY_H

#include "glm/gtc/type_ptr.hpp"
#include "assimp/matrix4x4.h"
#include "ozz/base/maths/soa_transform.h"

namespace miku{

//    inline auto ImGuiLabelID(const vot::string& label, const vot::string& hideLabel) -> const char*{
//        return (label + "##" + hideLabel).c_str();
//    }

    inline auto AssimpGLMConverter(const aiMatrix4x4& from) -> glm::mat4{
        glm::mat4 to;
        memcpy(glm::value_ptr(to), &from, sizeof(glm::mat4));
        return glm::transpose(to);
    }

    inline auto AssimpOzzConverter(const aiMatrix4x4 &aiTransform) -> ozz::math::Transform {
        aiVector3D scale, translation;
        aiQuaternion rotation;
        aiTransform.Decompose(scale, rotation, translation);

        ozz::math::Transform ozzTransform = {
                ozz::math::Float3(translation.x, translation.y, translation.z),
                ozz::math::Quaternion(rotation.x, rotation.y, rotation.z, rotation.w),
                ozz::math::Float3(scale.x, scale.y, scale.z)
        };
        return ozzTransform;
    }

    inline auto OzzGLMConverter(const ozz::math::Float4x4 &from) -> glm::mat4 {
        glm::mat4 to;
        memcpy(glm::value_ptr(to), &from.cols[0], sizeof(glm::mat4));
        return to;
    }


    inline auto OzzGLMConverter(const glm::mat4 &m) -> ozz::math::Float4x4 {
        ozz::math::Float4x4 ret{};
        for (int j = 0; j < 4; ++j) {
            ret.cols[j] = ozz::math::simd_float4::Load(
                    m[0][j],
                    m[1][j],
                    m[2][j],
                    m[3][j]
            );
        }
        return ret;
    }


}

#endif //VKCELSHADINGRENDERER_AUXILIARY_H
