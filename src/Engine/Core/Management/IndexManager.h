//
// Created by lenovo on 10/29/2024.
//

#ifndef VKCELSHADINGRENDERER_INDEXMANAGER_H
#define VKCELSHADINGRENDERER_INDEXMANAGER_H

namespace yic {

    class IndexManager {
    public:
        IndexManager();
        explicit IndexManager(uint32_t initIndex);

        auto allocate() -> uint32_t ;
        auto deallocate(uint32_t index) -> void ;

    private:
        uint32_t nextIndex{};
        vot::stack<uint32_t> availableIndices;
    };

} // yic

#endif //VKCELSHADINGRENDERER_INDEXMANAGER_H
