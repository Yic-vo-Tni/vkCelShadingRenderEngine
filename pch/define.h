//
// Created by lenovo on 6/3/2024.
//

#ifndef VKCELSHADINGRENDERER_DEFINE_H
#define VKCELSHADINGRENDERER_DEFINE_H

#define Lambda_y(lambda) lambda()

#define Rvk_y(des) return vkCreate(des)
#define Rvk_t(des, level) return vkCreate(des, level)

#define DEFINE_STATIC_ACCESSOR(func, param, types) \
    static auto func param { \
        return get()->func##_impl types; \
    }
#define DEFINE_STATIC_RETYPE_ACCESSOR(func, ret_type, param, types) \
    static ret_type func param { \
        return get()->func##_impl types; \
    }

#endif //VKCELSHADINGRENDERER_DEFINE_H
