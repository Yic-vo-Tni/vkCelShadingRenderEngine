//
// Created by lenovo on 7/11/2024.
//

#ifndef VKCELSHADINGRENDERER_AUTOINDEXEDCONTAINER_H
#define VKCELSHADINGRENDERER_AUTOINDEXEDCONTAINER_H

#include "pch.h"
#include "base.h"


struct Identifiable  {
    explicit Identifiable(std::string id) : id(std::move(id)) {}

    virtual ~Identifiable() = default;

    std::string id;
};

struct IdExtractor {
    static std::string extract(const Identifiable& obj) {
        return obj.id;
    }
    static std::string extract(const std::shared_ptr<Identifiable>& obj) {
        return obj->id;
    }
};

struct IdGenerator{
    vkGet auto get = []() { return Singleton<IdGenerator>::get();};

    static std::string uniqueId(){
        auto inst = get();
        auto now = std::chrono::high_resolution_clock ::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distribution(0, 9999);

        std::stringstream ss;
        ss << millis << "-" << distribution(gen);

        return ss.str();
    }
};

namespace vot{

    template<typename T>
    class concurrent_shared_ptr_unordered_map {
    public:
        std::string tempId;
        static_assert(std::is_base_of_v<Identifiable, T>, "T must be derived form Identifiable");

        concurrent_shared_ptr_unordered_map() = default;

        explicit concurrent_shared_ptr_unordered_map(const std::shared_ptr<T> &s_ptr) {
            add(s_ptr);
        }

        template<typename U>
        concurrent_shared_ptr_unordered_map(const std::shared_ptr<U> &init_value) {
            static_assert(std::is_base_of<T, U>::value, "U must be derived from T");
            add(std::static_pointer_cast<T>(init_value));
        }

        concurrent_shared_ptr_unordered_map& operator=(const concurrent_shared_ptr_unordered_map& other){
            if (this != &other){
                for(const auto [key, value] : other.map){
                    this->add(value);
                }
            }

            return *this;
        }

        void add(const std::shared_ptr<T> &value) {
            tempId = IdExtractor::extract(*value);
            map[tempId] = value;
        }

        const std::shared_ptr<T> &find_ref(const std::string &k) const {
            static const std::shared_ptr<T> null_ptr = nullptr;
            auto it = map.find(k);
            if (it != map.end()) {
                return it->second;
            }
            return null_ptr;
        }

        bool remove(const std::string &k) {
            return map.unsafe_erase(k) > 0;
        }

        [[nodiscard]] size_t size() const {
            return map.size();
        }

    private:
        oneapi::tbb::concurrent_unordered_map<std::string, std::shared_ptr<T>> map;
    };

}












//template<typename T, typename key>
//class MultiIndexContainer{
//    public:
//    using containerType = boost::multi_index::multi_index_container<T, boost::multi_index::indexed_by<boost::multi_index::ordered_unique<boost::multi_index::tag<un::id>, key>>>;
//
//    void add(const T& object) {
//        container.insert(object);
//    }
//
//    const T* find(const std::string& k) const {
//        const auto& index = container.template get<un::id>();
//        auto it = index.find(k);
//        if (it != index.end()) {
//            return &*it;
//        }
//        return nullptr;
//    }
//
//    bool remove(const std::string& k) {
//        const auto& index = container.template get<un::id>();
//        auto it = index.find(k);
//        if (it != index.end()) {
//            index.erase(it);
//            return true;
//        }
//        return false;
//    }
//
//    private:
//    containerType container;
//};

#endif //VKCELSHADINGRENDERER_AUTOINDEXEDCONTAINER_H
