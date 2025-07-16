//
// Created by lenovo on 7/16/2025.
//

#include "DirectionLight.h"

namespace sm {

    auto DirectionLightTool::getFrustumCornersWorldSpace(const glm::mat4 &proj,
                                                     const glm::mat4 &view) -> vot::vector<glm::vec3> {
        const auto inv = glm::inverse(proj * view);

        vot::vector<glm::vec3> frustumCorners;
        for(int x = 0; x < 2; ++x)
            for(int y = 0; y < 2; ++y)
                for(int z = 0; z < 2; ++z){
                    auto pt = inv * glm::vec4(
                            2.f * (float)x - 1.f,
                            2.f * (float)y - 1.f,
                            z,
                            1.f
                            );
                    frustumCorners.push_back(glm::vec3(pt) / pt.w);
                }

        return frustumCorners;
    }

    auto DirectionLightTool::updateLightSpaceMat(const glm::vec3& dir,
                                             const glm::mat4 &proj,
                                              const glm::mat4 &view) -> glm::mat4 {
//        glm::vec3 lightDir = glm::normalize(dir);
//        auto frustumCorners = getFrustumCornersWorldSpace(proj, view);
//
//        glm::vec3 center(0.f);
//        for(const auto& v : frustumCorners) center += v;
//        center /= frustumCorners.size();
//
//        for (auto& v : frustumCorners) {
//            std::cout << "corner: " << v.x << ", " << v.y << ", " << v.z << std::endl;
//        }
//
//        float dis = 100.f;
//        glm::mat4 lightView = glm::lookAt(center - lightDir * dis, center, glm::vec3(0,1,0));
//
//        float minX=FLT_MAX, maxX=-FLT_MAX;
//        float minY=FLT_MAX, maxY=-FLT_MAX;
//        float minZ=FLT_MAX, maxZ=-FLT_MAX;
//        for (const auto& corner : frustumCorners) {
//            auto trf = lightView * glm::vec4(corner, 1.0f);
//            minX = std::min(minX, trf.x); maxX = std::max(maxX, trf.x);
//            minY = std::min(minY, trf.y); maxY = std::max(maxY, trf.y);
//            minZ = std::min(minZ, trf.z); maxZ = std::max(maxZ, trf.z);
//        }
//
////        yic::logger->info("x: {0}, {1}", minX, minX);
////        yic::logger->info("y: {0}, {1}", minY, minY);
////        yic::logger->info("z: {0}, {1}", minZ, minZ);
//        glm::mat4 lightProj = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
//
//        glm::mat4 lightSpaceMatrix = lightProj * lightView;

        auto dis = 30.f, nearPlane = 0.1f, farPlane = 60.f;
        auto v = glm::lookAt(dir, glm::vec3 {0.f, 0.f, 0.f}, glm::vec3 {0.f, 1.f, 0.f});
        auto left = -dis, right = dis, bottom = -dis, top = dis;
        auto p = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
        p[1][1] *= -1;

        glm::mat4 lightSpaceMatrix = p * v;

        return lightSpaceMatrix;
    }
} // sm