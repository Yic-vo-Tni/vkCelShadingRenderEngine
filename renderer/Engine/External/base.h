//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_BASE_H
#define VKCELSHADINGRENDERER_BASE_H

#include "pch.h"
#include "stl_mimalloc.h"

class nonCopyable{
public:
    nonCopyable() = default;
    ~nonCopyable() = default;
    nonCopyable(const nonCopyable&) = delete;
    nonCopyable& operator=(const nonCopyable&) = delete;
};

template<typename T>
class Singleton : public nonCopyable{
public:
    template<typename...Args>
    static T* make_ptr(Args&&...args){
        static T singleton{std::forward<Args>(args)...};
        return &singleton;
    }

    template<typename...Args>
    static T& make(Args&&...args){
        static T singleton{std::forward<Args>(args)...};
        return singleton;
    }
};

template<typename T>
class SingletonN : public nonCopyable {
public:
    template<typename... Args>
    static T* make_ptr(Args&&... args) {
        if (!s_ptr) {
            s_ptr = new T(std::forward<Args>(args)...);
        }
        return s_ptr;
    }

    template<typename... Args>
    static T& make(Args&&... args) {
        return *make_ptr(std::forward<Args>(args)...);
    }

    static void destroy_ptr() {
        delete s_ptr;
        s_ptr = nullptr;
    }

private:
    inline static T* s_ptr = nullptr;
};

template<template<typename> typename UPtr, typename T>
struct DeepCopyUptr {
    UPtr<T> ptr;

    DeepCopyUptr() = default;

    DeepCopyUptr(UPtr<T> p)
            : ptr(std::move(p)) {}

    DeepCopyUptr(const DeepCopyUptr& other) {
        if (other.ptr) ptr.reset(new T(*other.ptr));
    }

    DeepCopyUptr& operator=(const DeepCopyUptr& other) {
        if (this != &other) {
            if (other.ptr) ptr.reset(new T(*other.ptr));
            else            ptr.reset();
        }
        return *this;
    }

    DeepCopyUptr(DeepCopyUptr&&) noexcept = default;
    DeepCopyUptr& operator=(DeepCopyUptr&&) noexcept = default;

    T*       operator->()       { return ptr.get(); }
    T const* operator->() const { return ptr.get(); }
    T&       operator*()        { return *ptr; }
    T const& operator*()  const { return *ptr; }
};

template<typename T, typename Mutex>
struct Locked : public nonCopyable {
    T& ref;
    std::unique_lock<Mutex> lock;

    Locked(T& ref, Mutex& m) : ref(ref), lock(m) {}

    T* operator->() {
        return &ref;
    }
};

template<typename T>
struct Locked<T, oneapi::tbb::queuing_rw_mutex> : public nonCopyable {
    T& ref;
    oneapi::tbb::queuing_rw_mutex::scoped_lock lock;

    Locked(T& ref, oneapi::tbb::queuing_rw_mutex& m) : ref(ref), lock(m) {}

    T* operator->() {
        return &ref;
    }\



};

template<typename T>
struct Locked<T, oneapi::tbb::spin_rw_mutex> : public nonCopyable{
    T& ref;
    oneapi::tbb::spin_rw_mutex::scoped_lock lock;

    Locked(T& ref, oneapi::tbb::spin_rw_mutex& mutex) : ref(ref), lock(mutex, true){}

    T* operator->(){
        return &ref;
    }
};

struct Identifiable  {
    explicit Identifiable(vot::string id) : id(std::move(id)) {}

    virtual ~Identifiable() = default;

    vot::string id;
};

struct IdExtractor {
    static vot::string extract(const Identifiable& obj) {
        return obj.id;
    }
    static vot::string extract(const std::shared_ptr<Identifiable>& obj) {
        return obj->id;
    }
};

#define Make inline static constexpr auto make
#define Destroy_PTR inline static constexpr auto destroy

#define MAKE_SINGLETON(Self)               \
  public:                                          \
    template<typename... Args>                     \
    static Self* make(Args&&... args) {            \
      return SingletonN<Self>::make_ptr(           \
        std::forward<Args>(args)...                \
      );                                           \
    }                                              \
    inline static constexpr auto destroy = []{     \
    return SingletonN<Self>::destroy_ptr();        \
    }                                              \


struct IdGenerator{
    Make = []() { return Singleton<IdGenerator>::make_ptr();};

    static vot::string uniqueId(){
        auto inst = make();
        auto now = std::chrono::high_resolution_clock ::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distribution(0, 9999);

        std::stringstream ss;
        ss << millis << "-" << distribution(gen);

        return ss.str().c_str();
    }
};

#endif //VKCELSHADINGRENDERER_BASE_H
