//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_BASE_H
#define VKCELSHADINGRENDERER_BASE_H

#include "pch.h"

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
    static T* get(Args&&...args){
        static T singleton{std::forward<Args>(args)...};
        return &singleton;
    }

    template<typename...Args>
    static T& ref(Args&&...args){
        static T singleton{std::forward<Args>(args)...};
        return singleton;
    }
};



#define vkGet inline static constexpr

//
//class EventBus{
//public:
//    template<typename Event>
//    using EventHandler = std::function<void(const Event&)>;
//
//    template<typename Event>
//    void subscribe(const EventHandler<Event>& handler){
//        auto type = std::type_index(typeid(Event));
//        mHandlers[type].emplace_back([handler](const void* event){
//            handler(*static_cast<const Event*>(event));
//        });
//    }
//
//    template<typename Event>
//    void publish(const Event& event) const{
//        auto type = std::type_index(typeid(Event));
//        if (mHandlers.count(type)){
//            for(const auto& handler : mHandlers.at(type)){
//                handler(&event);
//            }
//        }
//    }
//
//private:
//    std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> mHandlers;
//};
//
//// example
//struct WindowSizeEvent{
//    int width, height;
//};
//
//class UIComponent{
//public:
//    void onWindowSizeChanged(const WindowSizeEvent& event) {
//        mWindowSize.width = event.width;
//        mWindowSize.height = event.height;
//        std::cout << "UI component resized to" << mWindowSize.width << " " << mWindowSize.height << "\n";
//    }
//
//private:
//    WindowSizeEvent mWindowSize{};
//};
//
//void fun(){
//    EventBus eventBus;
//    auto ui = std::make_shared<UIComponent>();
//
//    eventBus.subscribe<WindowSizeEvent>([ui](const WindowSizeEvent& event){
//        ui->onWindowSizeChanged(event);
//    });
//
//    eventBus.publish(WindowSizeEvent{800, 600});
//}

#endif //VKCELSHADINGRENDERER_BASE_H
