//
// Created by lenovo on 9/1/2025.
//

#ifndef VKCELSHADINGRENDERER_RESOURCEMANAGER_H
#define VKCELSHADINGRENDERER_RESOURCEMANAGER_H

#include "RS/Model/Generic/rs2.h"

namespace rs2 {
enum class PoolCategory { VertexIndex, Skinning, Misc };

template<typename T> struct PoolTag { static constexpr auto value = PoolCategory::Misc; };

template<> struct PoolTag<rs2::details::Position3> { static constexpr auto value = PoolCategory::VertexIndex; };
template<> struct PoolTag<rs2::details::Normal3>   { static constexpr auto value = PoolCategory::VertexIndex; };
template<> struct PoolTag<rs2::details::UV2>       { static constexpr auto value = PoolCategory::VertexIndex; };
template<> struct PoolTag<rs2::details::IndexU32>   { static constexpr auto value = PoolCategory::VertexIndex; };
template<> struct PoolTag<LBS>        { static constexpr auto value = PoolCategory::Skinning; };
template<> struct PoolTag<SDEF>       { static constexpr auto value = PoolCategory::Skinning; };
template<> struct PoolTag<DQ>         { static constexpr auto value = PoolCategory::Skinning; };

    class ResourceManager {
    public:
        MAKE_SINGLETON(ResourceManager);

        std::pmr::unsynchronized_pool_resource vertexIndexPool;
        std::pmr::unsynchronized_pool_resource skinningPool;
        std::pmr::unsynchronized_pool_resource miscPool;

        template<typename T>
        std::pmr::vector<T> makeVector() {
            switch(auto cat = PoolTag<T>::value) {
                case PoolCategory::VertexIndex: return std::pmr::vector<T>(&vertexIndexPool);
                case PoolCategory::Skinning:    return std::pmr::vector<T>(&skinningPool);
                default:                        return std::pmr::vector<T>(&miscPool);
            }
        }
    };




}

namespace yic {
    inline rs2::ResourceManager* rsHub;
}


#endif //VKCELSHADINGRENDERER_RESOURCEMANAGER_H