//
// Created by lenovo on 8/20/2024.
//

#ifndef VKCELSHADINGRENDERER_STL_MIMALLOC_H
#define VKCELSHADINGRENDERER_STL_MIMALLOC_H

#include "pch.h"

namespace vot{

    template<typename T>
    using vector = std::vector<T, mi_stl_allocator<T>>;

    template<typename T>
    using deque = std::deque<T, mi_stl_allocator<T>>;

    template<typename T>
    using list = std::list<T, mi_stl_allocator<T>>;

    template<typename Key, typename Comp = std::less<Key>>
    using set = std::set<Key, Comp, mi_stl_allocator<Key>>;

    template<typename Key, typename T, typename Comp = std::less<Key>>
    using map = std::map<Key, T, Comp, mi_stl_allocator<std::pair<const Key, T>>>;

    template<typename Key, typename Hash = std::hash<Key>, typename Eq = std::equal_to<Key>>
    using unordered_set = std::unordered_set<Key, Hash, Eq, mi_stl_allocator<Key>>;

    template<typename Key, typename T, typename Hash = std::hash<Key>, typename Eq = std::equal_to<Key>>
    using unordered_map = std::unordered_map<Key, T, Hash, Eq, mi_stl_allocator<std::pair<const Key, T>>>;

    template<typename T, typename Container = deque<T>>
    using queue = std::queue<T, Container>;

    template<typename T, typename Container = deque<T>>
    using stack = std::stack<T, Container>;

    using string = std::basic_string<char, std::char_traits<char>, mi_stl_allocator<char>>;

    using uint32L = std::initializer_list<uint32_t>;

    template<typename T>
    struct smart_vector{
        smart_vector() = default;
        explicit smart_vector(size_t size){
            vec.resize(size);
        }
        explicit smart_vector(T t){
            vec.emplace_back(t);
        }

        operator T() const { return vec.front(); }
        operator vector<T>() const { return vec; }

        operator std::vector<T>() const { return std::vector<T>(begin(), end()); }

        T& operator[](std::size_t index){
            return vec[index];
        }

        const T& operator [](std::size_t index) const{
            return vec[index];
        }

        auto reserve(std::size_t count) { vec.reserve(count); }

        template<typename ...Args>
        auto emplace_back(Args&& ...args) { vec.emplace_back(std::forward<Args>(args)...); }

        auto begin() { return vec.begin(); }
        auto end() { return vec.end(); }
        auto begin() const { return vec.begin(); }
        auto end() const { return vec.end(); }
        auto cbegin() const { return vec.cbegin(); }
        auto cend() const { return vec.cend(); }
        auto size() const { return vec.size(); }
        auto back() const { return vec.back(); }
        auto front() const { return vec.front(); }

        auto toStdVector() const { return std::vector<T>(begin(), end());}
    private:
        vector<T> vec{};
    };

}


#endif //VKCELSHADINGRENDERER_STL_MIMALLOC_H



