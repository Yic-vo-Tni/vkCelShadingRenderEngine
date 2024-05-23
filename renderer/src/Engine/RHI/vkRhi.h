//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_VKRHI_H
#define VKCELSHADINGRENDERER_VKRHI_H

#include "vkInit.h"

namespace yic {

    class vkRhi {
    public:
        vkRhi();

        bool run();

        void setRunCondition() { mRun.store(false);}
    private:
        std::atomic<bool> mRun;
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKRHI_H
