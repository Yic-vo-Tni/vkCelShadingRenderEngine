//
// Created by lenovo on 8/2/2024.
//

#ifndef VKCELSHADINGRENDERER_ECS_TYPES_H
#define VKCELSHADINGRENDERER_ECS_TYPES_H

namespace et{

    struct modelPath{
        HANA_OPT(modelPath,
                 (std::vector<std::string>, paths));

        RETURN_REF(paths);
    };

}

#endif //VKCELSHADINGRENDERER_ECS_TYPES_H
