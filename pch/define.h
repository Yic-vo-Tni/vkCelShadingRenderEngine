//
// Created by lenovo on 6/3/2024.
//

#ifndef VKCELSHADINGRENDERER_DEFINE_H
#define VKCELSHADINGRENDERER_DEFINE_H

#include "pch.h"

#define Lambda_y(lambda) lambda()

#define Rvk_y(des) return vkCreate(des)
#define Rvk_t(des, level) return vkCreate(des, level)

#define DEFINE_STATIC_ACCESSOR_PARAM(func, param, types) \
    static auto func param { \
        return get()->func##_impl types; \
    }

#define DEFINE_STATIC_ACCESSOR(func, ...) \
    static auto func() -> decltype(auto) { \
        return get()->func##_impl(__VA_ARGS__); \
    }

#define DEFINE_STATIC_CUSTOM_ACCESSOR(name, func, param, types) \
    static auto name param { \
        return get()->func types; \
    }

#define DEFINE_STATIC_RETYPE_ACCESSOR(func, ret_type, param, types) \
    static ret_type func param { \
        return get()->func##_impl types; \
    }

#define INIT_MULTI(TYPE, ...) \
    TYPE __VA_ARGS__



#endif //VKCELSHADINGRENDERER_DEFINE_H
