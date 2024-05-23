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

namespace pch {

    struct IfDebug {
    public:
        template<typename Func>
        auto operator=(Func &&func) -> decltype(func()) {
            if constexpr (enableDebug) {
                func();
                return *this;
            }
        }

    private:
        static constexpr bool enableDebug =
#ifdef NDEBUG
                false;
#else
                true;
#endif
    };

//    enum level{
//        eTrance, eInfo, eWarn, eError
//    };
//
//    extern void try_catch(const std::function<void()>& fun, const std::string & des, level);
//
//    struct vkCreateInvoker {
//        explicit vkCreateInvoker(std::string description, level level) : mDescription(std::move(description)), mLevel(level) {};
//        vkCreateInvoker() = default;
//
//        template<typename Func>
//        vkCreateInvoker& operator=(Func &&func)  {
//            try_catch(func, mDescription, mLevel);
//            return *this;
//        }
//
//    private:
//        std::string mDescription{};
//        level mLevel{};
//    };



}



inline constexpr pch::IfDebug if_debug;

//inline pch::vkCreateInvoker vkCreate(const std::string & description = {}, pch::level level = pch::level::eInfo){
//    return pch::vkCreateInvoker(description, level);
//}

//void x(){
//    vkCreate() = [&]{
//
//    } ;
//
//}

//extern void vkCreate(const std::function<void()>& fun, const std::string& description, int n = -1);

#endif //VKCELSHADINGRENDERER_PCH_H
