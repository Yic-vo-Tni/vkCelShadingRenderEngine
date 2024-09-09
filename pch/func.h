//
// Created by lenovo on 6/3/2024.
//

#ifndef VKCELSHADINGRENDERER_FUNC_H
#define VKCELSHADINGRENDERER_FUNC_H

#include "pch.h"

//template<typename T>
//using fn = std::function<T()>;
//
//template<typename T>
//auto f_call_t(const fn<std::optional<T>>& func) -> std::optional<T>{
//    func();
//}
//template<>
//auto f_call_t<bool>(const fn<std::optional<bool>>& func) -> std::optional<bool>{
//    func();
//
//    return true;
//}
//template<>
//auto f_call_t<std::string>(const fn<std::optional<std::string>>& func) -> std::optional<std::string>{
//    func();
//
//    return std::nullopt;
//}

//template<typename Key, typename Value>
//using lru_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::LRUCachePolicy>;

inline auto runningTIme(const std::function<void()>& fn) -> void{
    auto start = std::chrono::high_resolution_clock::now();

    fn();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << elapsed.count() << "ms\n";
}

#endif //VKCELSHADINGRENDERER_FUNC_H
