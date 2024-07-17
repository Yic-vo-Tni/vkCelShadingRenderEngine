//
// Created by lenovo on 7/17/2024.
//

#ifndef VKCELSHADINGRENDERER_ENUM_H
#define VKCELSHADINGRENDERER_ENUM_H

#include "pch.h"

template<typename T>
inline auto enum_name(const T& t) -> std::enable_if_t<std::is_enum<T>::value, std::string>{
    return std::string(magic_enum::enum_name(t));
}

enum class RenderProcessPhases{
    ePrimary, ePost
};

#endif //VKCELSHADINGRENDERER_ENUM_H
