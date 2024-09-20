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

    using string = std::basic_string<char, std::char_traits<char>, mi_stl_allocator<char>>;

    using uint32L = std::initializer_list<uint32_t>;

    template<typename T>
    struct smart_vector{
        smart_vector() = default;
        explicit smart_vector(size_t size){
            vec.resize(size);
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

        auto toStdVector() const { return std::vector<T>(begin(), end());}
    private:
        vector<T> vec{};
    };

}


#endif //VKCELSHADINGRENDERER_STL_MIMALLOC_H







//template<typename T>
//    struct unique_ptr {
//        template<typename... Args>
//        explicit unique_ptr(Args&&... args);
//
//        unique_ptr(const unique_ptr&) = delete;
//        unique_ptr& operator=(const unique_ptr&) = delete;
//        unique_ptr(unique_ptr&&) noexcept = default;
//        unique_ptr& operator=(unique_ptr&&) noexcept = default;
//
//        T* get() const { return ptr.get(); }
//        T* operator->() const { return ptr.operator->(); }
//        T& operator*() const { return *ptr; }
//
//    private:
//        std::unique_ptr<T> ptr;
//    };
//
//    template<typename T>
//    template<typename... Args>
//    unique_ptr<T>::unique_ptr(Args&&... args) : ptr(std::make_unique<T>(std::forward<Args>(args)...)) {}
//
//    template<typename T>
//    struct shared_ptr {
//        template<typename ...Args>
//        explicit shared_ptr(Args&&...args) : ptr(std::make_shared<T>(std::forward<Args>(args)...)) {}
//
//        shared_ptr(const shared_ptr& other) : ptr(other.ptr) {}
//        shared_ptr(shared_ptr&& other) noexcept : ptr(std::move(other.ptr)) {}
//        shared_ptr& operator=(const shared_ptr& other) {
//            if (this != &other) {
//                ptr = other.ptr;
//            }
//            return *this;
//        }
//        shared_ptr& operator=(shared_ptr&& other) noexcept {
//            if (this != &other) {
//                ptr = std::move(other.ptr);
//            }
//            return *this;
//        }
//
//        T& operator*() const { return *ptr; }
//        T* operator->() const { return ptr.operator->(); }
//        T* get() const { return ptr.get(); }
//
//        void reset(T* p = nullptr) { ptr.reset(p); }
//        void swap(shared_ptr& other) noexcept { ptr.swap(other.ptr); }
//        long use_count() const { return ptr.use_count(); }
//        bool unique() const { return ptr.unique(); }
//
//    private:
//        std::shared_ptr<T> ptr;
//    };
//
//
//
//    struct T{
//        int x;
//        int y;
//    };
//    void x(){
//        unique_ptr<int> t = unique_ptr<int>();
//        unique_ptr<T> t1 = unique_ptr<T>(2, 4);
//        t1->x;
//        shared_ptr<T> t_s = shared_ptr<T>(5, 5);
//    }