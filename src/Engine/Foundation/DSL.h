//
// Created by lenovo on 8/31/2025.
//

#ifndef VKCELSHADINGRENDERER_DSL_H
#define VKCELSHADINGRENDERER_DSL_H

#include "../External/pch.h"

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

    template<typename T, typename F = void>
    struct Match;

    template<typename T>
    struct Match<T, void> {
        const T& value;
        bool matched{false};

        template<typename U, typename F>
        Match& case_(U&& expected, F&& func) {
            if (!matched && value == expected) {
                func();
                matched = true;
            }
            return *this;
        }

        Match& case_(std::initializer_list<std::string_view> exts, auto&& func) {
            if (!matched && std::ranges::any_of(exts, [&](auto e){ return value == e; })) {
                func();
                matched = true;
            }
            return *this;
        }

        template<typename F>
        auto default_(F &&func) -> void {
            if (!matched) func();
        }
    };

    template<typename T, typename R>
    struct Match {
        const T &value;
        bool matched{false};
        std::optional<R> result;

        template<typename U, typename F>
        Match &case_(U &&expected, F &&func) {
            if (!matched && value == expected) {
                result = func();
                matched = true;
            }
            return *this;
        }

        template<typename F>
        R default_(F &&func) {
            if (!matched)
                return func();
            return std::move(*result);
        }

        R unreachable_default() {
            if (!matched) {
                std::unreachable();
            }
            return std::move(*result);
        }
    };



}

#endif //VKCELSHADINGRENDERER_DSL_H
