//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_PCH_H
#define VKCELSHADINGRENDERER_PCH_H

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "vulkan/vulkan.hpp"

#include "iostream"
#include "stdexcept"
#include "vector"
#include "set"
#include "optional"
#include "limits"
#include "utility"
#include "chrono"
#include "memory"
#include "fstream"
#include "bitset"
#include "unordered_map"
#include "variant"
#include "tuple"
#include "thread"
#include "mutex"
#include "functional"
#include "map"
#include "queue"
#include "random"
#include "sstream"
#include "future"
#include "concepts"
#include "typeinfo"
#include "typeindex"
#include "type_traits"
#include "ranges"
#include "stack"

#include "include/spdlog/spdlog.h"
#include "include/spdlog/sinks/stdout_color_sinks.h"
#include "include/spdlog/fmt/ostr.h"
#include "include/spdlog/sinks/ansicolor_sink.h"


#ifdef NDEBUG
const bool enableDebug = false;
#else
const bool enableDebug = true;
#endif

#define if_debug  if(enableDebug)


extern inline void try_catch(const std::function<void()> &fun, const std::string &des, spdlog::level::level_enum);

template<typename ReturnType>
struct vkCreateInvoker {
    explicit vkCreateInvoker(std::string description, spdlog::level::level_enum level) : mDescription(std::move(description)),
                                                                     mLevel(level) {};

    vkCreateInvoker() = default;

    template<typename Func>
    ReturnType operator=(Func &&func) {
        ReturnType returnType;
        try_catch(func, mDescription, mLevel);
        return returnType;
    }

private:
    std::string mDescription{};
    spdlog::level::level_enum mLevel{};
};

template<typename ReturnType>
inline vkCreateInvoker<ReturnType> vkCreate(const std::string & description = {}, spdlog::level::level_enum level = spdlog::level::info){
    return vkCreateInvoker<ReturnType>(description, level);
}

#endif //VKCELSHADINGRENDERER_PCH_H
