//
// Created by lenovo on 7/17/2024.
//

#ifndef VKCELSHADINGRENDERER_ENUM_H
#define VKCELSHADINGRENDERER_ENUM_H

#include "pch.h"

enum class RenderPhase{
    ePrimary, eCount
};

enum PrimaryRenderSeq{
    eRT, eGraphics, eCount
};

enum class ResFormat{
    ePmx, eObj, eFbx, eGltf,
    eImg,
    eVmd,
};

template<typename T>
inline auto enum_name(const T& t) -> std::enable_if_t<std::is_enum<T>::value, std::string>{
    return std::string(magic_enum::enum_name(t));
}


template<typename T>
inline auto Name(const T& t) -> std::enable_if_t<std::is_enum<T>::value, std::string>{
    return std::string(magic_enum::enum_name(t));
}
template std::string Name<RenderPhase>(const RenderPhase& t);
template std::string Name<PrimaryRenderSeq>(const PrimaryRenderSeq& t);
//
//
//void x(){
////    auto x = Name(RenderPhase::ePrimary);
//}

#endif //VKCELSHADINGRENDERER_ENUM_H
