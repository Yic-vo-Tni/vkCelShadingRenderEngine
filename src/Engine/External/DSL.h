//
// Created by lenovo on 8/31/2025.
//

#ifndef VKCELSHADINGRENDERER_DSL_H
#define VKCELSHADINGRENDERER_DSL_H

#include "pch.h"

namespace vot::dsl {

    template<typename T, typename F>
    auto operator>>=(T &&arg, F &&f) {
        return std::forward<F>(f)(std::forward<T>(arg));
    }

    namespace pass {
        template<typename T, typename F>
        auto operator|(T &&arg, F &&f) -> T && {
            if constexpr (std::is_invocable_v<F, T &>) {
                std::invoke(std::forward<F>(f), arg);
            } else if constexpr (std::is_invocable_v<F>) {
                auto step = std::invoke(std::forward<F>(f));
                step(arg);
            } else {
                static_assert(!sizeof(F), "operator| : f must be a step or a step factory");
            }
            return std::forward<T>(arg);
        }
    }

}

#endif //VKCELSHADINGRENDERER_DSL_H
