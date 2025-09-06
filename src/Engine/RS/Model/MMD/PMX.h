//
// Created by lenovo on 9/1/2025.
//

#ifndef VKCELSHADINGRENDERER_PMX_H
#define VKCELSHADINGRENDERER_PMX_H

#include "RS/Model/Generic/rs2.h"
#include "ResourceManager.h"

namespace rs2 {

    class PMXLoader {
    public:

        auto Load(const vot::string& pt, const vot::string& dataDir) -> bool;
    private:

    };

} // rs

#endif //VKCELSHADINGRENDERER_PMX_H