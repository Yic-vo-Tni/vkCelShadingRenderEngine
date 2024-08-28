//
// Created by lenovo on 8/2/2024.
//

#ifndef VKCELSHADINGRENDERER_ECS_TYPES_H
#define VKCELSHADINGRENDERER_ECS_TYPES_H

namespace et{

    struct eEcs_ptr{
        HANA_OPT(eEcs_ptr,
                 (flecs::world*, ecs));

        RETURN_REF(ecs);
    };

    struct modelPath{
        HANA_OPT(modelPath,
                 (std::vector<std::string>, paths));

        RETURN_REF(paths);
    };

    struct eResPaths{
        HANA(eResPaths,
             (opt<std::unordered_map<ResFormat, std::vector<std::string>>>, paths));

        RETURN_VALUE(paths);
    };
    struct eResPathsNfd{
        HANA(eResPathsNfd,
             (opt<std::pair<ResFormat, void*>>, file));
    };

    struct eModel_ptr{
        HANA_OPT(eModel_ptr,
                 (sc::Model*, model));

        RETURN_REF(model);
    };

}

#endif //VKCELSHADINGRENDERER_ECS_TYPES_H
