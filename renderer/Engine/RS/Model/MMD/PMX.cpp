//
// Created by lenovo on 9/1/2025.
//

#include "PMX.h"

#include "Saba/Base/Path.h"

namespace rs2 {

    auto rs2::PMXLoader::Load(const vot::string &pt, const vot::string &dataDir) -> bool {
        saba::PMXFile pmx;
        if (!saba::ReadPMXFile(&pmx, pt.c_str())) {
            yic::logger->error("failed to read pmx file: {}", pt);
            return false;
        }

        const auto vertexCount = pmx.m_vertices.size();
        auto pos_pmr = yic::rsHub->makeVector<details::Position3>();
        auto nor_pmr = yic::rsHub->makeVector<details::Normal3>();
        auto uv_pmr = yic::rsHub->makeVector<details::UV2>();
        auto skin_pmr = yic::rsHub->makeVector<Skinning>();
        auto sdef_pmr = yic::rsHub->makeVector<SDEF>();
        pos_pmr.resize(vertexCount);
        nor_pmr.resize(vertexCount);
        uv_pmr.resize(vertexCount);
        skin_pmr.resize(vertexCount);

        const auto indexCount = pmx.m_faces.size() * 3;
        auto idx_pmr = yic::rsHub->makeVector<details::IndexU32>();
        idx_pmr.resize(indexCount);

        for (const auto& v : pmx.m_vertices) {
            auto pos = v.m_position * glm::vec3(1, 1, -1);
            auto nor = v.m_normal * glm::vec3(1, 1, -1);
            auto uv = glm::vec2(v.m_uv.x, 1.0f - v.m_uv.y);

            pos_pmr.push_back(pos);
            nor_pmr.push_back(nor);
            uv_pmr.push_back(uv);

            Skinning skinning;
            LBS lbs;
            SDEF sdef;
            if (saba::PMXVertexWeight::SDEF != v.m_weightType) {
                lbs.boneIds[0] = v.m_boneIndices[0];
                lbs.boneIds[1] = v.m_boneIndices[1];
                lbs.boneIds[2] = v.m_boneIndices[2];
                lbs.boneIds[3] = v.m_boneIndices[3];

                lbs.boneWeights[0] = v.m_boneWeights[0];
                lbs.boneWeights[1] = v.m_boneWeights[1];
                lbs.boneWeights[2] = v.m_boneWeights[2];
                lbs.boneWeights[3] = v.m_boneWeights[3];
            }

            switch (v.m_weightType) {
                case saba::PMXVertexWeight::BDEF1:
                    skinning.type = SkinningType::eLBS;
                    break;
                case saba::PMXVertexWeight::BDEF2:
                    skinning.type = SkinningType::eLBS;
                    break;
                case saba::PMXVertexWeight::BDEF4:
                    skinning.type = SkinningType::eLBS;
                    break;
                case saba::PMXVertexWeight::SDEF:
                    skinning.type = SkinningType::eSDEF;
                    {
                        auto w0 = v.m_boneWeights[0];
                        auto w1 = 1.0f - w0;

                        auto center = v.m_sdefC * glm::vec3(1, 1, -1);
                        auto r0 = v.m_sdefR0 * glm::vec3(1, 1, -1);
                        auto r1 = v.m_sdefR1 * glm::vec3(1, 1, -1);
                        auto rw = r0 * w0 + r1 * w1;
                        r0 = center + r0 - rw;
                        r1 = center + r1 - rw;
                        auto cr0 = (center + r0) * 0.5f;
                        auto cr1 = (center + r1) * 0.5f;

                        lbs.boneIds[0] = v.m_boneIndices[0];
                        lbs.boneIds[1] = v.m_boneIndices[1];
                        lbs.boneWeights[0] = v.m_boneWeights[0];
                        sdef.c = center;
                        sdef.r0 = cr0;
                        sdef.r1 = cr1;
                    }
                    break;
                case saba::PMXVertexWeight::QDEF:
                    skinning.type = SkinningType::eDQ;
                default:
                    break;
            }
            sdef_pmr.push_back(sdef);
            skinning.exIndex = sdef_pmr.size() - 1;
            skin_pmr.push_back(skinning);
        }

        for (auto idx = 0u; const auto& f : pmx.m_faces) {
            for (auto i = 0u; i < 3; i++) {
                auto vi = f.m_vertices[3 - i - 1];
                idx_pmr[idx++] = (uint32_t)vi;
            }
        }

        vot::vector<vot::string> texPh;
        texPh.resize(pmx.m_textures.size());
        for (const auto& tx : pmx.m_textures) {
            std::filesystem::path dirPath = std::filesystem::path(pt).parent_path();
            std::filesystem::path texPath = dirPath / tx.m_textureName;
            texPh.emplace_back(texPath.string());
        }




        return true;
    }

}
