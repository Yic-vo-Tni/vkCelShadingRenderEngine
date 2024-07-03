//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENTTYPES_H
#define VKCELSHADINGRENDERER_EVENTTYPES_H

#include "Engine/RHI/vkStruct.h"

#define HANA(...) BOOST_HANA_DEFINE_STRUCT(__VA_ARGS__)

#define RETURN_REF(Member) \
    [[nodiscard]] const auto& Member##_ref() const { \
        if (!Member.has_value()) \
            throw std::runtime_error(#Member " is not exist"); \
        return Member.value(); \
    } \
    [[nodiscard]] auto& Member##_ref() { \
        if (!Member.has_value()) \
            throw std::runtime_error(#Member " is not exist"); \
        return Member.value(); \
    } \


#define RETURN_CUSTOM_REF(MethodName, Member, CustomLogic) \
    [[nodiscard]] const auto& MethodName##_ref() const { \
        if (!Member.has_value()) \
            throw std::runtime_error(#MethodName " is not exist"); \
        return CustomLogic; \
    }  \
    [[nodiscard]] auto& MethodName##_ref() { \
        if (!Member.has_value()) \
            throw std::runtime_error(#MethodName " is not exist"); \
        return CustomLogic; \
    }

#define RETURN_VALUE(Member) \
    [[nodiscard]] auto Member##_v() const { \
        if (!Member.has_value()) \
            throw std::runtime_error(#Member " is not exist"); \
        return Member.value(); \
    }

#define RETURN_CUSTOM_VALUE(MethodName, Member, CustomLogic) \
    [[nodiscard]] auto MethodName##_v() const { \
        if (!Member.has_value()) \
            throw std::runtime_error(#MethodName " is not exist"); \
        return CustomLogic; \
    }  \

#define RETURN_MAP_REF(Member, KeyType) \
    [[nodiscard]] auto& Member##_ref(const KeyType& key) const { \
        if (!Member.has_value()) \
            throw std::runtime_error(#Member " is not exist"); \
        const auto& map = Member.value(); \
        auto it = map.find(key); \
        if (it == map.end()) \
            throw std::runtime_error(key + " is not exist"); \
        return it->second; \
    }

//namespace et {

template<typename T>
using opt = std::optional<T>;

//}

#include "vk_struct/vkBuf.h"

#include "gl_types.h"
#include "vk_types.h"
#include "ui_types.h"

namespace et{





}

#endif //VKCELSHADINGRENDERER_EVENTTYPES_H
